import express from "express";
import dotenv from "dotenv";
import { MongoClient } from "mongodb";
import cors from "cors";
import { WebSocketServer } from "ws";
import { createServer } from "http";

dotenv.config();

const app = express();
app.use(express.json({ limit: "50mb" }));
app.use(cors());

const PORT = process.env.PORT || 3000;
const uri = process.env.MONGODB_URI;

if (!uri) {
  console.error("MONGODB_URI not set");
  process.exit(1);
}

let db;

async function connectDB() {
  try {
    const client = new MongoClient(uri);
    await client.connect();
    db = client.db();

    // Ensure the channels collection exists with a unique index
    await db.collection("channels").createIndex({ name: 1 }, { unique: true });

    console.log("MongoDB connected");
  } catch (err) {
    console.error(err);
    process.exit(1);
  }
}

// ── REST endpoints (unchanged) ───────────────────────────────────────────────

app.get("/", (req, res) => {
  res.json({ status: "NoteFlow API running" });
});

app.post("/notes", async (req, res) => {
  try {
    const note = req.body;
    const result = await db.collection("notes").insertOne(note);
    res.json({ success: true, id: result.insertedId });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

app.get("/notes", async (req, res) => {
  try {
    const notes = await db.collection("notes").find().toArray();
    res.json(notes);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

app.post("/files", async (req, res) => {
  try {
    const file = req.body;
    const result = await db.collection("files").insertOne(file);
    res.json({ success: true, id: result.insertedId });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

app.get("/files/:channel", async (req, res) => {
  try {
    const files = await db
      .collection("files")
      .find({ channel: req.params.channel })
      .toArray();
    res.json(files);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// ── In-memory channel membership ────────────────────────────────────────────

const channels = new Map(); // channelName -> Set<ws>

function getChannel(name) {
  if (!channels.has(name)) channels.set(name, new Set());
  return channels.get(name);
}

function broadcast(channelName, payload, exclude = null) {
  const members = channels.get(channelName);
  if (!members) return;

  const data = JSON.stringify(payload);

  for (const client of members) {
    if (client !== exclude && client.readyState === 1) {
      client.send(data);
    }
  }
}

// Send a payload to a single client
function sendTo(ws, payload) {
  if (ws.readyState === 1) {
    ws.send(JSON.stringify(payload));
  }
}

// ── WebSocket server ─────────────────────────────────────────────────────────

const server = createServer(app);
const wss = new WebSocketServer({
  server,
  maxPayload: 50 * 1024 * 1024, // 50 MB — needed for file uploads
});

const keepAlive = setInterval(() => {
  wss.clients.forEach((ws) => {
    if (!ws.isAlive) {
      ws.terminate();
      return;
    }
    ws.isAlive = false;
    ws.ping();
  });
}, 25000);

wss.on("close", () => clearInterval(keepAlive));

wss.on("connection", async (ws) => {
  ws.isAlive = true;
  ws.currentChannel = null;
  ws.displayName = "anonymous";

  ws.on("pong", () => {
    ws.isAlive = true;
  });

  // ── Send the persisted channel list on connect ──
  try {
    const channelDocs = await db.collection("channels").find().toArray();
    const names = channelDocs.map((c) => c.name);
    sendTo(ws, { type: "channel_list", channels: names });
  } catch (err) {
    console.error("Failed to send channel list:", err);
  }

  // ── Message router ──
  ws.on("message", async (raw) => {
    let msg;
    try {
      msg = JSON.parse(raw);
    } catch {
      return;
    }

    // ────────────────────────────────────────────────────────────────────
    // JOIN — switch channels, send message history + file list to joiner
    // ────────────────────────────────────────────────────────────────────
    if (msg.type === "join") {
      // Leave previous channel
      if (ws.currentChannel) {
        const prev = channels.get(ws.currentChannel);
        if (prev) {
          prev.delete(ws);
          broadcast(ws.currentChannel, {
            type: "presence",
            channel: ws.currentChannel,
            count: prev.size,
          });
        }
      }

      ws.currentChannel = msg.channel;
      ws.displayName = msg.sender || "anonymous";
      getChannel(msg.channel).add(ws);

      broadcast(msg.channel, {
        type: "presence",
        channel: msg.channel,
        count: channels.get(msg.channel).size,
      });

      // Send message history to the joining client
      try {
        const history = await db
          .collection("messages")
          .find({ channel: msg.channel })
          .sort({ _id: 1 })
          .toArray();

        sendTo(ws, {
          type: "message_history",
          channel: msg.channel,
          messages: history.map((m) => ({
            sender: m.sender,
            text: m.text,
            ts: m.ts,
          })),
        });
      } catch (err) {
        console.error("Failed to send message history:", err);
      }

      // Send all files that belong to this channel
      try {
        const files = await db
          .collection("files")
          .find({ channel: msg.channel })
          .toArray();

        for (const f of files) {
          sendTo(ws, {
            type: "file_upload",
            channel: msg.channel,
            filename: f.filename,
            content: f.content, // already base64
          });
        }
      } catch (err) {
        console.error("Failed to send file list:", err);
      }
    }

    // ────────────────────────────────────────────────────────────────────
    // CHANNEL CREATE — persist + broadcast to everyone
    // ────────────────────────────────────────────────────────────────────
    else if (msg.type === "channel_create") {
      if (!msg.channel) return;

      try {
        await db
          .collection("channels")
          .insertOne({ name: msg.channel, createdBy: msg.sender });
      } catch (err) {
        // Duplicate key = channel already exists — that's fine
        if (err.code !== 11000) console.error("channel_create error:", err);
      }

      // Notify every connected client (except the creator)
      for (const client of wss.clients) {
        if (client !== ws && client.readyState === 1) {
          client.send(
            JSON.stringify({
              type: "channel_create",
              channel: msg.channel,
            })
          );
        }
      }
    }

    // ────────────────────────────────────────────────────────────────────
    // MESSAGE — persist + broadcast
    // ────────────────────────────────────────────────────────────────────
    else if (msg.type === "message") {
      if (!msg.channel || !msg.text) return;

      const ts = msg.ts || new Date().toTimeString().slice(0, 5);

      // Persist the message
      try {
        await db.collection("messages").insertOne({
          channel: msg.channel,
          sender: msg.sender,
          text: msg.text,
          ts,
        });
      } catch (err) {
        console.error("Failed to persist message:", err);
      }

      broadcast(
        msg.channel,
        {
          type: "message",
          channel: msg.channel,
          sender: msg.sender,
          text: msg.text,
          ts,
        },
        ws
      );
    }

    // ────────────────────────────────────────────────────────────────────
    // FILE UPLOAD — persist + broadcast to channel (except sender)
    // ────────────────────────────────────────────────────────────────────
    else if (msg.type === "file_upload") {
      if (!msg.channel || !msg.filename || !msg.content) return;

      console.log(
        `[file_upload] ${msg.filename} -> ${msg.channel} (${msg.content.length} base64 chars)`
      );

      // Persist (upsert so re-uploading the same filename replaces it)
      try {
        await db.collection("files").updateOne(
          { channel: msg.channel, filename: msg.filename },
          {
            $set: {
              content: msg.content,
              sender: msg.sender,
              uploadedAt: new Date(),
            },
          },
          { upsert: true }
        );
      } catch (err) {
        console.error("Failed to persist file:", err);
      }

      // Broadcast to everyone else in the channel
      broadcast(
        msg.channel,
        {
          type: "file_upload",
          channel: msg.channel,
          filename: msg.filename,
          content: msg.content,
        },
        ws
      );
    }

    // ────────────────────────────────────────────────────────────────────
    // FILE EDIT — broadcast to channel (except sender)
    // ────────────────────────────────────────────────────────────────────
    else if (msg.type === "file_edit") {
      if (!msg.channel || !msg.filename) return;

      broadcast(
        msg.channel,
        {
          type: "file_edit",
          channel: msg.channel,
          sender: msg.sender,
          filename: msg.filename,
          position: msg.position,
          length: msg.length,
          text: msg.text,
          isAddition: msg.isAddition,
        },
        ws
      );
    }
  });

  // ── Cleanup on disconnect ──
  ws.on("close", () => {
    if (ws.currentChannel) {
      const ch = channels.get(ws.currentChannel);
      if (ch) {
        ch.delete(ws);
        broadcast(ws.currentChannel, {
          type: "presence",
          channel: ws.currentChannel,
          count: ch.size,
        });
      }
    }
  });
});

// ── Start ────────────────────────────────────────────────────────────────────

connectDB().then(() => {
  server.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
  });
});
