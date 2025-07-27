#include "Playlist.h"

Playlist::Playlist(QObject *parent) : QObject(parent) {

}


QJsonDocument Playlist::GetAlbumPlaylist(ApiRequester *requester, qint64 id) {
    QJsonObject root;
    root["id"] = id;
    QString cache_key = CacheKeyEncrypt(QString("id=%1").arg(id).toUtf8()).toBase64();
    root["cache_key"] = cache_key;
    QJsonDocument json(root);

    QString json_text = json.toJson();
    return requester->ApiRequest(json_text, "https://music.163.com/eapi/album/v3/detail", "/api/album/v3/detail");
}

QJsonDocument Playlist::GetArtistAlbums(ApiRequester *requester, qint64 id, int offset, int limit) {
    QJsonObject root;
    root["offset"] = offset;
    root["limit"] = QString::number(limit);
    root["verifyId"] = 1;
    root["e_r"] = true;
    QJsonDocument json(root);

    QString json_text = json.toJson();
    return requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/artist/albums/" + QString::number(id), "/api/artist/albums/" + QString::number(id));
}

QJsonDocument Playlist::GetPlaylistDetail(ApiRequester *requester, qint64 id) {
    QJsonObject root;
    root["id"] = id;
    root["newStyle"] = "true";
    root["verifyId"] = 1;
    root["newDetailPage"] = true;
    root["e_r"] = true;
    root["n"] = "300";
    root["s"] = "5";
    QJsonDocument json(root);

    QString json_text = json.toJson();
    return requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/v6/playlist/detail", "/api/v6/playlist/detail");
}

QJsonDocument Playlist::GetArtistHot(ApiRequester *requester, qint64 artistId, int num) {
    QJsonObject root;
    root["id"] = QString::number(artistId);
    root["order"] = "hot";
    root["top"] = num;
    root["e_r"] = true;
    root["verifyId"] = 1;
    root["work_type"] = "5";

    QJsonDocument json(root);

    QString json_text = json.toJson();

    return requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/v1/artist/top/song", "/api/v1/artist/top/song");
}

bool Playlist::SubscribePlaylist(ApiRequester *requester, qint64 playlistId) {
    QJsonObject root;
    root["id"] = playlistId;
    root["e_r"] = true;
    root["verifyId"] = 1;
    QJsonDocument json(root);
    QString json_text = json.toJson();
    json = requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/playlist/subscribe", "/api/playlist/subscribe");
    if (json["code"] == 200) {
        return true;
    }
    return false;
}

bool Playlist::UnsubscribePlaylist(ApiRequester *requester, qint64 playlistId) {
    QJsonObject root;
    root["id"] = playlistId;
    root["e_r"] = true;
    root["verifyId"] = 1;
    QJsonDocument json(root);
    QString json_text = json.toJson();
    json = requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/playlist/unsubscribe", "/api/playlist/unsubscribe");
    if (json["code"] == 200) {
        return true;
    }
    return false;
}

QJsonArray Playlist::GetRcmdPlaylist(ApiRequester *requester, qint64 playlistId) {
    QJsonObject root;
    root["playlistId"] = playlistId;
    root["e_r"] = true;
    root["verifyId"] = 1;
    root["newStyle"] = true;
    root["scene"] = "playlist_tail";
    QJsonDocument json(root);
    QString json_text = json.toJson();
    json = requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/playlist/detail/rcmd/get", "/api/playlist/detail/rcmd/get");
    return json["data"]["recPlaylist"].toArray();
}

QJsonArray Playlist::GetSongDetail(ApiRequester *requester, QJsonArray trackIds) {
    QJsonArray result;
    QJsonObject root;
    root["trialMode"] = 12;
    root["e_r"] = true;
    root["verifyId"] = 1;
    root["source"] = "";
    QString req_c = "[";
    for (auto trackId : trackIds) {
        req_c += QString(R"del({"id":%1},)del").arg(trackId.toObject()["id"].toInteger());
    }
    req_c.chop(1);
    req_c += "]";
    root["c"] = req_c;
    QJsonDocument json(root);
    QString json_text = json.toJson();
    json = requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/v3/song/detail", "/api/v3/song/detail");
    return json["songs"].toArray();
}

QJsonArray Playlist::GetSongDetail(ApiRequester *requester, QList<QVariant> trackIds) {
    QJsonArray result;
    QJsonObject root;
    root["trialMode"] = 12;
    root["e_r"] = true;
    root["verifyId"] = 1;
    root["source"] = "";
    QString req_c = "[";
    for (auto trackId : trackIds) {
        req_c += QString(R"del({"id":%1},)del").arg(trackId.toLongLong());
    }
    req_c.chop(1);
    req_c += "]";
    root["c"] = req_c;
    QJsonDocument json(root);
    QString json_text = json.toJson();
    return requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/v3/song/detail", "/api/v3/song/detail")["songs"].toArray();
}

bool Playlist::SubscribeAlbum(ApiRequester *requester, qint64 album_id) {
    QJsonObject root;
    root["id"] = QString::number(album_id);
    root["e_r"] = true;
    root["verifyId"] = 1;
    QJsonDocument json(root);
    QString json_text = json.toJson();
    json = requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/album/sub", "/api/album/sub");
    if (json["code"] == 200) {
        return true;
    }
    return false;
}
bool Playlist::UnsubscribeAlbum(ApiRequester *requester, qint64 album_id) {
    QJsonObject root;
    root["id"] = QString::number(album_id);
    root["e_r"] = true;
    root["verifyId"] = 1;
    QJsonDocument json(root);
    QString json_text = json.toJson();
    json = requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/album/unsub", "/api/album/unsub");
    if (json["code"] == 200) {
        return true;
    }
    return false;
}

QJsonDocument Playlist::GetAlbumSub(ApiRequester *requester, qint64 album_id) {
    QJsonObject root;
    root["id"] = QString::number(album_id);
    root["e_r"] = true;
    root["verifyId"] = 1;
    QJsonDocument json(root);
    QString json_text = json.toJson();
    json = requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/album/detail/dynamic", "/api/album/detail/dynamic");
    return json;
}

QJsonDocument Playlist::CreatePlaylist(ApiRequester *requester, const QString &name, int privacy) {
    QJsonDocument json;
    QJsonObject root;
    root["os"] = "iOS";
    root["verifyId"] = 1;
    root["privacy"] = privacy;
    root["type"] = "NORMAL";
    root["name"] = name;
    root["e_r"] = true;
    json.setObject(root);
    return requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://interface3.music.163.com/eapi/playlist/create", "/api/playlist/create");
}

QJsonDocument Playlist::DeletePlaylist(ApiRequester *requester, qint64 playlistId) {
    QJsonDocument json;
    QJsonObject root;
    root["os"] = "iOS";
    root["verifyId"] = 1;
    root["pid"] = playlistId;
    root["e_r"] = true;
    json.setObject(root);
    return requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://interface3.music.163.com/eapi/playlist/delete", "/api/playlist/delete");
}

QJsonDocument Playlist::RequestTrackStatus(ApiRequester *requester, qint64 userId, qint64 trackId, int offset) {
    QJsonDocument json;
    QJsonObject root;
    root["includeShareStatus"] = true;
    root["includeVideo"] = false;
    root["os"] = "iOS";
    root["uid"] = userId;
    root["offset"] = QString::number(offset);
    root["verifyId"] = 1;
    root["trackIds"] = QString::number(trackId);
    root["e_r"] = true;
    json.setObject(root);
    return requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://interface3.music.163.com/eapi/user/playlist/v1s", "/api/user/playlist/v1");
}


QJsonDocument Playlist::AddTrackToPlaylist(ApiRequester *requester, qint64 playlistId, qint64 trackId) {
    QJsonDocument json;
    QJsonObject root;
    root["os"] = "iOS";
    root["verifyId"] = 1;
    root["pid"] = QString::number(playlistId);
    root["trackIds"] = QString(R"(["%1"])").arg(trackId);
    root["op"] = "add";
    root["e_r"] = true;
    json.setObject(root);
    return requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://interface3.music.163.com/eapi/v1/playlist/manipulate/tracks", "/api/v1/playlist/manipulate/tracks");
}

QJsonDocument Playlist::DeleteTrackFromPlaylist(ApiRequester *requester, qint64 playlistId, qint64 trackId) {
    QJsonDocument json;
    QJsonObject root;
    root["os"] = "iOS";
    root["verifyId"] = 1;
    root["pid"] = playlistId;
    root["trackIds"] = QString(R"(["%1"])").arg(trackId);
    root["op"] = "del";
    root["e_r"] = true;
    json.setObject(root);
    return requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://interface3.music.163.com/eapi/v1/playlist/manipulate/tracks", "/api/v1/playlist/manipulate/tracks");
}

