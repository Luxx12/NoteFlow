#include "CodeViewer.h"
#include "SyntaxHighlighter.h"

#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QWidget>
#include <QFont>
#include <QFontDatabase>
#include <QClipboard>
#include <QApplication>
#include <QStatusBar>

CodeViewer::CodeViewer(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Code Viewer");
    setMinimumSize(800, 560);
    resize(1000, 660);
    buildUI();
}

void CodeViewer::buildUI()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);
    central->setStyleSheet("background: #1E1E1E;");

    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Top bar ──────────────────────────────────────────
    auto *topBar = new QWidget(central);
    topBar->setFixedHeight(52);
    topBar->setObjectName("topBar");
    topBar->setStyleSheet(
        "#topBar { background: #111111; border-bottom: 1px solid #1A1A1A; }");

    auto *tl = new QHBoxLayout(topBar);
    tl->setContentsMargins(16, 0, 16, 0);
    tl->setSpacing(10);

    auto *openBtn = new QPushButton("Open", topBar);
    openBtn->setFixedSize(68, 30);
    openBtn->setCursor(Qt::PointingHandCursor);
    openBtn->setStyleSheet(
        "QPushButton {"
        "  background: #2A2A2A; border: 1px solid #363636; border-radius: 5px;"
        "  color: #C0C0C0; font-size: 12px; font-weight: 600;"
        "  letter-spacing: 0.04em;"
        "}"
        "QPushButton:hover { background: #333333; border-color: #484848; color: #E0E0E0; }"
        "QPushButton:pressed { background: #222222; }");

    auto *copyBtn = new QPushButton("Copy", topBar);
    copyBtn->setFixedSize(58, 30);
    copyBtn->setCursor(Qt::PointingHandCursor);
    copyBtn->setStyleSheet(
        "QPushButton {"
        "  background: transparent; border: 1px solid #2A2A2A; border-radius: 5px;"
        "  color: #555555; font-size: 12px; font-weight: 600;"
        "  letter-spacing: 0.04em;"
        "}"
        "QPushButton:hover { border-color: #404040; color: #C0C0C0; }"
        "QPushButton:pressed { background: #1A1A1A; }");

    // Divider
    auto *divider = new QFrame(topBar);
    divider->setFixedSize(1, 20);
    divider->setStyleSheet("background: #252525;");

    // File info labels — all explicitly transparent so palette doesn't bleed through
    m_fileLabel = new QLabel("no file open", topBar);
    m_fileLabel->setStyleSheet(
        "color: #666666; font-size: 12px; background: transparent;");

    m_langLabel = new QLabel("", topBar);
    m_langLabel->setStyleSheet(
        "color: #3A3A3A; font-size: 10px; font-weight: 600; "
        "letter-spacing: 0.10em; background: transparent;");

    m_lineLabel = new QLabel("", topBar);
    m_lineLabel->setStyleSheet(
        "color: #3A3A3A; font-size: 10px; background: transparent;");

    tl->addWidget(openBtn);
    tl->addWidget(copyBtn);
    tl->addWidget(divider);
    tl->addSpacing(4);
    tl->addWidget(m_fileLabel);
    tl->addStretch();
    tl->addWidget(m_langLabel);
    tl->addSpacing(16);
    tl->addWidget(m_lineLabel);

    root->addWidget(topBar);

    // ── Editor ───────────────────────────────────────────
    m_editor = new QPlainTextEdit(central);
    m_editor->setReadOnly(true);
    m_editor->setLineWrapMode(QPlainTextEdit::NoWrap);

    // Use a monospace font
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(11);
    m_editor->setFont(font);

    m_editor->setStyleSheet(
        "QPlainTextEdit {"
        "  background: #1E1E1E; color: #D4D4D4;"
        "  border: none; padding: 12px;"
        "  selection-background-color: #264F78;"
        "}"
        "QScrollBar:vertical {"
        "  background: #1E1E1E; width: 10px; border-radius: 5px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #3A3A3A; border-radius: 5px; min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover { background: #505050; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar:horizontal {"
        "  background: #1E1E1E; height: 10px; border-radius: 5px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "  background: #3A3A3A; border-radius: 5px; min-width: 20px;"
        "}"
        "QScrollBar::handle:horizontal:hover { background: #505050; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }");

    m_highlighter = new SyntaxHighlighter(m_editor->document());

    root->addWidget(m_editor);

    // ── Status bar ───────────────────────────────────────
    statusBar()->setStyleSheet(
        "QStatusBar { background: #111111; color: #444444; font-size: 11px;"
        "border-top: 1px solid #1A1A1A; }");
    statusBar()->showMessage("Open a file to get started");

    connect(openBtn, &QPushButton::clicked, this, &CodeViewer::onOpenFile);
    connect(copyBtn, &QPushButton::clicked, this, &CodeViewer::onCopyAll);
}

void CodeViewer::onOpenFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, "Open Code File", QString(),
        "Code Files (*.cpp *.h *.c *.cc *.hpp *.cxx *.py *.js *.ts *.jsx *.tsx "
        "*.java *.cs *.go *.rs *.rb *.php *.swift *.kt *.json *.xml *.html "
        "*.css *.scss *.md *.txt *.sh *.bat *.cmake *.pro);;"
        "All Files (*)");

    if (!path.isEmpty())
        openFile(path);
}

void CodeViewer::openFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        statusBar()->showMessage("Failed to open: " + filePath);
        return;
    }

    QTextStream in(&file);
    const QString content = in.readAll();
    file.close();

    QFileInfo info(filePath);
    const QString ext  = info.suffix().toLower();
    const int     lines = content.count('\n') + 1;

    m_highlighter->setLanguage(ext);
    m_editor->setPlainText(content);

    updateHeader(info.fileName(), ext, lines);
    setWindowTitle("Code Viewer — " + info.fileName());
    statusBar()->showMessage("Opened: " + filePath);
}

void CodeViewer::onCopyAll()
{
    if (m_editor->toPlainText().isEmpty()) return;
    QApplication::clipboard()->setText(m_editor->toPlainText());
    statusBar()->showMessage("Copied to clipboard");
}

void CodeViewer::updateHeader(const QString &fileName, const QString &extension, int lines)
{
    m_fileLabel->setText(fileName);
    m_langLabel->setText(detectLanguage(extension).toUpper());
    m_lineLabel->setText(QString::number(lines) + " lines");
}

QString CodeViewer::detectLanguage(const QString &extension) const
{
    static const QMap<QString, QString> map = {
                                               {"cpp","C++"}, {"h","C/C++ Header"}, {"c","C"}, {"cc","C++"},
                                               {"hpp","C++ Header"}, {"cxx","C++"},
                                               {"py","Python"}, {"js","JavaScript"}, {"ts","TypeScript"},
                                               {"jsx","React"}, {"tsx","React/TS"}, {"java","Java"},
                                               {"cs","C#"}, {"go","Go"}, {"rs","Rust"}, {"rb","Ruby"},
                                               {"php","PHP"}, {"swift","Swift"}, {"kt","Kotlin"},
                                               {"json","JSON"}, {"xml","XML"}, {"html","HTML"},
                                               {"css","CSS"}, {"scss","SCSS"}, {"md","Markdown"},
                                               {"sh","Shell"}, {"bat","Batch"}, {"txt","Text"},
                                               };
    return map.value(extension, extension.isEmpty() ? "Unknown" : extension);
}
