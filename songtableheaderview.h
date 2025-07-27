#ifndef SONGTABLEHEADERVIEW_H
#define SONGTABLEHEADERVIEW_H

#include "common.h"


#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>

class SongTableHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    SongTableHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

public:
    virtual void mousePressEvent(QMouseEvent *event) override;

protected:
    virtual void paintSection(QPainter *painter, const QRect &rect, int logical_index) const override;

private:
    QSize button_size;
    QIcon download_icon;
    QIcon favourite_icon;
};

#endif // SONGTABLEHEADERVIEW_H

/*
 *     iconItem = new QStandardItem();
    iconItem->setIcon(QIcon(":/images/icon3.png"));
    model->setItem(2, 2, iconItem);
*/
