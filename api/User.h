#ifndef USER_H
#define USER_H

#include "../common.h"

#include "Request.h"

const QString ID_XOR_KEY_1 = "3go8&$8*3*3h0k(2)2";

class User : public QObject {
    Q_OBJECT

public:
    User(QObject *parent = nullptr);

public:
    bool checkLogin(ApiRequester *requester);

    void GetUserInfo(ApiRequester *requester);
    QList<qint64> GetFavouriteIds(ApiRequester *requester, qint64 fav_id);
    QJsonDocument GetFavouritePlayListJson(ApiRequester *requester, qint64 my_favorite_id);
    QJsonDocument GetDailyRecommend(ApiRequester *requester);

    bool FollowArtist(ApiRequester *requester, qint64 artist_id);
    bool DelfollowArtist(ApiRequester *requester, qint64 artist_id);
    bool FollowUser(ApiRequester *requester, qint64 user_id);
    bool DelfollowUser(ApiRequester *requester, qint64 user_id);
    bool SongLike(ApiRequester *requester, qint64 song_id, bool is_like);

    QJsonDocument GetMyFollowedUser(ApiRequester *requester, int size = 1000);
    QJsonDocument GetMyFollowedArtist(ApiRequester *requester, int size = 1000);

    QJsonDocument GetArtistHomePage(ApiRequester *requester, qint64 artist_id);
    QJsonDocument GetUserDetail(ApiRequester *requester, qint64 user_id);
    qint64 GetFavouriteId(ApiRequester *requester, qint64 user_id, int offset = 0, int limit = PAGE_SIZE);
    QJsonArray GetFollowedUser(ApiRequester *requester, qint64 userId, int size = 1000);
    QJsonDocument GetUserPlaylists(ApiRequester *requester, qint64 user_id, int offset = 0, int limit = PAGE_SIZE);
    QJsonArray GetUserFollowedArtist(ApiRequester *requester, qint64 user_id, int offset = 0, int limit = 1000);

    QJsonDocument GetArtistFollowerCount(ApiRequester *requester, qint64 artist_id);
    QJsonDocument GetRcmdArtist(ApiRequester *requester, qint64 artist_id);
    QJsonDocument GetMyFlavor(ApiRequester *requester);
    bool CheckVIPSt(ApiRequester *requester);

    QString getRandomDeviceId();
    void registerAnonymous(ApiRequester *requester);

signals:
    void UserInfoGot(qint64 user_id, QString user_nick_name);

public:
    qint64 user_id;
    QString user_nick_name;
    ApiRequester *requester;

};

#endif // USER_H
