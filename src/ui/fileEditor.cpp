#include "fileEditor.h"

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QFont>
#include <QShortcut>
#include <QKeySequence>
#include <QDebug>

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexercss.h>
#include <Qsci/qscilexerbash.h>
#include <Qsci/qscilexerjava.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexersql.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscilexerruby.h>
#include <Qsci/qscilexerjson.h>
#include <Qsci/qscilexermarkdown.h>

fileEditor::fileEditor(QWidget *parent) : QWidget(parent)
{
    font.setFamilies({"Consolas", "Menlo"});
    font.setStyleHint(QFont::Monospace);

    root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    // ── Top bar ──
    QFrame *topBar = new QFrame();
    topBar->setFixedHeight(56);
    topBar->setStyleSheet(
        "QFrame { background: #111111; border-bottom: 1px solid #1E1E1E; }");

    topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(20, 0, 20, 0);
    topLayout->setSpacing(10);

    fileLabel = new QLabel("no file", topBar);
    fileLabel->setStyleSheet(
        "color: #E0E0E0; font-size: 14px; font-weight: 500; letter-spacing: 0.04em;");

    saveBtn = new QPushButton("Save", topBar);
    saveBtn->setFixedSize(52, 28);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setEnabled(false);
    saveBtn->setStyleSheet(
        "QPushButton {"
        "  background: #2A2A2A; border: 1px solid #383838; border-radius: 4px;"
        "  color: #C0C0C0; font-size: 11px; font-weight: 600;"
        "}"
        "QPushButton:hover { background: #333333; color: #E0E0E0; }"
        "QPushButton:pressed { background: #222222; }"
        "QPushButton:disabled { color: #2E2E2E; border-color: #1E1E1E; background: #181818; }");

    topLayout->addWidget(fileLabel);
    topLayout->addStretch();
    topLayout->addWidget(saveBtn);

    // ── QScintilla editor ──
    editor = new QsciScintilla(this);
    editor->setFont(font);
    editor->setMarginsFont(font);

    editor->setMarginType(0, QsciScintilla::NumberMargin);
    editor->setMarginLineNumbers(0, true);
    editor->setMarginsForegroundColor(QColor(0x88, 0x88, 0x88));
    editor->setMarginsBackgroundColor(QColor(0x11, 0x11, 0x11));
    editor->setMarginWidth(0, "00000");
    editor->setMarginType(1, QsciScintilla::SymbolMargin);
    editor->setMarginWidth(1, 18);

    editor->setCaretLineVisible(true);
    editor->setCaretWidth(2);
    editor->setCaretForegroundColor(QColor(0xE0, 0xE0, 0xE0));
    editor->setCaretLineBackgroundColor(QColor(0x1A, 0x1A, 0x1A));

    editor->setPaper(QColor(0x0F, 0x0F, 0x0F));
    editor->setColor(QColor(0xE0, 0xE0, 0xE0));

    editor->setTabWidth(4);
    editor->setIndentationsUseTabs(false);
    editor->setBackspaceUnindents(true);
    editor->setAutoIndent(true);
    editor->setBraceMatching(QsciScintilla::SloppyBraceMatch);

    root->addWidget(topBar);
    root->addWidget(editor);

    // ── Connections ──
    connect(editor, SIGNAL(SCN_MODIFIED(int,int,const char*,int,int,int,int,int,int,int)),
            this,   SLOT(onScintillaModified(int,int,const char*,int,int,int,int,int,int,int)));

    connect(saveBtn, &QPushButton::clicked, this, &fileEditor::saveFile);

    // Ctrl+S shortcut
    auto *saveShortcut = new QShortcut(QKeySequence::Save, this);
    connect(saveShortcut, &QShortcut::activated, this, &fileEditor::saveFile);
}

// ── Public accessors ─────────────────────────────────────────────────────────

QString fileEditor::currentFileName() const
{
    return fileLabel->text();
}

QString fileEditor::currentFilePath() const
{
    return m_filePath;
}

// ── Load / New / Save ────────────────────────────────────────────────────────

void fileEditor::loadFile(const QString &path)
{
    if (path.isEmpty()) return;

    QFile file(path);
    QFileInfo info(path);

    if (file.open(QIODevice::ReadOnly)) {
        editor->read(&file);
        file.close();
    } else {
        qDebug() << "Failed to open file:" << path;
        return;
    }

    m_filePath = path;
    ext = info.suffix().toLower();
    fileLabel->setText(info.fileName());
    saveBtn->setEnabled(true);

    setLexerForExtension(ext);
}

void fileEditor::newFile(const QString &filename)
{
    fileLabel->setText(filename);
    editor->clear();
    m_filePath.clear();
    saveBtn->setEnabled(false);
}

void fileEditor::saveFile()
{
    if (m_filePath.isEmpty()) return;

    QFile file(m_filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QByteArray content = editor->text().toUtf8();
        file.write(content);
        file.close();
        qDebug() << "Saved:" << m_filePath;
    } else {
        qDebug() << "Failed to save file:" << m_filePath;
    }
}

// ── Syntax highlighting ──────────────────────────────────────────────────────

void fileEditor::applyDarkTheme(QsciLexer *lexer)
{
    lexer->setDefaultPaper(QColor(0x0F, 0x0F, 0x0F));
    lexer->setDefaultColor(QColor(0xE0, 0xE0, 0xE0));
    lexer->setDefaultFont(font);

    for (int i = 0; i <= 128; ++i) {
        lexer->setPaper(QColor(0x0F, 0x0F, 0x0F), i);
        lexer->setFont(font, i);
    }

    editor->setLexer(lexer);

    // Re-apply margin colors after lexer change (lexer can reset them)
    editor->setMarginsBackgroundColor(QColor(0x11, 0x11, 0x11));
    editor->setMarginsForegroundColor(QColor(0x88, 0x88, 0x88));
}

void fileEditor::setLexerForExtension(const QString &extension)
{
    QsciLexer *lexer = nullptr;

    // ── C / C++ / Objective-C ──
    if (extension == "cpp" || extension == "c" || extension == "cc"
        || extension == "cxx" || extension == "h" || extension == "hpp"
        || extension == "hxx" || extension == "m" || extension == "mm") {
        auto *l = new QsciLexerCPP(this);
        l->setColor(QColor(0x56, 0x9C, 0xD6), QsciLexerCPP::Keyword);
        l->setColor(QColor(0x6A, 0x99, 0x55), QsciLexerCPP::Comment);
        l->setColor(QColor(0x6A, 0x99, 0x55), QsciLexerCPP::CommentLine);
        l->setColor(QColor(0x6A, 0x99, 0x55), QsciLexerCPP::CommentDoc);
        l->setColor(QColor(0xCE, 0x91, 0x78), QsciLexerCPP::DoubleQuotedString);
        l->setColor(QColor(0xCE, 0x91, 0x78), QsciLexerCPP::SingleQuotedString);
        l->setColor(QColor(0xB5, 0xCE, 0xA8), QsciLexerCPP::Number);
        l->setColor(QColor(0xC5, 0x86, 0xC0), QsciLexerCPP::PreProcessor);
        l->setColor(QColor(0x9C, 0xDC, 0xFE), QsciLexerCPP::Identifier);
        l->setColor(QColor(0xD4, 0xD4, 0xD4), QsciLexerCPP::Operator);
        lexer = l;
    }
    // ── Java / C# ──
    else if (extension == "java" || extension == "cs") {
        auto *l = new QsciLexerJava(this);
        l->setColor(QColor(0x56, 0x9C, 0xD6), QsciLexerCPP::Keyword);
        l->setColor(QColor(0x6A, 0x99, 0x55), QsciLexerCPP::Comment);
        l->setColor(QColor(0x6A, 0x99, 0x55), QsciLexerCPP::CommentLine);
        l->setColor(QColor(0xCE, 0x91, 0x78), QsciLexerCPP::DoubleQuotedString);
        l->setColor(QColor(0xB5, 0xCE, 0xA8), QsciLexerCPP::Number);
        lexer = l;
    }
    // ── JavaScript / TypeScript ──
    else if (extension == "js" || extension == "jsx" || extension == "ts"
             || extension == "tsx" || extension == "mjs" || extension == "cjs") {
        auto *l = new QsciLexerJavaScript(this);
        l->setColor(QColor(0x56, 0x9C, 0xD6), QsciLexerCPP::Keyword);
        l->setColor(QColor(0x6A, 0x99, 0x55), QsciLexerCPP::Comment);
        l->setColor(QColor(0x6A, 0x99, 0x55), QsciLexerCPP::CommentLine);
        l->setColor(QColor(0xCE, 0x91, 0x78), QsciLexerCPP::DoubleQuotedString);
        l->setColor(QColor(0xCE, 0x91, 0x78), QsciLexerCPP::SingleQuotedString);
        l->setColor(QColor(0xB5, 0xCE, 0xA8), QsciLexerCPP::Number);
        lexer = l;
    }
    // ── Python ──
    else if (extension == "py" || extension == "pyw") {
        auto *l = new QsciLexerPython(this);
        l->setColor(QColor(0x56, 0x9C, 0xD6), QsciLexerPython::Keyword);
        l->setColor(QColor(0x6A, 0x99, 0x55), QsciLexerPython::Comment);
        l->setColor(QColor(0xCE, 0x91, 0x78), QsciLexerPython::DoubleQuotedString);
        l->setColor(QColor(0xCE, 0x91, 0x78), QsciLexerPython::SingleQuotedString);
        l->setColor(QColor(0xCE, 0x91, 0x78), QsciLexerPython::TripleDoubleQuotedString);
        l->setColor(QColor(0xCE, 0x91, 0x78), QsciLexerPython::TripleSingleQuotedString);
        l->setColor(QColor(0xB5, 0xCE, 0xA8), QsciLexerPython::Number);
        l->setColor(QColor(0x4E, 0xC9, 0xB0), QsciLexerPython::Decorator);
        l->setColor(QColor(0xDC, 0xDC, 0xAA), QsciLexerPython::FunctionMethodName);
        lexer = l;
    }
    // ── HTML ──
    else if (extension == "html" || extension == "htm") {
        lexer = new QsciLexerHTML(this);
    }
    // ── CSS ──
    else if (extension == "css" || extension == "scss" || extension == "less") {
        lexer = new QsciLexerCSS(this);
    }
    // ── JSON ──
    else if (extension == "json") {
        lexer = new QsciLexerJSON(this);
    }
    // ── XML ──
    else if (extension == "xml" || extension == "svg" || extension == "xsl"
             || extension == "xsd" || extension == "ui") {
        lexer = new QsciLexerXML(this);
    }
    // ── SQL ──
    else if (extension == "sql") {
        lexer = new QsciLexerSQL(this);
    }
    // ── Shell / Bash ──
    else if (extension == "sh" || extension == "bash" || extension == "zsh") {
        lexer = new QsciLexerBash(this);
    }
    // ── Markdown ──
    else if (extension == "md" || extension == "markdown") {
        lexer = new QsciLexerMarkdown(this);
    }
    // ── Ruby ──
    else if (extension == "rb" || extension == "rake" || extension == "gemspec") {
        lexer = new QsciLexerRuby(this);
    }
    // ── Lua ──
    else if (extension == "lua") {
        lexer = new QsciLexerLua(this);
    }
    // ── Unknown — plain text, no lexer ──
    else {
        editor->setLexer(nullptr);
        editor->setPaper(QColor(0x0F, 0x0F, 0x0F));
        editor->setColor(QColor(0xE0, 0xE0, 0xE0));
        return;
    }

    applyDarkTheme(lexer);
}

// ── Collaborative editing ────────────────────────────────────────────────────

void fileEditor::onScintillaModified(int position, int modificationType,
                                     const char *text, int length,
                                     int /*linesAdded*/, int /*line*/,
                                     int /*foldLevelNow*/, int /*foldLevelPrev*/,
                                     int /*token*/, int /*annotationLinesAdded*/)
{
    if (m_isRemoteEdit) return;

    bool isAddition = (modificationType & QsciScintilla::SC_MOD_INSERTTEXT);
    bool isDeletion = (modificationType & QsciScintilla::SC_MOD_DELETETEXT);

    if (isAddition || isDeletion) {
        QString changedText = QString::fromUtf8(text, length);
        QString currentFilename = fileLabel->text();
        emit localFileChanged(currentFilename, position, length, changedText, isAddition);
    }
}

void fileEditor::applyRemoteEdit(int position, int length,
                                 const QString &text, bool isAddition)
{
    m_isRemoteEdit = true;

    if (isAddition) {
        editor->SendScintilla(QsciScintilla::SCI_INSERTTEXT,
                              position, text.toUtf8().data());
    } else {
        editor->SendScintilla(QsciScintilla::SCI_DELETERANGE,
                              position, length);
    }

    m_isRemoteEdit = false;
}
