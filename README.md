# NoteFlow
Desktop application that allows multiple users to collaboratively review and annotate a text/code file in real time collaboratively.

Server URL: `wss://noteflow-production-cd83.up.railway.app/`

## Build instructions:

1.  Clone the repo
2.  Install QScintilla https://www.riverbankcomputing.com/static/Docs/QScintilla/
3.  Open Qt Creator
4.  Open ui/CMakeLists.txt, configure with Qt 6 MinGW kit
6.  Also on CMakeLists update the library path to QScintilla to the library on your own system
7.  Run NoteFlow.exe or click Run in QtCreator

## Running NoteFlow

Download the build for your system from the [Releases](../../releases) page
(or, for the latest commit, open **Actions → latest Build run → Artifacts**).

| System | File |
| --- | --- |
| Windows 10/11 (64-bit) | `NoteFlow-Windows-x64.zip` |
| macOS 12+ (Intel or Apple Silicon) | `NoteFlow-macOS-Universal.zip` |
| Linux (x86_64) | `NoteFlow-Linux-x86_64.AppImage` |

### Windows
1. Right-click the zip → **Extract All**. Keep every file in the folder together, because `NoteFlow.exe` needs the DLLs next to it.
2. Double-click `NoteFlow.exe`.
3. If **"Windows protected your PC"** appears, click **More info → Run anyway**. This shows up because the app isn't code-signed.

### macOS
1. Unzip and drag `NoteFlow.app` into **Applications**.
2. Open it. Because the app isn't signed with an Apple Developer ID, macOS will block it the first time:
   - **macOS 15 (Sequoia) and later:** after the warning, go to **System Settings → Privacy & Security**, scroll down, and click **Open Anyway** next to NoteFlow, then confirm.
   - **macOS 14 and earlier:** right-click (or Control-click) `NoteFlow.app` → **Open** → **Open**.
3. If macOS says the app **"is damaged and can't be opened"**, clear the download quarantine flag in Terminal:
   ```sh
   xattr -dr com.apple.quarantine /Applications/NoteFlow.app
   ```
You only need to do this once; after that it opens normally.

### Linux
1. Make the file executable and run it:
   ```sh
   chmod +x NoteFlow-Linux-x86_64.AppImage
   ./NoteFlow-Linux-x86_64.AppImage
   ```
2. If you see an error about **FUSE**, install it (`sudo apt install libfuse2` on Ubuntu 22.04, `libfuse2t64` on 24.04+), or run without FUSE:
   ```sh
   ./NoteFlow-Linux-x86_64.AppImage --appimage-extract-and-run
   ```
