#include "Request.h"
#include <QMessageBox>

ApiRequester::ApiRequester(QObject* parent) : QObject(parent) {
    manager = new QNetworkAccessManager;
    request = new QNetworkRequest;

    if (parent != nullptr) {
        if (parent->metaObject()->className() == "MainWindow") {
            QObject::connect(this, SIGNAL("didntLogin"), parent, SLOT("Relogin"));
        }
    }
}

QString ApiRequester::generateRequestId() {
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    int randomNumber = QRandomGenerator::global()->bounded(1000); // 0 ~ 999
    QString randomStr = QString("%1").arg(randomNumber, 4, 10, QLatin1Char('0')); // 补齐4位
    return QString("%1_%2").arg(timestamp).arg(randomStr);
}

QJsonDocument ApiRequester::ApiRequest(const QString &json_text, const QString &url, const QString &path, bool log) {
    // qDebug() << is_busy;
    if (is_busy) {
        // QEventLoop loop;
        // QObject::connect(this, &ApiRequester::ApiRequestFinished, &loop, &QEventLoop::quit, Qt::QueuedConnection);
        // loop.exec();
        emit engaged();
        return QJsonDocument();
    }

    is_busy = true;
    QByteArray data = this->Splice(path, json_text);
    QJsonDocument resp_json;
    int st_code = 0;
    QByteArray resp_data = SendRequest(this->ToParams(data), url, path, log);
    qDebug() << url;
    if (url.contains("//interface3")) {
        // qDebug() << "/*DECODE*/";
        // QString input = "1234";
        // resp_data = QByteArray::fromHex("1234");
        resp_data = EapiDecrypt(resp_data);
    }
    // qDebug() << "[ApiReply data] " << resp_data.sliced(0,qMin(100, resp_data.size()));
    resp_json = QJsonDocument::fromJson(resp_data);
    st_code = resp_json["code"].toInt() / 100;
    if (st_code == 3 or st_code == 4) {
        QString message = resp_json["message"].toString();
        qDebug() << "[ApiRequestErr]" <<  st_code << message;
    }
    if (st_code == 3 and url != "http://interface3.music.163.com/eapi/song/like/get") {
        if (!isGuest) {
            emit didntLogin();
        }
    } else if (st_code == 4) {
    }
    // qDebug() << "[ApiReply json]" << resp_json;
    is_busy = false;
    // emit ApiRequestFinished();
    return resp_json;
}

// QJsonDocument ApiRequester::WeapiRequest(const QString &json_text, const QString &url, const QString &path) {
//  }


QByteArray ApiRequester::SendRequest(QByteArray data, QString url, QString path, bool vip) {
    // QObject::connect(manager, &QNetworkAccessManager::finished, this, callback);

    QUrl url_(url);
    request->setUrl(url_);

    request->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request->setRawHeader("User-Agent", ChooseUserAgent().toUtf8());
    request->setRawHeader("Host", "music.163.com");
    request->setRawHeader("Connection", "keep-alive");
    request->setRawHeader("Accept", "*/*");
    request->setRawHeader("Accept-Language", "zh-CN,zh;q=0.9");

    // 设置请求头
    if (vip) {
        request->setRawHeader("Cookie", QString("appver=8.9.70; buildver=%1; resulution=1920x1080; os=Android; NMTID=00Olq-ZjX3Zh6UjokPQs695eltgPzwAAAGWXOtzdw; MUSIC_U=%2; deviceId=9CC2C781CEEC4408333573ACF975B764BB3D6492A63544CD059A; channel=distribution; requestId=%3").arg(GetTime10Digits()).arg(MUSIC_U).arg(generateRequestId()).toUtf8());
    } else {
        request->setRawHeader("Cookie", QString("%1; osver=16.2; deviceId=9CC2C781CEEC4408333573ACF975B764BB3D6492A63544CD059A; os=iPhone OS; appver=9.0.90; versioncode=140; mobilename=; buildver=%2; resolution=1920x1080; channel=distribution; requestId=%3").arg(loginCookieStr).arg(GetTime10Digits()).arg(generateRequestId()).toUtf8());
    }

    // QString cookies_str;
    // QVector<QPair<QString, QString>> cookies = {
    //                                             {"appver", "9.2.35"},
    //                                             //{"buildver", GetTime10Digits()},
    //                                             {"resulution", "1920x1080"},
    //                                             {"os", "iPhone OS"},
    //                                             {"osver", "18.3.2"},
    //                                             {"MUSIC_U", MUSIC_U_value},
    //                                             {"packageType", "release"},
    //                                             {"NMTID" , nmtid_value},
    //                                             };
    // bool first = true;
    // for (auto& [name, value] : cookies) {
    //     if (value.isEmpty()) continue;
    //     if (first) {
    //         cookies_str.append(QString("%1=%2").arg(name, value));
    //         first = false;
    //     } else {
    //         cookies_str.append(QString("; %1=%2").arg(name, value));
    //     }
    // }
    // if (reply != nullptr and !(reply->isRunning())) {
    //     reply->abort();
    //     reply->deleteLater();
    //     reply = nullptr;
    // }
    // headers.append("Cookie", cookies_str);
    // request->setHeaders(headers);
    request->setTransferTimeout(2000);
    reply = manager->post(*request, data);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    // connect(reply, &QNetworkReply::finished, this, &ApiRequester::ApiRequestFinished);
    loop.exec();
    // qDebug() << "AfterPost" << reply->headers();
    QByteArray resp_data = reply->readAll();
    reply->deleteLater();
    reply = nullptr;
    return resp_data;
}

QString ApiRequester::GetTime10Digits() { // 获取当前时间的 Unix 时间戳（秒级）
    qint64 unixTimestamp = QDateTime::currentSecsSinceEpoch();

    // 将时间戳转换为字符串（十进制表示）
    QString timestampStr = QString::number(unixTimestamp);

    // 截取前 10 位字符
    QString truncatedTimestamp = timestampStr.left(10);

    return truncatedTimestamp;
}

QString ApiRequester::ApiRequester::EncodeURIComponent(QString str) {
    str = QUrl(str).toEncoded();
    str.replace("+", "%20");
    return str;
}

QByteArray ApiRequester::Splice(QString path, QString data) {
    const QString nobody_know_this = "36cd479b6b5";
    QString txt = QString("nobody%1use%2md5forencrypt").arg(path, data);
    QByteArray md5_sum = QCryptographicHash::hash(txt.toUtf8(), QCryptographicHash::Md5);
    QString md5_str = md5_sum.toHex();
    md5_str = md5_str.toLower();
    QString res("%1-%2-%3-%4-%5");
    res = res.arg(path, nobody_know_this, data, nobody_know_this, md5_str);

    // // qDebug() << res ;

    return res.toUtf8();
}

QByteArray ApiRequester::ToParams(QByteArray data) {
    QString res("params=%1");
    QString ciph = EapiEncrypt(data).toHex();
    ciph = ciph.toUpper();
    res = res.arg(ciph);
    // // qDebug() << QString("[ToParams] original=%1; %2").arg(data, res);
    return res.toUtf8();
}

QString ApiRequester::ChooseUserAgent() {
    // static const QString userAgentList[] = {
    //     "Mozilla/5.0 (iPhone; CPU iPhone OS 9_1 like Mac OS X) AppleWebKit/601.1.46 (KHTML, like Gecko) Version/9.0 Mobile/13B143 Safari/601.1",
    //     "Mozilla/5.0 (iPhone; CPU iPhone OS 9_1 like Mac OS X) AppleWebKit/601.1.46 (KHTML, like Gecko) Version/9.0 Mobile/13B143 Safari/601.1",
    //     "Mozilla/5.0 (Linux; Android 5.0; SM-G900P Build/LRX21T) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.115 Mobile Safari/537.36",
    //     "Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.115 Mobile Safari/537.36",
    //     "Mozilla/5.0 (Linux; Android 5.1.1; Nexus 6 Build/LYZ28E) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.115 Mobile Safari/537.36",
    //     "Mozilla/5.0 (iPhone; CPU iPhone OS 10_3_2 like Mac OS X) AppleWebKit/603.2.4 (KHTML, like Gecko) Mobile/14F89;GameHelper",
    //     "Mozilla/5.0 (iPhone; CPU iPhone OS 10_0 like Mac OS X) AppleWebKit/602.1.38 (KHTML, like Gecko) Version/10.0 Mobile/14A300 Safari/602.1",
    //     "NeteaseMusic/6.5.0.1575377963(164);Dalvik/2.1.0 (Linux; U; Android 9; MIX 2 MIUI/V12.0.1.0.PDECNXM)",
    //     "NeteaseMusic/2505 CFNetwork/3826.400.120 Darwin/24.3.0"
    // };
    // int idx = QRandomGenerator::global()->bounded(8, 9);
    // return userAgentList[idx];
    return "NeteaseMusic 9.0.90/5038 (iPhone; iOS 16.2; zh_CN)";
}

