#include "LoginWidget.h"

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget{parent}, user{this}
{
    requester = new ApiRequester(this);
    setupUi();
}

void LoginWidget::closeEvent(QCloseEvent *event) {
    timer->stop();
    QMessageBox msgBox;
    msgBox.setText("确定要退出整个程序吗？");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    int reply = msgBox.exec();  // exec() 阻塞当前事件循环，确保消息框不消失
    if (reply == QMessageBox::Yes) {
        emit quitApp();
        event->accept();
        // QTimer::singleShot(0, qApp, &QCoreApplication::quit);  // 更稳妥的退出方式
    } else {
        event->ignore();
        timer->start(2000);
    }
}


void LoginWidget::setupUi() {
    resize(350, 600);

    qrImgLabel = new QLabel(this);
    qrImgLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    qrImgLabel->resize(QSize{210, 210});

    msgLabel = new QLabel(this);
    msgLabel->setText("欢迎使用");

    anonymousLoginButton = new QPushButton(this);
    anonymousLoginButton->setText("游客登录");
    QObject::connect(anonymousLoginButton, &QPushButton::clicked, this, &LoginWidget::anonymousLogin);

    vbLayout = new QVBoxLayout;
    vbLayout->addWidget(qrImgLabel);
    vbLayout->addWidget(msgLabel);
    vbLayout->addWidget(anonymousLoginButton);
    setLayout(vbLayout);

    setWindowTitle("请打开手机端网易云音乐app扫码登录~");
}

void LoginWidget::setCookieJar(QNetworkCookieJar *cookieJar) {
    requester->setCookieJar(cookieJar);
}

void LoginWidget::stopCheckQR() {
    timer->stop();
}

void LoginWidget::createQR() {
    timer = new QTimer(this);
    login_timer_conn = connect(timer, &QTimer::timeout, this, &LoginWidget::checkQRState);

    QJsonDocument json;
    QJsonObject root;

    root["type"] = 3; root["e_r"] = false;
    json.setObject(root);
    json = requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://interface.music.163.com/eapi/login/qrcode/unikey", "/api/login/qrcode/unikey", true);
    qrUnikey = json["unikey"].toString();

    QPixmap qrcode = generateQRCode("https://music.163.com/login?codekey=" + qrUnikey);
    qrImgLabel->setPixmap(qrcode);

    timer->setSingleShot(false);
    timer->start(2000);
}

QPixmap LoginWidget::generateQRCode(QString text, int size) {
    // 生成 QR 码数据
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(text.toUtf8().constData(), qrcodegen::QrCode::Ecc::LOW);
    int qrSize = qr.getSize();

    // 创建 QPixmap 并绘制 QR 码
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setBrush(Qt::black);

    double scale = size / double(qrSize);
    for (int y = 0; y < qrSize; ++y) {
        for (int x = 0; x < qrSize; ++x) {
            if (qr.getModule(x, y)) {
                painter.drawRect(x * scale, y * scale, scale, scale);
            }
        }
    }

    return pixmap;
}

void LoginWidget::checkQRState() {
    QJsonDocument json;
    QJsonObject root;

    root["key"] = qrUnikey;
    root["type"] = 3;
    json.setObject(root);
    json = requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://music.163.com/eapi/login/qrcode/client/login", "/api/login/qrcode/client/login", true);

    if (json["code"] == 803) {
        msgLabel->setText("登录成功!");        
        QObject::disconnect(login_timer_conn);
        timer->stop();
        hide();
        isGuest = false;
        // QEventLoop loop; QTimer::singleShot(2500, &loop, &QEventLoop::quit); loop.exec(); // 防止请求冲突
        emit loginSuccessful(isGuest);
    } else if (json["code"] == 802) {
        // qDebug() << "Nickname: " << json["nickname"].toString();
        msgLabel->setText(json["nickname"].toString());
    } else if (json["code"] == 801) {
        msgLabel->setText("等待扫码");
    } else if (json["code"] == 800) {
        msgLabel->setText("二维码已失效！");
        QObject::disconnect(login_timer_conn);
        timer->stop();
        createQR();
    } else {
        qDebug() << json["code"] << "验证！";
    }
}

void LoginWidget::anonymousLogin() {
    timer->stop();
    QEventLoop loop;
    user.registerAnonymous(requester);
    isGuest = true;

    msgLabel->setText("Success!");
    QObject::disconnect(login_timer_conn);
    hide();
    emit loginSuccessful(isGuest);
}
