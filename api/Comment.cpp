#include "Comment.h"

Comment::Comment(QObject *parent) : QObject(parent) {

}

// QJsonDocument Comment::GetComment(ApiRequester *requester, qint64 songId, QString cursor, int limit) {
//     QJsonDocument json;
//     QJsonObject root;
//     // root["pageNo"] = QString::number(offset);
//     root["pageNo"] = "2";
//     root["pageSize"] = QString::number(limit);
//     root["scene"] = "SONG_COMMENT";
//     root["preloadExpGroupName"] = "t1";
//     root["sortType"] = "2";
//     root["showInner"] = "0";
//     root["cursor"] = cursor;
//     root["threadId"] = QString("R_SO_4_%1").arg(songId);
//     root["verifyId"] = 1;
//     root["e_r"] = true;
//     json.setObject(root);
//     // qDebug() << "GetComment" << json;
//     // QString jsontxt = QString(R"({"cursor":"%1","pageNo":"%2","scene":"SONG_COMMENT","pageSize":"%3","preloadExpGroupName":"t1","sortType":"1","showInner":"0","threadId":"R_SO_4_%4","verifyId":1,"e_r":true})").arg((offset - 1) * limit).arg(offset).arg(limit).arg(songId);
//     json = requester->ApiRequest(json.toJson(), "https://interface3.music.163.com/eapi/v2/resource/comments", "/api/v2/resource/comments");
//     return json;
// }

QJsonDocument Comment::GetCommentFloor(ApiRequester *requester, qint64 songId, qint64 parentCommentId, qint64 time, QString cursor, int limit) {
    QJsonDocument json;
    QJsonObject root;
    root["source"] = "";
    root["threadId"] = QString("R_SO_4_%1").arg(songId);
    root["parentCommentId"] = QString::number(parentCommentId);
    root["limit"] = limit;
    root["order"] = 0;
    root["scene"] = "SONG_COMMENT";
    root["time"] = time;
    root["verifyId"] = 1;
    root["os"] = "iOS";
    root["order"] = 0;
    root["cursor"] = cursor;
    // root["commentId"] = "";
    root["e_r"] = true;
    json.setObject(root);
    // qDebug() << "GetCommentFloor" << json;
    return requester->ApiRequest(json.toJson(), "https://interface3.music.163.com/eapi/v2/resource/comment/floor/get", "/api/v2/resource/comment/floor/get");
}
