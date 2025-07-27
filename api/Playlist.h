#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "../common.h"
#include "Request.h"

class Playlist : public QObject {
    Q_OBJECT

public:
    Playlist(QObject *parent = nullptr);

public:
    QJsonDocument GetAlbumPlaylist(ApiRequester *requester, qint64 id);
    QJsonDocument GetArtistAlbums(ApiRequester *requester, qint64 id, int offset = 0, int limit = 20);
    QJsonDocument GetPlaylistDetail(ApiRequester *requester, qint64 id);
    QJsonDocument GetArtistHot(ApiRequester *requester, qint64 artistId, int num = 50);
    QJsonArray GetSongDetail(ApiRequester *requester, QJsonArray trackIds);
    QJsonArray GetSongDetail(ApiRequester *requester, QList<QVariant> trackIds);
    QJsonArray GetRcmdPlaylist(ApiRequester *requester, qint64 playlistId);

    bool SubscribePlaylist(ApiRequester *requester, qint64 playlistId);
    bool UnsubscribePlaylist(ApiRequester *requester, qint64 playlistId);

    bool SubscribeAlbum(ApiRequester *requester, qint64 album_id);
    bool UnsubscribeAlbum(ApiRequester *requester, qint64 album_id);

    QJsonDocument GetAlbumSub(ApiRequester *requester, qint64 album_id);

    QJsonDocument AddTrackToPlaylist(ApiRequester *requester, qint64 playlistId, qint64 trackId);
    QJsonDocument DeleteTrackFromPlaylist(ApiRequester *requester, qint64 playlistId, qint64 trackId);
    QJsonDocument RequestTrackStatus(ApiRequester *requester, qint64 userId, qint64 trackId, int offset = 0);

    QJsonDocument CreatePlaylist(ApiRequester *requester, const QString &name, int privacy = 0); // privacy = 0 公开 privacy = 10 隐私
    QJsonDocument DeletePlaylist(ApiRequester *requester, qint64 playlistId);
};

#endif // PLAYLIST_H
