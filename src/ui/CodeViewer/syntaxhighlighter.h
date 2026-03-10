#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);
    void setLanguage(const QString &extension);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct Rule {
        QRegularExpression pattern;
        QTextCharFormat    format;
    };

    void buildCppRules();
    void buildPythonRules();
    void buildJsRules();
    void buildGenericRules();

    QVector<Rule>      m_rules;
    QTextCharFormat    m_multiLineCommentFormat;
    QRegularExpression m_commentStart;
    QRegularExpression m_commentEnd;
    bool               m_supportsMultiLineComments = false;
};
