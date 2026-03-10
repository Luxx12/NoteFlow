#pragma once
#include <QMainWindow>

class QPlainTextEdit;
class QLabel;
class SyntaxHighlighter;

class CodeViewer : public QMainWindow {
    Q_OBJECT
public:
    explicit CodeViewer(QWidget *parent = nullptr);
    void openFile(const QString &filePath);

private slots:
    void onOpenFile();
    void onCopyAll();

private:
    void buildUI();
    void updateHeader(const QString &fileName, const QString &extension, int lines);
    QString detectLanguage(const QString &extension) const;

    QPlainTextEdit  *m_editor      = nullptr;
    QLabel          *m_fileLabel   = nullptr;
    QLabel          *m_langLabel   = nullptr;
    QLabel          *m_lineLabel   = nullptr;
    SyntaxHighlighter *m_highlighter = nullptr;
};
