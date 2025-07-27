#include "Search.h"

Search::Search(QObject *parent)
    : QObject{parent}
{}

QJsonDocument Search::SearchArtist(ApiRequester* requester, const QString& keyword, int offset, int limit) {
    QJsonObject root;
    root["s"] = keyword;
    root["limit"] = limit;
    root["offset"] = offset;
    root["e_r"] = true;
    root["queryCorrect"] = true;
    root["q_scene"] = "defaultquery";

    QJsonDocument json(root);
    // qDebug() << "[Json]" << json;
    QString txt = json.toJson(QJsonDocument::Compact);
    return requester->ApiRequest(txt, "https://interface3.music.163.com/eapi/v1/search/artist/get", "/api/v1/search/artist/get");
}

QJsonDocument Search::SearchAlbum(ApiRequester* requester, const QString& keyword, int offset, int limit) {
    QJsonObject root;
    root["s"] = keyword;
    root["limit"] = limit;
    root["offset"] = offset;
    root["e_r"] = true;
    root["queryCorrect"] = true;
    root["q_scene"] = "defaultquery";

    QJsonDocument json(root);
    // qDebug() << "[Json]" << json;
    QString txt = json.toJson(QJsonDocument::Compact);
    return requester->ApiRequest(txt, "https://interface3.music.163.com/eapi/v1/search/album/get", "/api/v1/search/album/get");
}

QJsonDocument Search::SearchUser(ApiRequester* requester, const QString& keyword, int offset, int limit) {
    QJsonObject root;
    root["s"] = keyword;
    root["limit"] = limit;
    root["offset"] = offset;
    root["e_r"] = true;
    root["queryCorrect"] = true;
    root["q_scene"] = "defaultquery";

    QJsonDocument json(root);
    // qDebug() << "[Json]" << json;
    QString txt = json.toJson(QJsonDocument::Compact);
    return requester->ApiRequest(txt, "https://interface3.music.163.com/eapi/v1/search/user/get", "/api/v1/search/user/get");
}

QJsonDocument Search::SearchPlaylist(ApiRequester* requester, const QString& keyword, int offset, int limit) {
    QJsonObject root;
    root["s"] = keyword;
    root["limit"] = limit;
    root["offset"] = offset;
    root["e_r"] = true;
    root["queryCorrect"] = true;
    root["q_scene"] = "defaultquery";

    QJsonDocument json(root);
    // qDebug() << "[Json]" << json;
    QString txt = json.toJson(QJsonDocument::Compact);
    return requester->ApiRequest(txt, "https://interface3.music.163.com/eapi/v1/search/playlist/get", "/api/v1/search/playlist/get");
}
