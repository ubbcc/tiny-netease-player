#ifndef SONGSEARCH_H
#define SONGSEARCH_H

#include "../common.h"
#include "Request.h"
// #include "GetUserInfo.h"

#include <QtCore/QVector>
#include <QJsonDocument>

QJsonDocument SongSearch(ApiRequester* requester, const QString& keyword, int offset = 0, int limit = 20);

#endif // SONGSEARCH_H
