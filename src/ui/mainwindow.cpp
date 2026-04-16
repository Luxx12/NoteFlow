#include "mainwindow.h"
#include "WsClient.h"
#include "ChatView.h"
#include "ChannelItemWidget.h"
#include "fileEditor.h"
#include "fileitemwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStatusBar>
#include <QTimer>
#include <QDateTime>
#include <QSplitter>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <Qsci/qsciscintilla.h>
#include <QFileDialog>

// Shared scrollbar stylesheet fragment
static const char *SCROLLBAR_STYLE =
    "QScrollBar:vertical {"
    "  background: #0C0E16; width: 6px; border: none; margin: 0;"
    "}"
    "QScrollBar::handle:vertical {"
    "  background: #1A2030; min-height: 20px;"
    "}"
    "QScrollBar::handle:vertical:hover {"
    "  background: #2A3448;"
    "}"
    "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
    "  height: 0px;"
    "}"
    "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
    "  background: none;"
    "}"
    "QScrollBar:horizontal {"
    "  background: #0C0E16; height: 6px; border: none; margin: 0;"
    "}"
    "QScrollBar::handle:horizontal {"
    "  background: #1A2030; min-width: 20px;"
    "}"
    "QScrollBar::handle:horizontal:hover {"
    "  background: #2A3448;"
    "}"
    "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
    "  width: 0px;"
    "}"
    "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
    "  background: none;"
    "}";

MainWindow::MainWindow(const QString &serverUrl, const QString &displayName, QWidget *parent)
    : QMainWindow(parent), m_myName(displayName), m_serverUrl(serverUrl)
{
    setWindowTitle("NoteFlow");
    setMinimumSize(860, 580);
    resize(1100, 680);

    m_ws = new WsClient(displayName, this);
    connect(m_ws, &WsClient::connected,       this, &MainWindow::onWsConnected);
    connect(m_ws, &WsClient::disconnected,    this, &MainWindow::onWsDisconnected);
    connect(m_ws, &WsClient::messageReceived, this, &MainWindow::onMessageReceived);
    connect(m_ws, &WsClient::errorOccurred,   this, &MainWindow::onWsError);

    connect(m_ws, &WsClient::channelListReceived,    this, &MainWindow::onChannelListReceived);
    connect(m_ws, &WsClient::channelCreated,         this, &MainWindow::onChannelCreated);
    connect(m_ws, &WsClient::messageHistoryReceived, this, &MainWindow::onMessageHistoryReceived);

    buildUI();

    statusBar()->setStyleSheet(
        "QStatusBar { background: #0A0C12; color: #4A5670; font-size: 11px;"
        " font-family: 'Consolas', monospace; letter-spacing: 0.05em;"
        " border-top: 1px solid #1A2030; }");
    statusBar()->showMessage("connecting...");

    m_ws->connectToServer(QUrl(serverUrl));
}

void MainWindow::buildUI()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);
    central->setStyleSheet(QString("background: #0A0C12; ") + SCROLLBAR_STYLE);

    auto *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Sidebar ──────────────────────────────
    auto *sidebar = new QWidget(central);
    sidebar->setFixedWidth(220);
    sidebar->setStyleSheet(
        "QWidget#sidebar { background: #0C0E16; border-right: 2px solid #1A2030; }");
    sidebar->setObjectName("sidebar");

    auto *sl = new QVBoxLayout(sidebar);
    sl->setContentsMargins(0, 0, 0, 0);
    sl->setSpacing(0);

    // ── Channel Section header ──
    auto *sectionHeader = new QWidget(sidebar);
    sectionHeader->setFixedHeight(48);
    sectionHeader->setObjectName("sectionHeader");
    sectionHeader->setStyleSheet(
        "#sectionHeader { background: #0E1018; border-bottom: 1px solid #1A2030; }");

    auto *shl = new QHBoxLayout(sectionHeader);
    shl->setContentsMargins(16, 0, 12, 0);

    auto *sectionLabel = new QLabel("CHANNELS", sectionHeader);
    sectionLabel->setStyleSheet(
        "color: #5B8AD4; font-size: 10px; font-weight: 700;"
        " letter-spacing: 0.18em; font-family: 'Consolas', monospace;"
        " background: transparent;");

    auto *addBtn = new QPushButton("+", sectionHeader);
    addBtn->setFixedSize(22, 22);
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setStyleSheet(
        "QPushButton {"
        "  background: transparent; border: 1px solid #1A2030;"
        "  border-radius: 0px; color: #4A5670; font-size: 14px; font-weight: 700;"
        "  font-family: 'Consolas', monospace;"
        "}"
        "QPushButton:hover { border-color: #5B8AD4; color: #5B8AD4; }"
        "QPushButton:pressed { background: #101828; }");

    shl->addWidget(sectionLabel);
    shl->addStretch();
    shl->addWidget(addBtn);
    sl->addWidget(sectionHeader);

    // ── Inline channel input ──
    m_channelInputWidget = new QWidget(sidebar);
    m_channelInputWidget->setFixedHeight(48);
    m_channelInputWidget->setStyleSheet("background: #0C0E16; border-bottom: 1px solid #1A2030;");
    m_channelInputWidget->setVisible(false);
    auto *cil = new QHBoxLayout(m_channelInputWidget);
    cil->setContentsMargins(10, 8, 10, 8);
    cil->setSpacing(6);

    m_channelInput = new QLineEdit(m_channelInputWidget);
    m_channelInput->setPlaceholderText("channel name");
    m_channelInput->setFixedHeight(28);
    m_channelInput->setMaxLength(32);
    m_channelInput->setStyleSheet(
        "QLineEdit {"
        "  background: #080A0E; border: 1px solid #1A2030; border-radius: 0px;"
        "  color: #A8B8D0; font-size: 11px; padding: 0 8px;"
        "  font-family: 'Consolas', monospace;"
        "}"
        "QLineEdit:focus { border-color: #5B8AD4; }"
        "QLineEdit::placeholder { color: #1A2030; }");

    auto *confirmBtn = new QPushButton("ADD", m_channelInputWidget);
    confirmBtn->setFixedSize(40, 28);
    confirmBtn->setCursor(Qt::PointingHandCursor);
    confirmBtn->setStyleSheet(
        "QPushButton {"
        "  background: #101828; border: 1px solid #5B8AD4; border-radius: 0px;"
        "  color: #5B8AD4; font-size: 10px; font-weight: 700;"
        "  font-family: 'Consolas', monospace;"
        "}"
        "QPushButton:hover { background: #1A2438; color: #7AAAE8; }"
        "QPushButton:pressed { background: #0A0E18; }");

    cil->addWidget(m_channelInput);
    cil->addWidget(confirmBtn);
    sl->addWidget(m_channelInputWidget);

    // ── Channel Tree ──
    m_channelTree = new QTreeWidget(sidebar);
    m_channelTree->setFrameShape(QFrame::NoFrame);
    m_channelTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_channelTree->setIndentation(12);
    m_channelTree->setHeaderHidden(true);
    m_channelTree->setStyleSheet(
        QString(
            "QTreeWidget {"
            "  background: #0C0E16; border: none; outline: none;"
            "  font-family: 'Consolas', monospace;"
            "}"
            "QTreeWidget::item {"
            "  border: none; padding: 0px; background: transparent;"
            "}"
            "QTreeWidget::item:selected {"
            "  background: #101828; border-left: 2px solid #5B8AD4;"
            "}"
            "QTreeWidget::item:hover:!selected {"
            "  background: #0E1220;"
            "}")
        + SCROLLBAR_STYLE);

    sl->addWidget(m_channelTree);
    sl->addStretch();

    // ── User footer ──
    auto *footer = new QFrame(sidebar);
    footer->setFixedHeight(50);
    footer->setObjectName("footer");
    footer->setStyleSheet(
        "#footer { background: #0E1018; border-top: 1px solid #1A2030; }");
    auto *fl = new QHBoxLayout(footer);
    fl->setContentsMargins(16, 0, 16, 0);
    fl->setSpacing(0);

    m_statusDot = new QFrame(footer);
    m_statusDot->setFixedSize(8, 8);
    m_statusDot->setStyleSheet("background: #1A2030; border-radius: 0px;");

    auto *userNameLabel = new QLabel(m_myName, footer);
    userNameLabel->setStyleSheet(
        "color: #A8B8D0; font-size: 11px; font-weight: 600;"
        " letter-spacing: 0.08em; font-family: 'Consolas', monospace;"
        " background: transparent;");

    fl->addWidget(m_statusDot);
    fl->addSpacing(8);
    fl->addWidget(userNameLabel);
    fl->addStretch();
    sl->addWidget(footer);

    // ── Editor and splitter ──
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->setStyleSheet(
        "QSplitter::handle { background: #1A2030; width: 2px; }");
    m_editor = new fileEditor(this);

    // ── Chat View ──
    m_chatView = new ChatView(central);
    connect(m_chatView, &ChatView::messageSent, this, &MainWindow::onSendMessage);

    root->addWidget(splitter);
    splitter->addWidget(sidebar);
    splitter->addWidget(m_editor);
    splitter->addWidget(m_chatView);

    // ── Connections ──
    connect(addBtn, &QPushButton::clicked, this, [this]() {
        showChannelInput(!m_channelInputWidget->isVisible());
    });
    connect(confirmBtn,     &QPushButton::clicked,     this, &MainWindow::onAddChannelConfirmed);
    connect(m_channelInput, &QLineEdit::returnPressed, this, &MainWindow::onAddChannelConfirmed);
    connect(m_channelTree,  &QTreeWidget::itemClicked,
            this, &MainWindow::onTreeItemClicked);

    connect(m_ws,     &WsClient::fileEditReceived,   this, &MainWindow::onRemoteFileEdit);
    connect(m_ws,     &WsClient::fileUploadReceived, this, &MainWindow::onFileUploadReceived);
    connect(m_editor, &fileEditor::localFileChanged, this, &MainWindow::onLocalFileChanged);
}

void MainWindow::showChannelInput(bool visible)
{
    m_channelInputWidget->setVisible(visible);
    if (visible) {
        m_channelInput->clear();
        m_channelInput->setFocus();
    }
}

void MainWindow::addChannelToList(const QString &name)
{
    auto *channelItem = new QTreeWidgetItem(m_channelTree);
    channelItem->setData(0, Qt::UserRole, "CHANNEL:" + name);

    auto *itemWidget = new ChannelItemWidget(name, "", 0, m_channelTree);
    m_channelTree->setItemWidget(channelItem, 0, itemWidget);

    connect(itemWidget, &ChannelItemWidget::addFile, this, [this, channelItem, name]() {
        QString filePath = QFileDialog::getOpenFileName(this, "Upload file to " + name);
        if (!filePath.isEmpty()) {
            auto *fileItem = new QTreeWidgetItem(channelItem);
            fileItem->setData(0, Qt::UserRole, "FILE:" + filePath);
            auto *fileWidget = new fileItemWidget(m_channelTree, filePath);
            m_channelTree->setItemWidget(fileItem, 0, fileWidget);
            channelItem->setExpanded(true);

            connect(fileWidget, &fileItemWidget::fileSelected,
                    this, [this](const QString &path) {
                        m_editor->loadFile(path);
                        m_currentFile = QFileInfo(path).fileName();
                    });

            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray content = file.readAll();
                file.close();
                m_ws->sendFileUpload(name, QFileInfo(filePath).fileName(), content);
            }
        }
    });
}

// ── Persistence handlers ─────────────────────────────────────────────────────

void MainWindow::onChannelListReceived(const QStringList &channels)
{
    for (const QString &name : channels) {
        if (!m_channels.contains(name)) {
            m_channels.append(name);
            addChannelToList(name);
        }
    }
}

void MainWindow::onChannelCreated(const QString &channel)
{
    if (!m_channels.contains(channel)) {
        m_channels.append(channel);
        addChannelToList(channel);
    }
}

void MainWindow::onMessageHistoryReceived(const QString &channel,
                                          const QVector<Message> &messages)
{
    m_messages[channel] = messages;
    if (channel == m_activeChannel) {
        m_chatView->loadChannel(m_activeChannel, m_messages[m_activeChannel]);
    }
}

// ── Channel management ───────────────────────────────────────────────────────

void MainWindow::onAddChannelConfirmed()
{
    QString name = m_channelInput->text().trimmed().toLower();
    name.replace(" ", "-");
    if (name.isEmpty() || m_channels.contains(name)) {
        showChannelInput(false);
        return;
    }

    m_channels.append(name);
    addChannelToList(name);
    showChannelInput(false);
    m_ws->sendChannelCreate(name);

    int lastIndex = m_channelTree->topLevelItemCount() - 1;
    QTreeWidgetItem *newItem = m_channelTree->topLevelItem(lastIndex);
    if (newItem) {
        m_channelTree->setCurrentItem(newItem);
        onTreeItemClicked(newItem, 0);
    }
}

void MainWindow::onRemoveCurrentChannel()
{
    QTreeWidgetItem *currentItem = m_channelTree->currentItem();
    if (!currentItem) return;

    QString data = currentItem->data(0, Qt::UserRole).toString();

    if (data.startsWith("CHANNEL:")) {
        QString name = data.mid(8);
        m_channels.removeOne(name);
        m_messages.remove(name);
        delete currentItem;

        if (!m_channels.isEmpty()) {
            QTreeWidgetItem *topItem = m_channelTree->topLevelItem(0);
            m_channelTree->setCurrentItem(topItem);
            onTreeItemClicked(topItem, 0);
        } else {
            m_activeChannel.clear();
        }
    }
    else if (data.startsWith("FILE:")) {
        delete currentItem;
    }
}

void MainWindow::onTreeItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (!item) return;

    QString data = item->data(0, Qt::UserRole).toString();

    if (data.startsWith("CHANNEL:")) {
        m_activeChannel = data.mid(8);
        m_chatView->loadChannel(m_activeChannel, m_messages[m_activeChannel]);
        m_ws->joinChannel(m_activeChannel);
    }
}

// ── Chat ─────────────────────────────────────────────────────────────────────

void MainWindow::onSendMessage(const QString &text)
{
    if (m_activeChannel.isEmpty()) return;

    Message msg;
    msg.sender    = m_myName;
    msg.text      = text;
    msg.timestamp = QDateTime::currentDateTime().toString("hh:mm");
    msg.isMe      = true;

    m_messages[m_activeChannel].append(msg);
    m_chatView->appendMessage(msg);
    m_ws->sendMessage(m_activeChannel, text);
}

void MainWindow::onMessageReceived(const QString &channel, const Message &msg)
{
    if (msg.isMe) return;
    m_messages[channel].append(msg);
    if (channel == m_activeChannel)
        m_chatView->appendMessage(msg);
}

// ── Connection state ─────────────────────────────────────────────────────────

void MainWindow::onWsConnected()
{
    m_statusDot->setStyleSheet("background: #4A9E6A; border-radius: 0px;");
    m_chatView->setInputEnabled(true);
    statusBar()->showMessage("online  " + m_myName);

    if (!m_activeChannel.isEmpty())
        m_ws->joinChannel(m_activeChannel);
}

void MainWindow::onWsDisconnected()
{
    m_statusDot->setStyleSheet("background: #1A2030; border-radius: 0px;");
    m_chatView->setInputEnabled(false);
    statusBar()->showMessage("disconnected  retrying...");
    QTimer::singleShot(3000, this, [this]() {
        statusBar()->showMessage("reconnecting...");
        m_ws->connectToServer(QUrl(m_serverUrl));
    });
}

void MainWindow::onWsError(const QString &reason)
{
    statusBar()->showMessage("error  " + reason);
}

// ── File sync ────────────────────────────────────────────────────────────────

void MainWindow::onLocalFileChanged(const QString &filename, int position, int length,
                                    const QString &text, bool isAddition)
{
    if (m_activeChannel.isEmpty()) return;
    m_ws->sendFileEdit(m_activeChannel, filename, position, length, text, isAddition);
}

void MainWindow::onRemoteFileEdit(const QString &channel, const QString &filename,
                                  int position, int length, const QString &text,
                                  bool isAddition)
{
    if (channel != m_activeChannel) return;
    if (filename != m_currentFile) return;
    m_editor->applyRemoteEdit(position, length, text, isAddition);
}

void MainWindow::onFileUploadReceived(const QString &channel, const QString &filename,
                                      const QByteArray &content)
{
    QString localDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
    + "/NoteFlow/" + channel;
    QDir().mkpath(localDir);
    QString localPath = localDir + "/" + filename;

    QFile file(localPath);
    if (!file.open(QIODevice::WriteOnly)) return;
    file.write(content);
    file.close();

    bool channelFound = false;
    for (int i = 0; i < m_channelTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *channelItem = m_channelTree->topLevelItem(i);
        QString data = channelItem->data(0, Qt::UserRole).toString();

        if (data == "CHANNEL:" + channel) {
            channelFound = true;

            bool alreadyExists = false;
            for (int j = 0; j < channelItem->childCount(); ++j) {
                QString childData = channelItem->child(j)->data(0, Qt::UserRole).toString();
                if (childData == "FILE:" + localPath) {
                    alreadyExists = true;
                    break;
                }
            }
            if (alreadyExists) break;

            auto *fileItem = new QTreeWidgetItem(channelItem);
            fileItem->setData(0, Qt::UserRole, "FILE:" + localPath);
            auto *fileWidget = new fileItemWidget(m_channelTree, localPath);
            m_channelTree->setItemWidget(fileItem, 0, fileWidget);
            channelItem->setExpanded(true);

            connect(fileWidget, &fileItemWidget::fileSelected,
                    this, [this](const QString &path) {
                        m_editor->loadFile(path);
                        m_currentFile = QFileInfo(path).fileName();
                    });
            break;
        }
    }
}
