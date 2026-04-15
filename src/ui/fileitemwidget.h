#ifndef FILEITEMWIDGET_H
#define FILEITEMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>

class fileItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit fileItemWidget(QWidget *parent = nullptr, const QString &path = "");

signals:
    void fileSelected(const QString &path);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QString filePath;   // value, not pointer — no leak
    QLabel *label;
};

#endif // FILEITEMWIDGET_H
