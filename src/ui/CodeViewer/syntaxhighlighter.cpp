#include "SyntaxHighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{}

void SyntaxHighlighter::setLanguage(const QString &extension)
{
    m_rules.clear();
    m_supportsMultiLineComments = false;

    if (extension == "cpp" || extension == "h" || extension == "c" ||
        extension == "cc"  || extension == "hpp" || extension == "cxx") {
        buildCppRules();
    } else if (extension == "py") {
        buildPythonRules();
    } else if (extension == "js" || extension == "ts" || extension == "jsx" || extension == "tsx") {
        buildJsRules();
    } else {
        buildGenericRules();
    }

    rehighlight();
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    // Apply single-line rules
    for (const Rule &rule : m_rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Multi-line comment handling (C-style /* */)
    if (!m_supportsMultiLineComments) return;

    setCurrentBlockState(0);
    int startIndex = 0;

    if (previousBlockState() != 1)
        startIndex = text.indexOf(m_commentStart);

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = m_commentEnd.match(text, startIndex);
        int endIndex = endMatch.capturedStart();
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, m_multiLineCommentFormat);
        startIndex = text.indexOf(m_commentStart, startIndex + commentLength);
    }
}

void SyntaxHighlighter::buildCppRules()
{
    m_supportsMultiLineComments = true;
    m_commentStart = QRegularExpression("/\\*");
    m_commentEnd   = QRegularExpression("\\*/");

    Rule rule;

    // Keywords
    QTextCharFormat keywordFmt;
    keywordFmt.setForeground(QColor("#569CD6"));
    keywordFmt.setFontWeight(QFont::Bold);
    const QStringList keywords = {
        "auto","break","case","catch","class","const","constexpr","continue",
        "default","delete","do","else","enum","explicit","extern","false",
        "final","for","friend","if","inline","mutable","namespace","new",
        "noexcept","nullptr","operator","override","private","protected",
        "public","return","sizeof","static","struct","switch","template",
        "this","throw","true","try","typedef","typename","union","using",
        "virtual","void","volatile","while"
    };
    for (const QString &kw : keywords) {
        rule.pattern = QRegularExpression("\\b" + kw + "\\b");
        rule.format  = keywordFmt;
        m_rules.append(rule);
    }

    // Types
    QTextCharFormat typeFmt;
    typeFmt.setForeground(QColor("#4EC9B0"));
    const QStringList types = {
        "int","float","double","char","bool","long","short","unsigned","signed",
        "size_t","string","QString","QObject","QWidget","auto"
    };
    for (const QString &t : types) {
        rule.pattern = QRegularExpression("\\b" + t + "\\b");
        rule.format  = typeFmt;
        m_rules.append(rule);
    }

    // Preprocessor
    QTextCharFormat preprocFmt;
    preprocFmt.setForeground(QColor("#C586C0"));
    rule.pattern = QRegularExpression("^\\s*#\\s*\\w+");
    rule.format  = preprocFmt;
    m_rules.append(rule);

    // String literals
    QTextCharFormat stringFmt;
    stringFmt.setForeground(QColor("#CE9178"));
    rule.pattern = QRegularExpression("\".*?\"");
    rule.format  = stringFmt;
    m_rules.append(rule);

    // Char literals
    rule.pattern = QRegularExpression("'.'");
    rule.format  = stringFmt;
    m_rules.append(rule);

    // Numbers
    QTextCharFormat numberFmt;
    numberFmt.setForeground(QColor("#B5CEA8"));
    rule.pattern = QRegularExpression("\\b[0-9]+(\\.[0-9]+)?([eEfFlLuU]*)\\b");
    rule.format  = numberFmt;
    m_rules.append(rule);

    // Single-line comments //
    QTextCharFormat commentFmt;
    commentFmt.setForeground(QColor("#6A9955"));
    commentFmt.setFontItalic(true);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format  = commentFmt;
    m_rules.append(rule);

    // Multi-line comment format (used in highlightBlock)
    m_multiLineCommentFormat = commentFmt;

    // Function calls
    QTextCharFormat funcFmt;
    funcFmt.setForeground(QColor("#DCDCAA"));
    rule.pattern = QRegularExpression("\\b[A-Za-z_][A-Za-z0-9_]*(?=\\s*\\()");
    rule.format  = funcFmt;
    m_rules.append(rule);
}

void SyntaxHighlighter::buildPythonRules()
{
    Rule rule;

    // Keywords
    QTextCharFormat keywordFmt;
    keywordFmt.setForeground(QColor("#569CD6"));
    keywordFmt.setFontWeight(QFont::Bold);
    const QStringList keywords = {
        "and","as","assert","async","await","break","class","continue",
        "def","del","elif","else","except","False","finally","for","from",
        "global","if","import","in","is","lambda","None","nonlocal","not",
        "or","pass","raise","return","True","try","while","with","yield"
    };
    for (const QString &kw : keywords) {
        rule.pattern = QRegularExpression("\\b" + kw + "\\b");
        rule.format  = keywordFmt;
        m_rules.append(rule);
    }

    // Built-ins
    QTextCharFormat builtinFmt;
    builtinFmt.setForeground(QColor("#4EC9B0"));
    const QStringList builtins = {
        "print","len","range","int","str","float","list","dict","set",
        "tuple","type","isinstance","open","enumerate","zip","map","filter"
    };
    for (const QString &b : builtins) {
        rule.pattern = QRegularExpression("\\b" + b + "\\b");
        rule.format  = builtinFmt;
        m_rules.append(rule);
    }

    // Decorators
    QTextCharFormat decoratorFmt;
    decoratorFmt.setForeground(QColor("#C586C0"));
    rule.pattern = QRegularExpression("@[A-Za-z_][A-Za-z0-9_.]*");
    rule.format  = decoratorFmt;
    m_rules.append(rule);

    // Strings
    QTextCharFormat stringFmt;
    stringFmt.setForeground(QColor("#CE9178"));
    rule.pattern = QRegularExpression("\".*?\"|'.*?'");
    rule.format  = stringFmt;
    m_rules.append(rule);

    // Numbers
    QTextCharFormat numberFmt;
    numberFmt.setForeground(QColor("#B5CEA8"));
    rule.pattern = QRegularExpression("\\b[0-9]+(\\.[0-9]+)?\\b");
    rule.format  = numberFmt;
    m_rules.append(rule);

    // Comments
    QTextCharFormat commentFmt;
    commentFmt.setForeground(QColor("#6A9955"));
    commentFmt.setFontItalic(true);
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format  = commentFmt;
    m_rules.append(rule);

    // Function defs
    QTextCharFormat funcFmt;
    funcFmt.setForeground(QColor("#DCDCAA"));
    rule.pattern = QRegularExpression("\\bdef\\s+([A-Za-z_][A-Za-z0-9_]*)");
    rule.format  = funcFmt;
    m_rules.append(rule);
}

void SyntaxHighlighter::buildJsRules()
{
    m_supportsMultiLineComments = true;
    m_commentStart = QRegularExpression("/\\*");
    m_commentEnd   = QRegularExpression("\\*/");

    Rule rule;

    // Keywords
    QTextCharFormat keywordFmt;
    keywordFmt.setForeground(QColor("#569CD6"));
    keywordFmt.setFontWeight(QFont::Bold);
    const QStringList keywords = {
        "async","await","break","case","catch","class","const","continue",
        "debugger","default","delete","do","else","export","extends","false",
        "finally","for","from","function","if","import","in","instanceof",
        "let","new","null","of","return","static","super","switch","this",
        "throw","true","try","typeof","undefined","var","void","while","with","yield"
    };
    for (const QString &kw : keywords) {
        rule.pattern = QRegularExpression("\\b" + kw + "\\b");
        rule.format  = keywordFmt;
        m_rules.append(rule);
    }

    // Types (TypeScript)
    QTextCharFormat typeFmt;
    typeFmt.setForeground(QColor("#4EC9B0"));
    const QStringList types = {
        "string","number","boolean","any","void","never","unknown","interface","type","enum"
    };
    for (const QString &t : types) {
        rule.pattern = QRegularExpression("\\b" + t + "\\b");
        rule.format  = typeFmt;
        m_rules.append(rule);
    }

    // Strings
    QTextCharFormat stringFmt;
    stringFmt.setForeground(QColor("#CE9178"));
    rule.pattern = QRegularExpression("\".*?\"|'.*?'|`.*?`");
    rule.format  = stringFmt;
    m_rules.append(rule);

    // Numbers
    QTextCharFormat numberFmt;
    numberFmt.setForeground(QColor("#B5CEA8"));
    rule.pattern = QRegularExpression("\\b[0-9]+(\\.[0-9]+)?\\b");
    rule.format  = numberFmt;
    m_rules.append(rule);

    // Single-line comments
    QTextCharFormat commentFmt;
    commentFmt.setForeground(QColor("#6A9955"));
    commentFmt.setFontItalic(true);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format  = commentFmt;
    m_rules.append(rule);

    m_multiLineCommentFormat = commentFmt;

    // Function calls
    QTextCharFormat funcFmt;
    funcFmt.setForeground(QColor("#DCDCAA"));
    rule.pattern = QRegularExpression("\\b[A-Za-z_][A-Za-z0-9_]*(?=\\s*\\()");
    rule.format  = funcFmt;
    m_rules.append(rule);
}

void SyntaxHighlighter::buildGenericRules()
{
    Rule rule;

    // Strings
    QTextCharFormat stringFmt;
    stringFmt.setForeground(QColor("#CE9178"));
    rule.pattern = QRegularExpression("\".*?\"|'.*?'");
    rule.format  = stringFmt;
    m_rules.append(rule);

    // Numbers
    QTextCharFormat numberFmt;
    numberFmt.setForeground(QColor("#B5CEA8"));
    rule.pattern = QRegularExpression("\\b[0-9]+(\\.[0-9]+)?\\b");
    rule.format  = numberFmt;
    m_rules.append(rule);

    // Common comment styles
    QTextCharFormat commentFmt;
    commentFmt.setForeground(QColor("#6A9955"));
    commentFmt.setFontItalic(true);
    rule.pattern = QRegularExpression("//[^\n]*|#[^\n]*");
    rule.format  = commentFmt;
    m_rules.append(rule);
}
