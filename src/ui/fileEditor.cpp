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
    font.setFamilies({"Consolas", "Menlo", "Courier New"});
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(11);

    root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    // ── Top bar ──
    QFrame *topBar = new QFrame();
    topBar->setFixedHeight(48);
    topBar->setStyleSheet(
        "QFrame { background: #0E1018; border-bottom: 2px solid #1A2030; }");

    topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(20, 0, 20, 0);
    topLayout->setSpacing(10);

    fileLabel = new QLabel("no file", topBar);
    fileLabel->setStyleSheet(
        "color: #5B8AD4; font-size: 12px; font-weight: 700;"
        " letter-spacing: 0.06em; font-family: 'Consolas', monospace;");

    saveBtn = new QPushButton("SAVE", topBar);
    saveBtn->setFixedSize(52, 26);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setEnabled(false);
    saveBtn->setStyleSheet(
        "QPushButton {"
        "  background: #101828; border: 1px solid #1A2030; border-radius: 0px;"
        "  color: #4A5670; font-size: 10px; font-weight: 700;"
        "  letter-spacing: 0.1em; font-family: 'Consolas', monospace;"
        "}"
        "QPushButton:hover { border-color: #5B8AD4; color: #5B8AD4; }"
        "QPushButton:pressed { background: #0A0E18; }"
        "QPushButton:disabled { color: #121828; border-color: #121828; background: #0C0E16; }");

    topLayout->addWidget(fileLabel);
    topLayout->addStretch();
    topLayout->addWidget(saveBtn);

    // ── QScintilla editor ──
    editor = new QsciScintilla(this);
    editor->setFont(font);
    editor->setMarginsFont(font);

    editor->setMarginType(0, QsciScintilla::NumberMargin);
    editor->setMarginLineNumbers(0, true);
    editor->setMarginsForegroundColor(QColor(0x4A, 0x56, 0x70));
    editor->setMarginsBackgroundColor(QColor(0x0C, 0x0E, 0x16));
    editor->setMarginWidth(0, "00000");
    editor->setMarginType(1, QsciScintilla::SymbolMargin);
    editor->setMarginWidth(1, 14);

    editor->setCaretLineVisible(true);
    editor->setCaretWidth(2);
    editor->setCaretForegroundColor(QColor(0x5B, 0x8A, 0xD4));
    editor->setCaretLineBackgroundColor(QColor(0x0E, 0x10, 0x1A));

    editor->setPaper(QColor(0x0A, 0x0C, 0x12));
    editor->setColor(QColor(0xA8, 0xB8, 0xD0));

    editor->setTabWidth(4);
    editor->setIndentationsUseTabs(false);
    editor->setBackspaceUnindents(true);
    editor->setAutoIndent(true);
    editor->setBraceMatching(QsciScintilla::SloppyBraceMatch);

    editor->setMatchedBraceBackgroundColor(QColor(0x1A, 0x20, 0x30));
    editor->setMatchedBraceForegroundColor(QColor(0x5B, 0x8A, 0xD4));

    root->addWidget(topBar);
    root->addWidget(editor);

    // ── Connections ──
    connect(editor, SIGNAL(SCN_MODIFIED(int,int,const char*,int,int,int,int,int,int,int)),
            this,   SLOT(onScintillaModified(int,int,const char*,int,int,int,int,int,int,int)));

    connect(saveBtn, &QPushButton::clicked, this, &fileEditor::saveFile);

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

// ── Syntax highlighting (cool blue palette) ──────────────────────────────────

void fileEditor::applyDarkTheme(QsciLexer *lexer)
{
    lexer->setDefaultPaper(QColor(0x0A, 0x0C, 0x12));
    lexer->setDefaultColor(QColor(0xA8, 0xB8, 0xD0));
    lexer->setDefaultFont(font);

    for (int i = 0; i <= 128; ++i) {
        lexer->setPaper(QColor(0x0A, 0x0C, 0x12), i);
        lexer->setFont(font, i);
    }

    editor->setLexer(lexer);

    editor->setMarginsBackgroundColor(QColor(0x0C, 0x0E, 0x16));
    editor->setMarginsForegroundColor(QColor(0x4A, 0x56, 0x70));
}

void fileEditor::setLexerForExtension(const QString &extension)
{
    QsciLexer *lexer = nullptr;

    // Cool blue syntax palette:
    // Keywords:    #5B8AD4  (blue accent)
    // Comments:    #3A4458  (muted slate)
    // Strings:     #7A9EC4  (light steel)
    // Numbers:     #8AACD4  (pale blue)
    // Preproc:     #4A6A9E  (deep blue)
    // Identifiers: #A8B8D0  (blue-white)
    // Operators:   #7888A4  (cool gray)
    // Functions:   #7AAAE8  (bright blue)

    if (extension == "cpp" || extension == "c" || extension == "cc"
        || extension == "cxx" || extension == "h" || extension == "hpp"
        || extension == "hxx" || extension == "m" || extension == "mm") {
        auto *l = new QsciLexerCPP(this);
        l->setColor(QColor(0x5B, 0x8A, 0xD4), QsciLexerCPP::Keyword);
        l->setColor(QColor(0x3A, 0x44, 0x58), QsciLexerCPP::Comment);
        l->setColor(QColor(0x3A, 0x44, 0x58), QsciLexerCPP::CommentLine);
        l->setColor(QColor(0x3A, 0x44, 0x58), QsciLexerCPP::CommentDoc);
        l->setColor(QColor(0x7A, 0x9E, 0xC4), QsciLexerCPP::DoubleQuotedString);
        l->setColor(QColor(0x7A, 0x9E, 0xC4), QsciLexerCPP::SingleQuotedString);
        l->setColor(QColor(0x8A, 0xAC, 0xD4), QsciLexerCPP::Number);
        l->setColor(QColor(0x4A, 0x6A, 0x9E), QsciLexerCPP::PreProcessor);
        l->setColor(QColor(0xA8, 0xB8, 0xD0), QsciLexerCPP::Identifier);
        l->setColor(QColor(0x78, 0x88, 0xA4), QsciLexerCPP::Operator);
        lexer = l;
    }
    else if (extension == "java" || extension == "cs") {
        auto *l = new QsciLexerJava(this);
        l->setColor(QColor(0x5B, 0x8A, 0xD4), QsciLexerCPP::Keyword);
        l->setColor(QColor(0x3A, 0x44, 0x58), QsciLexerCPP::Comment);
        l->setColor(QColor(0x3A, 0x44, 0x58), QsciLexerCPP::CommentLine);
        l->setColor(QColor(0x7A, 0x9E, 0xC4), QsciLexerCPP::DoubleQuotedString);
        l->setColor(QColor(0x8A, 0xAC, 0xD4), QsciLexerCPP::Number);
        lexer = l;
    }
    else if (extension == "js" || extension == "jsx" || extension == "ts"
             || extension == "tsx" || extension == "mjs" || extension == "cjs") {
        auto *l = new QsciLexerJavaScript(this);
        l->setColor(QColor(0x5B, 0x8A, 0xD4), QsciLexerCPP::Keyword);
        l->setColor(QColor(0x3A, 0x44, 0x58), QsciLexerCPP::Comment);
        l->setColor(QColor(0x3A, 0x44, 0x58), QsciLexerCPP::CommentLine);
        l->setColor(QColor(0x7A, 0x9E, 0xC4), QsciLexerCPP::DoubleQuotedString);
        l->setColor(QColor(0x7A, 0x9E, 0xC4), QsciLexerCPP::SingleQuotedString);
        l->setColor(QColor(0x8A, 0xAC, 0xD4), QsciLexerCPP::Number);
        lexer = l;
    }
    else if (extension == "py" || extension == "pyw") {
        auto *l = new QsciLexerPython(this);
        l->setColor(QColor(0x5B, 0x8A, 0xD4), QsciLexerPython::Keyword);
        l->setColor(QColor(0x3A, 0x44, 0x58), QsciLexerPython::Comment);
        l->setColor(QColor(0x7A, 0x9E, 0xC4), QsciLexerPython::DoubleQuotedString);
        l->setColor(QColor(0x7A, 0x9E, 0xC4), QsciLexerPython::SingleQuotedString);
        l->setColor(QColor(0x7A, 0x9E, 0xC4), QsciLexerPython::TripleDoubleQuotedString);
        l->setColor(QColor(0x7A, 0x9E, 0xC4), QsciLexerPython::TripleSingleQuotedString);
        l->setColor(QColor(0x8A, 0xAC, 0xD4), QsciLexerPython::Number);
        l->setColor(QColor(0x4A, 0x6A, 0x9E), QsciLexerPython::Decorator);
        l->setColor(QColor(0x7A, 0xAA, 0xE8), QsciLexerPython::FunctionMethodName);
        lexer = l;
    }
    else if (extension == "html" || extension == "htm") {
        lexer = new QsciLexerHTML(this);
    }
    else if (extension == "css" || extension == "scss" || extension == "less") {
        lexer = new QsciLexerCSS(this);
    }
    else if (extension == "json") {
        lexer = new QsciLexerJSON(this);
    }
    else if (extension == "xml" || extension == "svg" || extension == "xsl"
             || extension == "xsd" || extension == "ui") {
        lexer = new QsciLexerXML(this);
    }
    else if (extension == "sql") {
        lexer = new QsciLexerSQL(this);
    }
    else if (extension == "sh" || extension == "bash" || extension == "zsh") {
        lexer = new QsciLexerBash(this);
    }
    else if (extension == "md" || extension == "markdown") {
        lexer = new QsciLexerMarkdown(this);
    }
    else if (extension == "rb" || extension == "rake" || extension == "gemspec") {
        lexer = new QsciLexerRuby(this);
    }
    else if (extension == "lua") {
        lexer = new QsciLexerLua(this);
    }
    else {
        editor->setLexer(nullptr);
        editor->setPaper(QColor(0x0A, 0x0C, 0x12));
        editor->setColor(QColor(0xA8, 0xB8, 0xD0));
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
