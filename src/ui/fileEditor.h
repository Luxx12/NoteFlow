#ifndef FILEEDITOR_H
#define FILEEDITOR_H

#include <QWidget>
#include <QFont>
#include <QString>
#include <Qsci/qsciscintilla.h>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QsciLexer;

class fileEditor : public QWidget
{
    Q_OBJECT
public:
    explicit fileEditor(QWidget *parent = nullptr);

    QString currentFileName() const;
    QString currentFilePath() const;

signals:
    void localFileChanged(const QString &filename, int position, int length,
                          const QString &text, bool isAddition);

public slots:
    void loadFile(const QString &path);
    void newFile(const QString &filename);
    void saveFile();
    void applyRemoteEdit(int position, int length, const QString &text, bool isAddition);

private slots:
    void onScintillaModified(int position, int modificationType, const char *text,
                             int length, int linesAdded, int line,
                             int foldLevelNow, int foldLevelPrev,
                             int token, int annotationLinesAdded);

private:
    void setLexerForExtension(const QString &ext);
    void applyDarkTheme(QsciLexer *lexer);

    QVBoxLayout   *root;
    QHBoxLayout   *topLayout;
    QLabel        *fileLabel;
    QPushButton   *saveBtn;
    QsciScintilla *editor;
    QFont          font;
    QString        m_filePath;
    QString        ext;
    bool           m_isRemoteEdit = false;
};

#endif // FILEEDITOR_H
