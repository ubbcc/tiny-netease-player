#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include "api/Request.h"
#include "api/User.h"
#include "QRCodeGenerator/qrcodegen.hpp"

#include <QMessageBox>
#include <QApplication>
#include <QCloseEvent>
#include <QTimer>

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>

#include <QPainter>

#include <QNetworkCookie>
#include <QNetworkCookieJar>


class LoginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LoginWidget(QWidget *parent = nullptr);

public:
    void createQR();
    QPixmap generateQRCode(QString text, int size = 200);
    void setupUi();
    void setCookieJar(QNetworkCookieJar *cookieJar);

public slots:
    void checkQRState();
    void stopCheckQR();
    void anonymousLogin();

signals:
    void quitApp();

private:
    ApiRequester *requester;
    User user;
    QString qrUnikey;

private:
    QVBoxLayout *vbLayout;
    QLabel *msgLabel, *qrImgLabel;
    QPushButton *anonymousLoginButton;
    QTimer *timer;
    QMetaObject::Connection login_timer_conn;

protected:
    void closeEvent(QCloseEvent *event) override;


signals:
    void loginSuccessful(bool isGuest);
};

#endif // LOGINWIDGET_H
