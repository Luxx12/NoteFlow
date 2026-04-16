#include "MessageBubble.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>

MessageBubble::MessageBubble(const Message &msg, QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto *outer = new QHBoxLayout(this);
    outer->setContentsMargins(16, 3, 16, 3);

    auto *col = new QVBoxLayout();
    col->setSpacing(2);

    if (!msg.isMe) {
        auto *senderLabel = new QLabel(msg.sender, this);
        senderLabel->setStyleSheet(
            "color: #5B8AD4; font-size: 10px; font-weight: 700;"
            " letter-spacing: 0.08em; font-family: 'Consolas', monospace;");
        col->addWidget(senderLabel);
    }

    auto *bubble = new QFrame(this);
    bubble->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    auto *bl = new QVBoxLayout(bubble);
    bl->setContentsMargins(10, 8, 10, 6);
    bl->setSpacing(4);

    auto *textLabel = new QLabel(msg.text, bubble);
    textLabel->setWordWrap(true);
    textLabel->setMaximumWidth(480);
    textLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    if (msg.isMe) {
        bubble->setStyleSheet(
            "QFrame { background: #101828; border: 1px solid #1A2030;"
            " border-radius: 0px; }");
        textLabel->setStyleSheet(
            "color: #A8B8D0; font-size: 13px; font-family: 'Consolas', monospace;");
    } else {
        bubble->setStyleSheet(
            "QFrame { background: #0C0E16; border: 1px solid #1A2030;"
            " border-radius: 0px; }");
        textLabel->setStyleSheet(
            "color: #8898B4; font-size: 13px; font-family: 'Consolas', monospace;");
    }

    auto *timeLabel = new QLabel(msg.timestamp, bubble);
    timeLabel->setStyleSheet(
        "color: #283048; font-size: 10px; font-family: 'Consolas', monospace;");
    timeLabel->setAlignment(Qt::AlignRight);

    bl->addWidget(textLabel);
    bl->addWidget(timeLabel);
    col->addWidget(bubble, 0, msg.isMe ? Qt::AlignRight : Qt::AlignLeft);

    if (msg.isMe) { outer->addStretch(); outer->addLayout(col); }
    else           { outer->addLayout(col); outer->addStretch(); }

    auto *effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0.0);
    setGraphicsEffect(effect);
    auto *anim = new QPropertyAnimation(effect, "opacity", this);
    anim->setDuration(120);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    QTimer::singleShot(10, anim, [anim]() { anim->start(); });
}
