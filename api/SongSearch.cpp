#include "SongSearch.h"

QJsonDocument SongSearch(ApiRequester* requester, const QString& keyword, int offset, int limit) {
    // QJsonObject root;
    // root["s"] = keyword;
    // root["limit"] = limit;
    // root["offset"] = offset;

    // QJsonDocument json(root);
    // // qDebug() << "[Json]" << json;
    QString txt;
    txt = QString(R"del({"s": "%1","limit": "%2","offset": "%3"})del").arg(keyword).arg(limit).arg(offset);
    return requester->ApiRequest(txt, "https://music.163.com/eapi/v1/search/song/get", "/api/v1/search/song/get");
}

