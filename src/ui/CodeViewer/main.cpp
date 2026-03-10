#include <QApplication>
#include <QPalette>
#include <QColor>
#include "CodeViewer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    // Dark palette
    QPalette dark;
    dark.setColor(QPalette::Window,          QColor(0x1E, 0x1E, 0x1E));
    dark.setColor(QPalette::WindowText,      QColor(0xD4, 0xD4, 0xD4));
    dark.setColor(QPalette::Base,            QColor(0x1E, 0x1E, 0x1E));
    dark.setColor(QPalette::AlternateBase,   QColor(0x2A, 0x2A, 0x2A));
    dark.setColor(QPalette::Text,            QColor(0xD4, 0xD4, 0xD4));
    dark.setColor(QPalette::Button,          QColor(0x2A, 0x2A, 0x2A));
    dark.setColor(QPalette::ButtonText,      QColor(0xD4, 0xD4, 0xD4));
    dark.setColor(QPalette::Highlight,       QColor(0x26, 0x4F, 0x78));
    dark.setColor(QPalette::HighlightedText, QColor(0xFF, 0xFF, 0xFF));
    app.setPalette(dark);

    CodeViewer viewer;

    // Accept an optional file path as a command line argument
    // so the chat app can launch it with: codeviewer.exe path/to/file.cpp
    const QStringList args = app.arguments();
    if (args.size() > 1)
        viewer.openFile(args.at(1));

    viewer.show();
    return app.exec();
}
