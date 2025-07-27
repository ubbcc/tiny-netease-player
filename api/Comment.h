#ifndef COMMENT_H
#define COMMENT_H

#include "../common.h"
#include "Request.h"

class Comment : public QObject {
    Q_OBJECT

public:
    Comment(QObject *parent = nullptr);

    // QJsonDocument GetComment(ApiRequester *requester, qint64 songId, QString cursor, int limit = 20);
    QJsonDocument GetCommentFloor(ApiRequester *requester, qint64 songId, qint64 parentCommentId, qint64 time = -1, QString cursor = "", int limit = 20);
};

#endif // COMMENT_H
