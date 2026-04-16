#include "fileitemwidget.h"
#include <QFileInfo>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

fileItemWidget::fileItemWidget(QWidget *parent, const QString &path)
    : QWidget(parent), filePath(path)
{
    QFileInfo info(path);

    setFixedHeight(30);
    setStyleSheet(
        "QWidget {"
        "  border-bottom: 1px solid #121828;"
        "  background: transparent;"
        "}");

    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(16, 0, 16, 0);
    l->setSpacing(2);

    auto *topRow = new QHBoxLayout();
    topRow->setContentsMargins(0, 0, 0, 0);

    label = new QLabel(info.fileName(), this);
    label->setStyleSheet(
        "color: #6878A0; font-size: 11px; font-weight: 600;"
        " font-family: 'Consolas', monospace; background: transparent;");

    l->addLayout(topRow);
    topRow->addWidget(label);
    setCursor(Qt::PointingHandCursor);
}

void fileItemWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit fileSelected(filePath);
    }
    QWidget::mouseReleaseEvent(event);
}
