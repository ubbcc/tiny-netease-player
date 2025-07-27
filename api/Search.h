#ifndef SEARCH_H
#define SEARCH_H

#include "../common.h"
#include "Request.h"

#include <QObject>

class Search : public QObject
{
    Q_OBJECT
public:
    Search(QObject *parent = nullptr);

public:
    QJsonDocument SearchArtist(ApiRequester* requester, const QString& keyword, int offset, int limit);
    QJsonDocument SearchAlbum(ApiRequester* requester, const QString& keyword, int offset, int limit);
    QJsonDocument SearchUser(ApiRequester* requester, const QString& keyword, int offset, int limit);
    QJsonDocument SearchPlaylist(ApiRequester* requester, const QString& keyword, int offset, int limit);
};

#endif // SEARCH_H
