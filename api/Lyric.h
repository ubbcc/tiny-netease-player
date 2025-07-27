#ifndef LYRIC_H
#define LYRIC_H

#include "../common.h"
#include "Request.h"

#include <QObject>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>

class Lyric : public QObject
{
    Q_OBJECT
public:
    Lyric(QObject *parent = nullptr);

    void LyricDownload(ApiRequester *requester, qint64 song_id, QString song_name = "", QString save_dir = "/Users/guest1/Desktop/TestDownload/LyricCache");
    QJsonDocument GetLyric(ApiRequester *requester, qint64 song_id);
    QString MergeLyric(QString lrc1, QString lrc2);

    void CreateDirectoryRecursively(const QString &dirPath);

    struct Line {
        QString time, words;
    };
};

#endif // LYRIC_H
