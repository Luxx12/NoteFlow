#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QFont>
#include "ConnectDialog.h"
#include "MainWindow.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    QFont monoFont("Consolas", 10);
    monoFont.setStyleHint(QFont::Monospace);
    app.setFont(monoFont);

    QPalette dark;
    dark.setColor(QPalette::Window,          QColor(0x0A, 0x0C, 0x12));
    dark.setColor(QPalette::WindowText,      QColor(0xA8, 0xB8, 0xD0));
    dark.setColor(QPalette::Base,            QColor(0x0A, 0x0C, 0x12));
    dark.setColor(QPalette::AlternateBase,   QColor(0x0E, 0x11, 0x1A));
    dark.setColor(QPalette::Text,            QColor(0xA8, 0xB8, 0xD0));
    dark.setColor(QPalette::Button,          QColor(0x0E, 0x11, 0x1A));
    dark.setColor(QPalette::ButtonText,      QColor(0xA8, 0xB8, 0xD0));
    dark.setColor(QPalette::BrightText,      QColor(0x5B, 0x8A, 0xD4));
    dark.setColor(QPalette::Highlight,       QColor(0x14, 0x1A, 0x28));
    dark.setColor(QPalette::HighlightedText, QColor(0x5B, 0x8A, 0xD4));
    app.setPalette(dark);

    ConnectDialog dlg;
    if (dlg.exec() != QDialog::Accepted) return 0;

    MainWindow win(dlg.serverUrl(), dlg.displayName());
    win.showMaximized();
    return app.exec();
}
