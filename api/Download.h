#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include "../common.h"
#include "Request.h"
#include "Lyric.h"

#include <QRandomGenerator64>
#include <QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QHttpHeaders>
#include <QtNetwork/QNetworkCookie>
#include <QtNetwork/QNetworkCookieJar>
#include <QCryptographicHash>

#include <QPointer>
#include <QDir>
#include <QtCore/QFile>
#include <QtWidgets/QMessageBox>
#include <QRegularExpression>

#include <QImageReader>

class Downloader : public QObject {
    Q_OBJECT

public:
    Downloader(QObject *parent = nullptr);

public slots:
    void SongDownload(qint64 song_id, QString artists_str, QString name = QString(""), QString dir = TEMP_DIR.absolutePath(), EncodeType encode_type = encodeType, Level level = quality);
    void PictureDownload(QString pic_url, QString dir = TEMP_PIC_DIR.absolutePath());

signals:
    void DownloadFinished(QString path);
    void DownloadMessageGenerated(const QString &msg, int timeout);
    void DownloadPortionUpdated(int rowNum, qint64 songId, double portion, QString type);


public slots:
    void StartDownloadFile(QString song_url, QString song_suffix);
    void SaveSongFileChunk();
    void UpdateDownloadProgress(qint64 bytes_recv, qint64 bytes_total);
    void OnDownloadFinished();

    void CreateDirectoryRecursively(const QString &dirPath);


private:
    ApiRequester *requester;
    QString save_dir, save_name;
    QString path;
    QFile song_file;
    Lyric lyric;

    QNetworkReply *reply;
    QNetworkRequest *request;
    QNetworkAccessManager *manager;
    QHttpHeaders headers;

private:
    void RequestSongUrl(qint64 id, EncodeType encode_type, Level level);
    QString RequestPlayMaxBrLevel(qint64 id);
    QString ReadMusicU(const QString &file_name);
};
#endif // DOWNLOAD_H
