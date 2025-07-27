#include "User.h"

User::User(QObject *parent) : QObject(parent) {
    requester = nullptr;


}

bool User::checkLogin(ApiRequester *requester) {
    QJsonDocument json = requester->ApiRequest("", "https://music.163.com/eapi/v1/user/info", "/api/v1/user/info");
    if (json["code"] == 404) {
        return false;
    } else {
        return true;
    }
}

void User::GetUserInfo(ApiRequester *requester) {
    this->requester = requester;
    QJsonDocument json = requester->ApiRequest("", "https://music.163.com/eapi/v1/user/info", "/api/v1/user/info");
    user_id = json["userPoint"].toObject()["userId"].toInteger();

    QString path = QString("/api/v1/user/detail/%1").arg(user_id);
    json = requester->ApiRequest("", "https://music.163.com/eapi/v1/user/detail", path);
    user_nick_name = json["profile"].toObject()["nickname"].toString();
    emit UserInfoGot(user_id, user_nick_name);
}

QJsonDocument User::GetDailyRecommend(ApiRequester *requester) {
    return requester->ApiRequest("", "https://music.163.com/api/v3/discovery/recommend/songs", "/api/v3/discovery/recommend/songs");
}

qint64 User::GetFavouriteId(ApiRequester *requester, qint64 user_id, int offset, int limit) {
    QJsonObject root;
    root["uid"] = user_id;
    root["offset"] = offset;
    root["limit"] = limit;

    QJsonDocument json(root);
    QString json_text = json.toJson(QJsonDocument::Compact);
    json = requester->ApiRequest(json_text, "https://music.163.com/eapi/user/playlist", "/api/user/playlist");
    QJsonArray my_playlists = json["playlist"].toArray();

    qint64 my_favorite_id = -1;
    for(QJsonValueRef my_playlist : my_playlists) {
        qint64 creator_id = my_playlist.toObject()["creator"].toObject()["userId"].toInteger();
        QString creator_name = my_playlist.toObject()["creator"].toObject()["nickname"].toString();
        QString play_list_name = my_playlist.toObject()["name"].toString();

        if (user_id == creator_id && creator_name + QString("喜欢的音乐") == play_list_name)
        {
            my_favorite_id = my_playlist.toObject()["id"].toInteger();
            break;
        }
    }

    // // qDebug() << "[Favorite Id]" << my_favorite_id;

    return my_favorite_id;
}

QString User::getRandomDeviceId() {
    QString path = cwd.absoluteFilePath("deviceid.txt");
    QFile file(path);
    QStringList deviceIdList;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty()) {
                deviceIdList.append(line);
            }
        }
        file.close();
    }

    if (deviceIdList.isEmpty()) return "";
    int index = QRandomGenerator::global()->bounded(deviceIdList.size());
    return deviceIdList.at(index);
}

void User::registerAnonymous(ApiRequester *requester) {
    QString deviceId = getRandomDeviceId();
    // qDebug() << "deviceId = " << deviceId;

    QString encodedId = (deviceId + " " + md5Base64(xorEncrypt(deviceId, ID_XOR_KEY_1))).toUtf8().toBase64();
    QJsonObject root;
    QJsonDocument json;
    root["username"] = encodedId;
    root["e_r"] = false;
    json.setObject(root);

    // qDebug() << "registerAnonymous json = " << json;
    json = requester->ApiRequest(json.toJson(), "https://music.163.com/eapi/register/anonimous", "/api/register/anonimous");
    // qDebug() << "response = " << json;
}

QList<qint64> User::GetFavouriteIds(ApiRequester *requester, qint64 fav_id) {
    QList<qint64> favourate_ids;
    QJsonObject root;
    root["verifyId"] = 1;
    root["e_r"] = true;
    QJsonDocument json(root);
    QString json_text = json.toJson(QJsonDocument::Compact);
    json = requester->ApiRequest(json_text, "http://interface3.music.163.com/eapi/song/like/get", "/api/song/like/get");
    if (json["code"].toInt() / 100 == 3) {
        // qDebug() << "usic.163.com/eapi/song/like/get FAIL";
        QJsonObject root2;
        root2["id"] = fav_id;
        root2["newStyle"] = "true";
        root2["verifyId"] = 1;
        root2["newDetailPage"] = true;
        root2["e_r"] = true;
        root2["n"] = "300";
        root2["s"] = "5";
        QJsonDocument jsonb(root2);
        json_text = jsonb.toJson();
        QJsonValue jsonc =  requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/v6/playlist/detail", "/api/v6/playlist/detail")["playlist"];
        for (auto trackId : jsonc["trackIds"].toArray()) {
            favourate_ids.append(trackId.toObject()["id"].toInteger());
        }
    } else {
        // qDebug() << "usic.163.com/eapi/song/like/get OK";
        for(QJsonValueRef id : json["ids"].toArray()) {
            // // qDebug() << id.toInteger();
            favourate_ids.append(id.toInteger());
        }
    }
    return favourate_ids;
}

QJsonDocument User::GetArtistHomePage(ApiRequester *requester, qint64 artist_id) {
    QJsonObject root;
    root["id"] = QString::number(artist_id);
    root["e_r"] = true;
    root["verifyId"] = 1;
    QJsonDocument json(root);
    QString json_text = json.toJson();
    json = requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/artist/head/info/get", "/api/artist/head/info/get");
    return json;
}

QJsonDocument User::GetUserPlaylists(ApiRequester *requester, qint64 user_id, int offset, int limit) {
    QJsonObject root;
    root["uid"] = user_id;
    root["offset"] = offset;
    root["limit"] = limit;

    QJsonDocument json(root);
    QString json_text = json.toJson();
    json = requester->ApiRequest(json_text, "https://music.163.com/eapi/user/playlist", "/api/user/playlist");

    return json;

}

QJsonDocument User::GetFavouritePlayListJson(ApiRequester *requester, qint64 my_favorite_id) {
    if(my_favorite_id == -1)
        return QJsonDocument();

    QJsonObject root;
    root["id"] = my_favorite_id;
    root["t"] = "0";
    root["n"] = "-1";
    root["s"] = "5";
    QJsonDocument json;
    json.setObject(root);
    QString json_text = json.toJson();
    return requester->ApiRequest(json_text, "https://music.163.com/eapi/v6/playlist/detail", "/api/v6/playlist/detail");
}


bool User::SongLike(ApiRequester *requester, qint64 song_id, bool is_like) {
    QJsonObject root;
    root["trackId"] = song_id;
    root["like"] = is_like;
    QJsonDocument json(root);
    QString json_text = json.toJson(QJsonDocument::Compact);

    json = requester->ApiRequest(json_text,"https://music.163.com/eapi/song/like","/api/song/like");

    int status = json["code"].toInt();
    // qDebug() << "[SongLike]" << song_id << is_like << status;
    if(status != 200) {
        return false;
    }
    return true;
}

QJsonDocument User::GetUserDetail(ApiRequester *requester, qint64 user_id) {
    QString path = QString("/api/v1/user/detail/%1").arg(user_id);
    return requester->ApiRequest("", "https://music.163.com/eapi/v1/user/detail", path);
}

bool User::FollowArtist(ApiRequester *requester, qint64 artist_id) {
    QJsonDocument json;
    QJsonObject root;
    root["artistId"] = QString::number(artist_id);
    root["e_r"] = true;
    json.setObject(root);
    QString json_text = json.toJson(QJsonDocument::Compact);
    json = requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/v1/artist/sub", "/api/v1/artist/sub");
    if (json["code"] == 200) {
        return true;
    }
    return false;
}


bool User::DelfollowArtist(ApiRequester *requester, qint64 artist_id) {
    QJsonDocument json;
    QJsonObject root;
    root["artistIds"] = QString("[%1]").arg(artist_id);
    root["e_r"] = true;
    json.setObject(root);
    QString json_text = json.toJson(QJsonDocument::Compact);
    json = requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/artist/unsub", "/api/artist/unsub");
    if (json["code"] == 200) {
        return true;
    }
    return false;
}

bool User::FollowUser(ApiRequester *requester, qint64 user_id) {
    QJsonDocument json;
    QJsonObject root;
    root["verifyId"] = 1;
    root["e_r"] = true;
    json.setObject(root);
    json = requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://interface3.music.163.com/eapi/user/follow/" + QString::number(user_id), "/api/user/follow/" + QString::number(user_id));
    if (json["code"] == 200 or json["code"] == 201) {
        return true;
    } else {
        return false;
    }
}

bool User::DelfollowUser(ApiRequester *requester, qint64 user_id) {
    QJsonDocument json;
    QJsonObject root;
    root["verifyId"] = 1;
    root["e_r"] = true;
    json.setObject(root);
    json = requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://interface3.music.163.com/eapi/user/delfollow/" + QString::number(user_id), "/api/user/delfollow/" + QString::number(user_id));
    if (json["code"] == 200 or json["code"] == 201) {
        return true;
    } else {
        return false;
    }
}

QJsonDocument User::GetMyFollowedUser(ApiRequester *requester, int size) {
    QString json_text = QString(R"({"scene":0,"authority":true,"page":"{\"size\":\"%1\"}","e_r":true,"verifyId":1})").arg(size);
    QJsonDocument json = requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/user/follow/users/mixed/get/v2", "/api/user/follow/users/mixed/get/v2");
    if (json["code"] == 200 or json["code"] == 201) {
        return json;
    }
    return QJsonDocument();
}

QJsonArray User::GetFollowedUser(ApiRequester *requester, qint64 userId, int size) {
    QJsonDocument json;
    QJsonObject root;
    root["page"] = QString(R"({"size":"%1","cursor":""})").arg(size);
    root["userId"] = QString::number(userId);
    root["verifyId"] = 1;
    root["e_r"] = true;
    json.setObject(root);
    QString json_text = json.toJson(QJsonDocument::Compact);
    return requester->ApiRequest(json_text, "https://interface3.music.163.com/eapi/user/v3/follows/get", "/api/user/v3/follows/get")["data"]["records"].toArray();
}

QJsonDocument User::GetArtistFollowerCount(ApiRequester *requester, qint64 artist_id) {
    QJsonDocument json;
    QJsonObject root;
    root["id"] = QString::number(artist_id);
    root["verifyId"] = 1;
    root["e_r"] = true;
    json.setObject(root);
    return requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://interface3.music.163.com/eapi/artist/follow/count/get", "/api/artist/follow/count/get");
}

QJsonArray User::GetUserFollowedArtist(ApiRequester *requester, qint64 user_id, int offset, int limit) {
    QJsonDocument json;
    QJsonObject root;
    root["offset"] = QString::number(offset);
    root["limit"] = QString::number(limit);
    root["id"] = QString::number(user_id);
    root["verifyId"] = 1;
    root["e_r"] = true;
    json.setObject(root);
    return requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://interface3.music.163.com/eapi/user/sub/artist/get", "/api/user/sub/artist/get")["data"]["artists"].toArray();
}

QJsonDocument User::GetRcmdArtist(ApiRequester *requester, qint64 artist_id) {
    QJsonDocument json;
    QJsonObject root;
    root["id"] = QString::number(artist_id);
    root["verifyId"] = 1;
    root["e_r"] = true;
    json.setObject(root);
    return requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://interface3.music.163.com/eapi/v1/similar/artist/get", "/api/v1/similar/artist/get");
}

QJsonDocument User::GetMyFlavor(ApiRequester *requester) {
    QJsonDocument json;
    QJsonObject root;
    root["scene"] = 1;
    root["verifyId"] = 1;
    root["e_r"] = true;
    root["addressPermission"] = false;
    json.setObject(root);
    return requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://interface3.music.163.com/eapi/user/unfollow/recommend/v1", "/api/user/unfollow/recommend/v1");
}

bool User::CheckVIPSt(ApiRequester *requester) {
    // return true;
    QJsonDocument json;
    QJsonObject root;
    root["verifyId"] = 1;
    root["e_r"] = true;
    root["os"] = "iOS";
    json.setObject(root);
    json = requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://interface3.music.163.com/eapi/music-vip-membership/client/vip/info", "/api/music-vip-membership/client/vip/info");

    if (json["code"] != 200) {
        return false;
    } else {
        qint64 now = json["data"]["now"].toInteger();
        qint64 expireTime = json["data"]["musicPackage"]["expireTime"].toInteger();
        return (now < expireTime);
    }
}
