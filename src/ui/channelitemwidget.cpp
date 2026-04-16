#include "ChannelItemWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

ChannelItemWidget::ChannelItemWidget(const QString &name, const QString &preview,
                                     int unread, QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(42);

    setStyleSheet(
        "QWidget {"
        "  border-bottom: 1px solid #121828;"
        "  background: transparent;"
        "}");

    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(14, 0, 14, 0);
    l->setSpacing(2);

    auto *topRow = new QHBoxLayout();
    topRow->setContentsMargins(0, 0, 0, 0);

    auto *nameLabel = new QLabel(name, this);
    nameLabel->setStyleSheet(
        "color: #A8B8D0; font-size: 12px; font-weight: 600;"
        " font-family: 'Consolas', monospace; background: transparent;");
    topRow->addWidget(nameLabel);

    auto *uploadBtn = new QPushButton("+");
    uploadBtn->setFixedSize(20, 20);
    uploadBtn->setCursor(Qt::PointingHandCursor);
    uploadBtn->setStyleSheet(
        "QPushButton {"
        "  background: transparent; border: 1px solid #1A2030;"
        "  border-radius: 0px; color: #4A5670; font-size: 13px; font-weight: 700;"
        "  font-family: 'Consolas', monospace;"
        "}"
        "QPushButton:hover { border-color: #5B8AD4; color: #5B8AD4; }"
        "QPushButton:pressed { background: #101828; }");
    topRow->addWidget(uploadBtn);
    connect(uploadBtn, &QPushButton::clicked, this, &ChannelItemWidget::addFile);

    topRow->addStretch();

    if (unread > 0) {
        auto *badge = new QLabel(QString::number(unread), this);
        badge->setFixedSize(18, 16);
        badge->setAlignment(Qt::AlignCenter);
        badge->setStyleSheet(
            "background: #5B8AD4; color: #0A0C12; border-radius: 0px;"
            " font-size: 10px; font-weight: 700; font-family: 'Consolas', monospace;");
        topRow->addWidget(badge);
    }

    l->addLayout(topRow);

    if (!preview.isEmpty()) {
        auto *previewLabel = new QLabel(preview, this);
        previewLabel->setStyleSheet(
            "color: #283048; font-size: 10px; font-family: 'Consolas', monospace;"
            " background: transparent;");
        previewLabel->setMaximumWidth(180);
        l->addWidget(previewLabel);
    }
}
