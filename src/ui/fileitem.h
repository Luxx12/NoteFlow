#ifndef FILEITEM_H
#define FILEITEM_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

class fileItem : public QWidget
{
    Q_OBJECT
public:
    explicit fileItem(QWidget *parent = nullptr);

signals:

private:
    QLabel *name;
    QString *filePath;
    QHBoxLayout *root;
};

#endif // FILEITEM_H
