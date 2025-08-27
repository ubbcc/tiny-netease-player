#ifndef REQUEST_H
#define REQUEST_H

#include "Crypto.h"
#include "../common.h"

#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QTimer>

#include <QRandomGenerator64>
#include <QDateTime>
#include <QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QHttpHeaders>
#include <QCryptographicHash>

#include <QEventLoop>
#include <QString>

// using RequestCallback = void(*)(QNetworkReply *);

class ApiRequester : public QObject {
    Q_OBJECT

public:
    ApiRequester(QObject* parent = nullptr);
    void SearchSong(const QString& keyword, int offset = 0, int limit = PAGE_SIZE);
    void setCookieJar(QNetworkCookieJar *cookieJar);
    QNetworkCookieJar* cookieJar();
    QJsonDocument ApiRequest(const QString &json_text, const QString &url, const QString &path, bool u = false);
    // QJsonDocument WeapiRequest(const QString &json_text, const QString &url, const QString &path);
    QByteArray SendRequest(QByteArray data, QString url, QString path, bool u = false);
    QString generateRequestId();
    bool isBusy();

signals:
    // void ApiRequestFinished();
    void didntLogin();
    void engaged();

// public slots:
    // void onRequestFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* manager;
    QNetworkRequest* request;
    QNetworkReply* reply;

    QJsonDocument resp_json;
    bool is_busy = false;

private:
    QString EncodeURIComponent(QString str);
    QByteArray Splice(QString path, QString data);
    QByteArray ToParams(QByteArray data);
    QString ChooseUserAgent();
    QString GetTime10Digits();
};

#endif // REQUEST_H
