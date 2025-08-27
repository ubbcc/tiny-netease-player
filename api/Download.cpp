#include "Download.h"

QString GetTime10Digits();
QString EncodeURIComponent(QString str);
QString ChooseUserAgent();

QString Downloader::ReadMusicU(const QString &file_name) {
    QString appDir = QCoreApplication::applicationDirPath();

    QDir dir(appDir);
    QDir parentDir = QFileInfo(appDir).dir();
    QString filePath = parentDir.absoluteFilePath(file_name);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // qDebug() << "无法打开文件：" << file.errorString();
        return "";
    }

    QString content = file.readAll();

    file.close();

    return content.trimmed();
}

Downloader::Downloader(QObject *parent)
    : QObject(parent)
    , requester(new ApiRequester)
    , request(new QNetworkRequest)
    , manager(new QNetworkAccessManager)
    , lyric(this){
    // QString MUSIC_U = ReadMusicU("music_u.txt");

    headers.append("User-Agent", ChooseUserAgent());
    headers.append("Content-Type", "application/x-www-form-urlencoded");
    headers.append("Cookie", QString("appver=8.9.70; buildver=%1; resulution=1920x1080; os=Android; NMTID=00Olq-ZjX3Zh6UjokPQs695eltgPzwAAAGWXOtzdw; MUSIC_U=%2; deviceId=9CC2C781CEEC4408333573ACF975B764BB3D6492A63544CD059A; channel=distribution").arg(GetTime10Digits()).arg(MUSIC_U).toUtf8());

    QObject::connect(this, &Downloader::DownloadFinished, this, [&](QString) {
        if (property("songModelRowNum") != QVariant() and property("songId") != QVariant() and property("type") != QVariant()) {
            int rowNum = property("songModelRowNum").toInt();
            qint64 songId = property("songId").toLongLong();
            QString type = property("type").toString();
            emit DownloadPortionUpdated(rowNum, songId, 1.0, type);
        }
    });
}

// void Downloader::setCookieJar() {
//     QNetworkCookieJar *cookieJar = new QNetworkCookieJar();
//     requester->setCookieJar(cookieJar);

//     // 创建 Cookie 并设置属性
//     QList<QNetworkCookie> cookies;
//     cookies.append(QNetworkCookie("appver", "8.9.70"));
//     cookies.append(QNetworkCookie("buildver", GetTime10Digits().toUtf8()));
//     cookies.append(QNetworkCookie("resulution", "1920x1080"));
//     cookies.append(QNetworkCookie("os", "Android"));
//     cookies.append(QNetworkCookie("NMTID", "00Olkf8yA_zXELz6kL0nny1CWLqpX0AAAGV2oQVxQ"));
//     cookies.append(QNetworkCookie("MUSIC_U", MUSIC_U.toUtf8()));
//     QStringList urls = {
//         "https://music.163.com/eapi/v3/song/detail",
//         "https://music.163.com/eapi/song/enhance/player/url/v1",
//     };
//     for (const QString &url : urls) {
//         cookieJar->setCookiesFromUrl(cookies, QUrl(url));
//     }
//     // qDebug() << cookieJar << MUSIC_U;
// }

void Downloader::StartDownloadFile(QString song_url, QString file_name) {
    CreateDirectoryRecursively(save_dir);
    path = QString("%1/%2").arg(save_dir).arg(file_name);
    song_file.setFileName(path);
    // // qDebug() << "StartDownloadFile "  << file_name << "url " << song_url;
    if (song_file.exists()) {
        emit DownloadFinished(path);
        // emit DownloadMessageGenerated(QString("Already downloaded"), 0);
        return;
    }

    if (!song_file.open(QIODevice::WriteOnly)) {
        qWarning() << "song_file.open(QIODevice::WriteOnly Error!";
        return;
    }

    QUrl url_(song_url);
    request->setUrl(url_);
    request->setHeaders(headers);

    reply = manager->get(*request);
    QObject::connect(reply, &QNetworkReply::readyRead, this, &Downloader::SaveSongFileChunk);
    QObject::connect(reply, &QNetworkReply::downloadProgress, this, &Downloader::UpdateDownloadProgress);
    QObject::connect(reply, &QNetworkReply::finished, this, &Downloader::OnDownloadFinished);
    QObject::connect(reply, &QNetworkReply::finished, this, &Downloader::deleteLater);
}


void Downloader::SongDownload(qint64 song_id, QString artists_str, QString name, QString dir, EncodeType encode_type, Level level) {
    save_dir = dir;
    CreateDirectoryRecursively(save_dir);
    if (name == QString("")) {
        save_name = QString::number(song_id);
    } else {
        save_name = name.replace(sanitizeFileNameregex,"");
        if (artists_str != QString("")) {
            save_name = artists_str + " - " + save_name;
        }
    }
    RequestSongUrl(song_id, encode_type, level);
    QJsonDocument lrc_json = lyric.GetLyric(requester, song_id);
    QString lrc = lrc_json["lrc"]["lyric"].toString();
    QString tlrc = lrc_json["tlyric"]["lyric"].toString();
    QString new_lrc = lyric.MergeLyric(lrc, tlrc);
    QFile newLrcFile(dir + "/" + save_name + ".lrc");
    if (newLrcFile.exists()) {
        // emit DownloadMessageGenerated("already downloaded", 0);
        return;
    }
    newLrcFile.open(QFile::WriteOnly);
    newLrcFile.write(new_lrc.toUtf8());
    newLrcFile.close();
}

void Downloader::PictureDownload(QString pic_url, QString dir) {
    save_dir = dir;

    QString file_name;
    QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
    // 匹配输入字符串
    QRegularExpressionMatch match = regex.match(pic_url);
    if (match.hasMatch()) {
        file_name = match.captured(1); // 获取匹配的文件名
    } else {
        file_name = "temp.jpg";
    }

    StartDownloadFile(pic_url, file_name);
}


void Downloader::UpdateDownloadProgress(qint64 bytes_recv, qint64 bytes_total) {
    if (bytes_total > 0) {
        // QString download_progress_msg = QString("%1%, %2 MB").arg(bytes_recv * 100 / bytes_total).arg((double)bytes_total / (1024 * 1024), 0, 'g', 3, QChar('u'));
        // // qDebug() << "download_progress_msg" << download_progress_msg;
        // emit Downloader::DownloadMessageGenerated(download_progress_msg, 0);
        if (property("songModelRowNum") != QVariant() and property("songId") != QVariant() and property("type") != QVariant()) {
            int rowNum = property("songModelRowNum").toInt();
            qint64 songId = property("songId").toLongLong();
            QString type = property("type").toString();
            double portion = 1.0 * bytes_recv / bytes_total;
            emit DownloadPortionUpdated(rowNum, songId, portion, type);
        }
    }
}

void Downloader::SaveSongFileChunk() {
    if (song_file.isOpen()) {
        song_file.write(reply->readAll()); // Write the chunk of data to file
    }
}

void Downloader::OnDownloadFinished() {
    if (reply->error() == QNetworkReply::NoError) {
        // qDebug() << "Download completed! " << save_name;
    } else {
        // qDebug() << "Download error! " << reply->errorString();
    }
    song_file.close();
    reply->deleteLater();
    emit DownloadFinished(path);
}

QString Downloader::RequestPlayMaxBrLevel(qint64 id) {
    QString json_text = QString(R"del({"c":"[{\"id\":%1}]"})del").arg(id);
    QJsonDocument json = requester->ApiRequest(json_text, "https://music.163.com/eapi/v3/song/detail", "/api/v3/song/detail");
    return json["privileges"][0]["playMaxBrLevel"].toString();
}

void Downloader::RequestSongUrl(qint64 id, EncodeType encode_type, Level level) {
    QJsonObject root;
    root["ids"] = QString(R"del(["%1"])del").arg(id);

    const QString kEncodeTypeTable[] = { "mp3", "aac" };
    root["encodeType"] = kEncodeTypeTable[encode_type];

    const QString kLevelTable[] = { "lossless", "higher", "standard", "hires", "jyeffect", "sky", "jymaster", "best" };

    if(kLevelTable[level] == "best") {
        root["level"] = RequestPlayMaxBrLevel(id);
    } else {
        root["level"] = kLevelTable[level];
    }

    // save_name = QString("【%1】 ").arg(root["level"].toString()) + save_name;
    auto qualityStr = [](const QJsonValue &v) -> QString {
        if (v == "lossless") return "无损";
        if (v == "higher") return "极高";
        if (v == "standard") return "标准";
        if (v == "hires") return "高解析度无损";
        if (v == "jyeffect") return "高清环绕声";
        if (v == "sky") return "沉浸环绕声";
        if (v == "jymaster") return "超清母带";
        if (v == "dolby") return "杜比";
        // qDebug() << "unknown qstr:" << v;
        return v.toString();
    };
    save_name = QString("【%1】").arg(qualityStr(root["level"])) + save_name;

    QJsonDocument json(root);
    QString json_text = json.toJson(QJsonDocument::Compact);
    json = requester->ApiRequest(json_text, "https://music.163.com/eapi/song/enhance/player/url/v1", "/api/song/enhance/player/url/v1");

    QJsonObject json_data = json["data"].toArray()[0].toObject();
    QString song_url = json_data["url"].toString();
    if (song_url == "") {
        QMessageBox::warning(nullptr, "", "这首歌需要订阅数字专辑才能播放、下载哦~");
        return;
    }
    QString song_suffix = json_data["type"].toString();
    QString file_name = save_name + "." + song_suffix;

    StartDownloadFile(song_url, file_name);
}

QString GetTime10Digits() { // 获取当前时间的 Unix 时间戳（秒级）
    qint64 unixTimestamp = QDateTime::currentSecsSinceEpoch();

    // 将时间戳转换为字符串（十进制表示）
    QString timestampStr = QString::number(unixTimestamp);

    // 截取前 10 位字符
    QString truncatedTimestamp = timestampStr.left(10);

    return truncatedTimestamp;
}

QString EncodeURIComponent(QString str) {
    str = QUrl(str).toEncoded();
    str.replace("+", "%20");
    return str;
}


QString ChooseUserAgent() {
    static const QString userAgentList[] = {
        "Mozilla/5.0 (iPhone; CPU iPhone OS 9_1 like Mac OS X) AppleWebKit/601.1.46 (KHTML, like Gecko) Version/9.0 Mobile/13B143 Safari/601.1",
        "Mozilla/5.0 (iPhone; CPU iPhone OS 9_1 like Mac OS X) AppleWebKit/601.1.46 (KHTML, like Gecko) Version/9.0 Mobile/13B143 Safari/601.1",
        "Mozilla/5.0 (Linux; Android 5.0; SM-G900P Build/LRX21T) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.115 Mobile Safari/537.36",
        "Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.115 Mobile Safari/537.36",
        "Mozilla/5.0 (Linux; Android 5.1.1; Nexus 6 Build/LYZ28E) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.115 Mobile Safari/537.36",
        "Mozilla/5.0 (iPhone; CPU iPhone OS 10_3_2 like Mac OS X) AppleWebKit/603.2.4 (KHTML, like Gecko) Mobile/14F89;GameHelper",
        "Mozilla/5.0 (iPhone; CPU iPhone OS 10_0 like Mac OS X) AppleWebKit/602.1.38 (KHTML, like Gecko) Version/10.0 Mobile/14A300 Safari/602.1",
        "NeteaseMusic/6.5.0.1575377963(164);Dalvik/2.1.0 (Linux; U; Android 9; MIX 2 MIUI/V12.0.1.0.PDECNXM)",
    };
    int idx = QRandomGenerator::global()->bounded(7, 8);
    return userAgentList[idx];
}

void Downloader::CreateDirectoryRecursively(const QString &dirPath) {
    QDir dir;
    if (!dir.exists(dirPath)) {
        dir.mkpath(dirPath);  // 递归创建目录
    }
}
