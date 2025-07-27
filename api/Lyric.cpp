#include "Lyric.h"

Lyric::Lyric(QObject *parent)
    : QObject{parent}
{}

void Lyric::LyricDownload(ApiRequester *requester, qint64 song_id, QString song_name, QString save_dir) {
    if (song_name == "") {
        song_name = QString::number(song_id);
    }

    QJsonDocument json = GetLyric(requester, song_id);
    // qDebug() << json;

    CreateDirectoryRecursively(save_dir);

    QFile lrc_file(save_dir + "/" + song_name + "_main.lrc");
    lrc_file.open(QFile::WriteOnly);
    lrc_file.write(json["lrc"]["lyric"].toString().toUtf8());
    lrc_file.close();

    if (json["tlyric"]["lyric"].toString() != "") {
        QFile tlrc_file(save_dir + "/" + song_name + "_trans.lrc");
        tlrc_file.open(QFile::WriteOnly);
        tlrc_file.write(json["tlyric"]["lyric"].toString().toUtf8());
        tlrc_file.close();
    }

    if (json["yrc"]["lyric"].toString() != "") {
        QFile yrc_file(save_dir + "/" + song_name + "_y.lrc");
        yrc_file.open(QFile::WriteOnly);
        yrc_file.write(json["yrc"]["lyric"].toString().toUtf8());
        yrc_file.close();
    }
}

QJsonDocument Lyric::GetLyric(ApiRequester *requester, qint64 song_id) {
    QJsonDocument json;
    QJsonObject root;
    root["id"] = song_id;
    root["lv"] = -1, root["kv"] =-1, root["tv"] = -1, root["yv"] = -1;
    json.setObject(root);

    return requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://music.163.com/eapi/song/lyric", "/api/song/lyric");
}

QString Lyric::MergeLyric(QString lrc1, QString lrc2) {
    static QRegularExpression re(R"(\[(\d{2}:\d{2}\.\d{1,3})\](.*))");

    QList <Line> slice1, slice2;
    for (const QRegularExpressionMatch& match : re.globalMatch(lrc1)) {
        slice1.append(Line{match.captured(1), match.captured(2)});
    }
    for (const QRegularExpressionMatch& match : re.globalMatch(lrc2)) {
        slice2.append(Line{match.captured(1), match.captured(2)});
    }

    int i = 0, j = 0;
    QString newLrc;
    while (i < slice1.length() && j < slice2.length()) {
        if (slice1[i].time < slice2[j].time) {
            if (slice1[i].words != "") {
                newLrc += "[" + slice1[i].time + "]" + slice1[i].words + "\n";
            }
            i++;
        } else if (slice1[i].time == slice2[j].time) {
            if (slice2[j].words != "") {
                newLrc += "[" + slice1[i].time + "]" + slice1[i].words + "\n" + "[" + slice1[i].time + "]" + slice2[j].words + "\n";
            } else {
                if (slice1[i].words != "") {
                    newLrc += "[" + slice1[i].time + "]" + slice1[i].words + "\n";
                }
            }
            i++; j++;
        } else {
            if (slice2[j].words != "") {
                newLrc += "[" + slice2[j].time + "]" + slice2[j].words + "\n";
            }
            j++;
        }
    }
    for (; i < slice1.length(); i++) {
        if (slice1[i].words != "") {
            newLrc += "[" + slice1[i].time + "]" + slice1[i].words + "\n";
        }
    }
    return newLrc;
}


void Lyric::CreateDirectoryRecursively(const QString &dirPath) {
    QDir dir;
    if (!dir.exists(dirPath)) {
        dir.mkpath(dirPath);  // 递归创建目录
    }
}
