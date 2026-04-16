#include "ChatView.h"
#include "MessageBubble.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>

ChatView::ChatView(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("background: #0A0C12;");

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    // Header
    m_header = new QFrame(this);
    m_header->setFixedHeight(48);
    m_header->setStyleSheet(
        "QFrame { background: #0E1018; border-bottom: 2px solid #1A2030; }");
    auto *hl = new QHBoxLayout(m_header);
    hl->setContentsMargins(20, 0, 20, 0);

    m_channelLabel = new QLabel("", m_header);
    m_channelLabel->setStyleSheet(
        "color: #5B8AD4; font-size: 13px; font-weight: 700;"
        " letter-spacing: 0.1em; font-family: 'Consolas', monospace;");
    hl->addWidget(m_channelLabel);
    m_layout->addWidget(m_header);

    // Scroll area
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setStyleSheet(
        "QScrollArea { background: #0A0C12; border: none; }"
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
        "}");

    m_msgContainer = new QWidget();
    m_msgContainer->setStyleSheet("background: #0A0C12;");
    m_msgLayout = new QVBoxLayout(m_msgContainer);
    m_msgLayout->setContentsMargins(0, 16, 0, 16);
    m_msgLayout->setSpacing(4);
    m_msgLayout->addStretch();

    m_scroll->setWidget(m_msgContainer);
    m_layout->addWidget(m_scroll);

    auto *div = new QFrame(this);
    div->setFixedHeight(2);
    div->setStyleSheet("background: #1A2030;");
    m_layout->addWidget(div);

    // Input area
    auto *inputArea = new QWidget(this);
    inputArea->setFixedHeight(54);
    inputArea->setStyleSheet("background: #0E1018;");
    auto *il = new QHBoxLayout(inputArea);
    il->setContentsMargins(16, 10, 16, 10);
    il->setSpacing(8);

    m_input = new QLineEdit(inputArea);
    m_input->setPlaceholderText("message");
    m_input->setFixedHeight(32);
    m_input->setEnabled(false);
    m_input->setStyleSheet(
        "QLineEdit {"
        "  background: #080A0E; border: 1px solid #1A2030; border-radius: 0px;"
        "  color: #A8B8D0; font-size: 13px; padding: 0 10px;"
        "  font-family: 'Consolas', monospace;"
        "}"
        "QLineEdit:focus { border-color: #5B8AD4; background: #0C0E16; }"
        "QLineEdit:disabled { color: #1A2030; background: #080A0E; border-color: #121828; }"
        "QLineEdit::placeholder { color: #1A2030; }");

    m_sendBtn = new QPushButton("SEND", inputArea);
    m_sendBtn->setFixedSize(60, 32);
    m_sendBtn->setEnabled(false);
    m_sendBtn->setCursor(Qt::PointingHandCursor);
    m_sendBtn->setStyleSheet(
        "QPushButton {"
        "  background: #101828; border: 1px solid #1A2030; border-radius: 0px;"
        "  color: #4A5670; font-size: 11px; font-weight: 700;"
        "  letter-spacing: 0.1em; font-family: 'Consolas', monospace;"
        "}"
        "QPushButton:hover { border-color: #5B8AD4; color: #5B8AD4; }"
        "QPushButton:pressed { background: #0A0E18; }"
        "QPushButton:disabled { color: #121828; border-color: #121828; background: #0C0E16; }");

    il->addWidget(m_input);
    il->addWidget(m_sendBtn);
    m_layout->addWidget(inputArea);

    connect(m_sendBtn, &QPushButton::clicked,     this, &ChatView::onSendClicked);
    connect(m_input,   &QLineEdit::returnPressed, this, &ChatView::onSendClicked);
}

void ChatView::loadChannel(const QString &name, QList<Message> &messages)
{
    m_channelLabel->setText(name);
    m_currentMessages = &messages;

    while (m_msgLayout->count() > 1) {
        QLayoutItem *item = m_msgLayout->takeAt(1);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    for (auto &msg : messages)
        m_msgLayout->addWidget(new MessageBubble(msg, m_msgContainer));

    QTimer::singleShot(50, this, &ChatView::scrollToBottom);
}

void ChatView::appendMessage(const Message &msg)
{
    m_msgLayout->addWidget(new MessageBubble(msg, m_msgContainer));
    QTimer::singleShot(50, this, &ChatView::scrollToBottom);
}

void ChatView::setInputEnabled(bool enabled)
{
    m_input->setEnabled(enabled);
    m_sendBtn->setEnabled(enabled);
}

void ChatView::onSendClicked()
{
    QString text = m_input->text().trimmed();
    if (text.isEmpty()) return;
    m_input->clear();
    emit messageSent(text);
}

void ChatView::scrollToBottom()
{
    m_scroll->verticalScrollBar()->setValue(
        m_scroll->verticalScrollBar()->maximum());
}
