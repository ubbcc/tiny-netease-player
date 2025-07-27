#include "songtableheaderview.h"

SongTableHeaderView::SongTableHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
    , button_size(QSize(20, 20)) {
    download_icon = QIcon(":/download_icon.png");
    favourite_icon = QIcon(":/favourite_icon.png");
}

void SongTableHeaderView::paintSection(QPainter *painter, const QRect &rect, int logical_index) const {
    QRect download_button_rect = QRect(rect.left(), rect.top(), button_size.width(), button_size.height());
    download_icon.paint(painter, download_button_rect);

    QRect favourite_button_rect = QRect(rect.right(), rect.top(), button_size.width(), button_size.height());
    favourite_icon.paint(painter, favourite_button_rect);
}

void SongTableHeaderView::mousePressEvent(QMouseEvent *event) {
    int logical_index = logicalIndexAt(event->pos());
    if (logical_index == -1) {
        return;
    }

    // QRect section_rect;
    // int section_y_position = sectionPosition(logical_index);
    // int section_height = sectionSize(logical_index);
    // section_rect.setTop(section_y_position);
    // section_rect.setLeft()
    QHeaderView::mousePressEvent(event);

}
