#include "ConnectDialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

ConnectDialog::ConnectDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("NoteFlow");
    setFixedSize(340, 180);
    setStyleSheet(
        "QDialog { background: #0A0C12; border: 2px solid #1A2030; }"
        "QLabel  { color: #4A5670; font-size: 10px; font-weight: 600;"
        "           letter-spacing: 0.15em; font-family: 'Consolas', monospace; }");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(0);

    auto *title = new QLabel("NOTEFLOW", this);
    title->setStyleSheet(
        "color: #5B8AD4; font-size: 15px; font-weight: 700;"
        " letter-spacing: 0.18em; font-family: 'Consolas', monospace;");
    layout->addWidget(title);
    layout->addSpacing(28);

    auto *nameLabel = new QLabel("DISPLAY NAME", this);
    layout->addWidget(nameLabel);
    layout->addSpacing(4);

    m_nameInput = new QLineEdit(this);
    m_nameInput->setPlaceholderText("enter your name");
    m_nameInput->setFixedHeight(34);
    m_nameInput->setStyleSheet(
        "QLineEdit {"
        "  background: #080A0E; border: 1px solid #1A2030; border-radius: 0px;"
        "  color: #A8B8D0; font-size: 13px; font-family: 'Consolas', monospace;"
        "  padding: 0 10px;"
        "}"
        "QLineEdit:focus { border-color: #5B8AD4; }"
        "QLineEdit::placeholder { color: #1A2030; }");
    layout->addWidget(m_nameInput);
    layout->addSpacing(24);

    auto *btn = new QPushButton("CONNECT", this);
    btn->setFixedHeight(36);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QPushButton {"
        "  background: #101828; border: 1px solid #5B8AD4; border-radius: 0px;"
        "  color: #5B8AD4; font-size: 12px; font-weight: 700;"
        "  letter-spacing: 0.15em; font-family: 'Consolas', monospace;"
        "}"
        "QPushButton:hover { background: #1A2438; color: #7AAAE8; }"
        "QPushButton:pressed { background: #0A0E18; }");
    layout->addWidget(btn);

    connect(btn,         &QPushButton::clicked,     this, &ConnectDialog::onConnect);
    connect(m_nameInput, &QLineEdit::returnPressed, this, &ConnectDialog::onConnect);
}

QString ConnectDialog::serverUrl() const
{
    return "wss://noteflow-production-cd83.up.railway.app/";
}

QString ConnectDialog::displayName() const
{
    return m_nameInput->text().trimmed();
}

void ConnectDialog::onConnect()
{
    if (m_nameInput->text().trimmed().isEmpty()) return;
    accept();
}
