#include "mainwindow.h"
#include "common.h"
#include "ui_mainwindow.h"
#include <qpainter.h>

SongItemDelegate::SongItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {
    favourite_icon = new QIcon(":/favourite_icon.svg");
    not_favourite_icon = new QIcon(":/not_favourite_icon.svg");
    download_icon = new QIcon(":/download_icon.png");
    downloaded_icon = new QIcon(":/check.svg");
    comment_icon = new QIcon(":/comment_icon.svg");
}

QWidget *SongItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return nullptr;
}

void SongItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    const QAbstractItemModel *model = index.model();
    QString modelName = model->headerData(0, Qt::Horizontal, Qt::EditRole).toString();

    if (modelName == "song_model") {
        if (index.column() == 9)  {
            // 下载
            // if (model->data(index).toBool()) {
            //     download_icon->paint(painter, option.rect);
            // } else {
            //     download_icon->paint(painter, option.rect);
            // }
            auto [portion, type] = model->data(index).value<QPair<double, QString>>();
            if (portion == 0) {
                download_icon->paint(painter, option.rect);
            } else if (portion == 1.0) {
                if (type == "ButtonDownload" or type == "MenuDownload") {
                    downloaded_icon->paint(painter, option.rect);
                } else {

                    download_icon->paint(painter, option.rect);
                }
            } else {
                QTextOption textOption;
                textOption.setAlignment(Qt::AlignCenter);
                painter->drawText(option.rect,
                                  QString("%1%").arg(portion * 100, 0, 'f', 1),
                                  textOption);
            }
        } else if (index.column() == 10) {
            // 喜欢
            if (model->data(index).toBool()) {
                favourite_icon->paint(painter, option.rect);
            } else {
                not_favourite_icon->paint(painter, option.rect);
            }
        } else if (index.column() == 11) {
            // 评论
            comment_icon->paint(painter, option.rect);
        }else {
            QStyledItemDelegate::paint(painter, option, index);
        }
    } else if (modelName == "artist_model") {
        if (index.column() == 2) {
            // 关注
            if (model->data(index).toBool()) {
                favourite_icon->paint(painter, option.rect);
            } else {
                not_favourite_icon->paint(painter, option.rect);
            }
        } else {
            QStyledItemDelegate::paint(painter, option, index);
        }
    } else if (modelName == "user_model") {
        if (index.column() == 4) {
            // 关注
            if (model->data(index).toBool()) {
                favourite_icon->paint(painter, option.rect);
            } else {
                not_favourite_icon->paint(painter, option.rect);
            }
        } else {
            QStyledItemDelegate::paint(painter, option, index);
        }
    } else if (modelName == "playlist_model") {
        if (index.column() == 5) {
            // 收藏歌单
            if (model->data(index).toInt() == 1) {
                favourite_icon->paint(painter, option.rect);
            } else if (model->data(index).toInt() == 0){
                not_favourite_icon->paint(painter, option.rect);
            }
        } else {
            QStyledItemDelegate::paint(painter, option, index);
        }
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize SongItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // 下载列和喜欢列图标 25x25
    if (index.column() == 10 || index.column() == 9) {
        return QSize(25, 25);
    }  else {
        QSize sz = QStyledItemDelegate::sizeHint(option, index);
        sz.setWidth(qMin(sz.width(), 200));
        return sz;
    }
}


void MainWindow::GetDefaultKeywords() {
    QJsonDocument json;
    QJsonObject root;
    root["limit"] = 6;
    root["positionCode"] = "homepage_default_word";
    root["e_r"] = true;
    json.setObject(root);
    json = suggestion_requester->ApiRequest(json.toJson(QJsonDocument::Compact), "https://interface3.music.163.com/eapi/search/default/keyword/list", "/api/search/default/keyword/list");

    if (json["code"].toInt()/100 != 2) {
        return;
    }
    QString defaultKeyword = json["data"]["keywords"][0]["showKeyword"].toString();
    ui->lineEdit->setPlaceholderText(defaultKeyword);

    suggest_keywords.clear();

    // QStringList defaultKeywords;
    for (auto kw : json["data"]["keywords"].toArray()) {
        QJsonValue kwv = kw;
        suggest_keywords[kwv["showKeyword"].toString()] = kwv["realkeyword"].toString();
        // defaultKeywords.push_back(qMakePair(kwv["showKeyword"].toString(),kwv["realkeyword"].toString()));
    }
    return;
}

void MainWindow::RefreshKeywordCompleteModel() {
    if (ui->comboBox->currentText() != "歌曲") {
        ui->lineEdit->setCompleter(nullptr);
        return;
    }
    ui->lineEdit->setCompleter(completer);


    QJsonDocument json;
    QJsonObject root;
    root["keyword"] = ui->lineEdit->text();
    root["e_r"] = true;
    root["verifyId"] = 1;
    root["header"] = QJsonObject();
    json.setObject(root);
    json = suggestion_requester->ApiRequest(json.toJson(QJsonDocument::Compact),
                                            "https://interface3.music.163.com/eapi/search/suggest/keyword/get",
                                            "/api/search/suggest/keyword/get");
    if (json["code"].toInt()/100 != 2) {
        return;
    }
    completeModel->clear();
    for (auto kwitemr : json["data"]["suggests"].toArray()) {
        QJsonValue kwJsonItem = kwitemr;
        QStandardItem *kwItem = new QStandardItem;
        QString fill_keyword = kwJsonItem["keyword"].toString();
        if (kwJsonItem["tagUrl"] == "https://p6.music.126.net/obj/wo3DlcOGw6DClTvDisK1/7828749870/a17d/ce93/e4af/c3897966025f76c1258cd4b27880775d.png") {
            fill_keyword += "❤️";
        }
        kwItem->setData(fill_keyword, Qt::DisplayRole);
        completeModel->appendRow(kwItem);
    }
    completer->complete();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , requester(new ApiRequester(this))
    , suggestion_requester(new ApiRequester(this))
    , user_requester(new ApiRequester(this))
    // , downloader(new Downloader(this))
    , song_model(new QStandardItemModel(this))
    , artist_model(new QStandardItemModel(this))
    , album_model(new QStandardItemModel(this))
    , user_model(new QStandardItemModel(this))
    , followed_model(new QStandardItemModel(this))
    , playlist_model(new QStandardItemModel(this))
    , rcmd_playlist_model(new QStandardItemModel(this))
    , comments_model(new QStandardItemModel(this))
    , completeModel(new QStandardItemModel)
    , delegate(new SongItemDelegate(this))
    , completer(new QCompleter(this))
    , user(this)
    , playlist(this)
    , search(this)
    , lyric(this)
    , comment(this)
    , offset(0)
    , comment_offset(0)
    , refreshTimer(new QTimer(this))
    , undoStack(this)
    , homepageWidget(new QWidget(this))
    , imageLabelEventFilter(new ImageLabelEventFilter(this))
    , commentsDialog(new CommentsDialog(this))
    , cursor(1)
{
    ui->setupUi(this);

    Setup();
    if (is_ending) {
        QTimer::singleShot(0, qApp, &QApplication::quit);
        return;
    }
    SetupDetailWidget();

    setCentralWidget(detailWidget);
}

MainWindow::~MainWindow()
{
    SaveSettings();
    delete ui;
}

void MainWindow::Refreshing() {
    this->ui->pushButton->setEnabled(false);
    this->ui->pageUpButton->setEnabled(false);
    this->ui->pageDownButton->setEnabled(false);
    this->ui->homepageButton->setEnabled(false);
    this->ui->undoButton->setEnabled(false);
    this->ui->playlistPageDownButton->setEnabled(false);
    this->ui->playlistPageDownButton->hide();
    this->ui->lineEdit->setEnabled(false);
    ui->downloadCoverPicButton->hide();

    if (currentWidget == kHomePage) {
        homepageWidget->hide();
    } else {
        ui->tableView->hide();
        ui->imageLabel->hide();
        ui->descriptionLabel->hide();
        ui->downloadCoverPicButton->hide();
        ui->descriptionScrollArea->hide();
        ui->creator_button->hide();
        ui->rcmd_button->hide();
        ui->sub_button->hide();
    }
    setFocus();

    statusBar()->showMessage("刷新中，请耐心等待哦...");
}

void MainWindow::Refreshed() {
    this->ui->pushButton->setEnabled(true);
    this->ui->pageUpButton->setEnabled(isPageButtonEnabled);
    this->ui->pageDownButton->setEnabled(isPageButtonEnabled);
    this->ui->homepageButton->setEnabled(true);
    this->ui->undoButton->setEnabled(true);
    this->ui->playlistPageDownButton->setEnabled(true);
    this->ui->lineEdit->setEnabled(true);

    if (currentWidget == kHomePage) {
        homepageWidget->show();
    } else {
        ui->tableView->show();
    }

    setFocus();
    statusBar()->showMessage("刷新完成啦~~~");
}

QString MainWindow::ReadMusicU(const QString &file_name) {
    QString appDir = QCoreApplication::applicationDirPath();

    QDir dir(appDir);
    QDir parentDir = dir;
    QString filePath = parentDir.absoluteFilePath(file_name);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // qDebug() << "无法打开文件：" << file.errorString();
        return "";
    }

    QByteArray content = file.readAll();
    // content = MyDec(content, cookiesKey);

    file.close();

    return content.trimmed();
}

void MainWindow::LoadSettings() {
    QString settingsPath = cwd.absoluteFilePath("settings.ini");
    if (not QFile::exists(settingsPath)) {
        return;
    }

    QSettings settings(settingsPath, QSettings::IniFormat);
    user_nick_name = settings.value("user_name").toString();
    user_id = settings.value("user_id").toLongLong();
    TEMP_DIR = settings.value("TEMP_DIR").toString();
    TEMP_PIC_DIR = TEMP_DIR.absoluteFilePath("PictureCache");
    DOWNLOAD_DIR = settings.value("DOWNLOAD_DIR").toString();
    HOMEPAGE_BLOCKS_ORDER_LIST = settings.value("HOMEPAGE_BLOCKS_ORDER_LIST").toJsonArray();
    quality = static_cast<Level>(settings.value("quality").toInt());
}

void MainWindow::SaveSettings() {
    QString settingsPath = cwd.absoluteFilePath("settings.ini");
    QSettings settings(settingsPath, QSettings::IniFormat);
    // qDebug() << "save settings" << user_nick_name;
    settings.setValue("user_name", static_cast<QString>(this->user_nick_name));
    settings.setValue("user_id", static_cast<qint64>(this->user_id));
    settings.setValue("TEMP_DIR", TEMP_DIR.absolutePath());
    settings.setValue("DOWNLOAD_DIR", DOWNLOAD_DIR.absolutePath());
    settings.setValue("HOMEPAGE_BLOCKS_ORDER_LIST", HOMEPAGE_BLOCKS_ORDER_LIST);
    settings.setValue("quality", static_cast<int>(quality));
}

QString MainWindow::GetTime10Digits() { // 获取当前时间的 Unix 时间戳（秒级）
    qint64 unixTimestamp = QDateTime::currentSecsSinceEpoch();

    // 将时间戳转换为字符串（十进制表示）
    QString timestampStr = QString::number(unixTimestamp);

    // 截取前 10 位字符
    QString truncatedTimestamp = timestampStr.left(10);

    return truncatedTimestamp;
}

// QList<QNetworkCookie> MainWindow::myParseCookies(const QString &cookieStr) {
//     QList<QNetworkCookie> cookiesList;
//     QString cleanStr = cookieStr.trimmed();
//     cleanStr.replace('\n', ';');
//     cleanStr.replace('\r', ';');
//     QStringList cookieParts = cleanStr.split(';', Qt::SkipEmptyParts);
//     for (const QString &part : cookieParts) {
//         QString trimmedPart = part.trimmed();
//         if (trimmedPart.isEmpty() || !trimmedPart.contains('=')) {
//             continue;
//         }

//         // 5. 使用 .section() 进行分割，这是最推荐的方式
//         QByteArray name = trimmedPart.section('=', 0, 0).toUtf8().trimmed();
//         QByteArray value = trimmedPart.section('=', 1).toUtf8().trimmed();

//         if (name.isEmpty()) {
//             continue;
//         }

//         // 6. 创建一个 QNetworkCookie 对象并添加到列表中
//         QNetworkCookie newCookie(name, value);
//         cookiesList.append(newCookie);
//     }

//     return cookiesList;
// }

void MainWindow::SaveMusicAtoFile(QString fileName) {
    QString filePath = cwd.absoluteFilePath(fileName);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法打开文件保存 cookies:" << file.errorString();
        return;
    }

    file.write(loginCookieStr.toUtf8());
    file.close();
}

CookieInputDialog::CookieInputDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("请粘贴您的Cookies");
    // setFixedSize(500, 800);

    this->setModal(true);
    m_cookieEdit = new QTextEdit(this);
    m_cookieEdit->setPlaceholderText("Paste your cookies here...");
    m_cookieEdit->setAcceptRichText(false);
    m_cookieBtn = new QPushButton(this);
    m_cookieBtn->setText("Confirm");
    m_layout = new QVBoxLayout(this);
    m_layout->addWidget(m_cookieEdit, 5);
    m_layout->addWidget(m_cookieBtn, 1);
    this->setLayout(m_layout);

    QObject::connect(m_cookieBtn, &QPushButton::clicked, this, [=, this](){
        emit checkCookie(m_cookieEdit->toPlainText().trimmed());
    });
}

CookieInputDialog::~CookieInputDialog()
{
}

// 这是实现拦截操作的核心代码
void CookieInputDialog::closeEvent(QCloseEvent *event)
{
    // 创建一个消息框
    QMessageBox::StandardButton result = QMessageBox::question(
        this,
        "确认关闭",                // 标题
        "关闭此窗口将使用游客登录",    // 提示文本
        QMessageBox::Yes | QMessageBox::No, // 按钮组合
        QMessageBox::No                     // 默认按钮
    );

    // 根据用户的选择决定如何处理关闭事件
    if (result == QMessageBox::Yes) {
        // 用户选择“是”，接受关闭事件，窗口将关闭
        event->accept();
    } else {
        // 用户选择“否”，忽略该事件，窗口将不会关闭
        event->ignore();
    }
}

void MainWindow::Setup() {
    QString appDir = QCoreApplication::applicationDirPath();
    QDir dir(appDir);
    // QDir parentDir = QFileInfo(appDir).dir();
    cwd = dir;
    setWindowTitle("Tiny Netease Music");
    QObject::connect(&user, &User::UserInfoGot, this, &MainWindow::SetUserInfo);

    LoadSettings();

    MUSIC_U = ReadMusicU("music_u_.txt");
    ApiRequester *VIPRequester = new ApiRequester(this);
    if (!user.CheckVIPSt(VIPRequester)) {
        is_ending = true;
        QMessageBox::warning(this, "", "服务器已更新，破解失败！请联系开发者获取更新版本！");
        return;
    }

    loginCookieStr = ReadMusicU("music_a.txt");
    isGuest = true;
    if (loginCookieStr.isEmpty() or !user.checkLogin(requester)) {
        hide();

        CookieInputDialog cookieDialog(this);
        QObject::connect(&cookieDialog, &CookieInputDialog::checkCookie, this, [&](QString cookieStr){
            loginCookieStr = cookieStr;
            if(cookieStr.isEmpty() or !user.checkLogin(requester)) {
                QMessageBox::warning(&cookieDialog, "Warning", "Cookies invalid!");
            } else {
                cookieDialog.accept();
            }
        });
        cookieDialog.exec();
        SaveMusicAtoFile();
        user.GetUserInfo(requester);
    } else {
        if (user_nick_name == "Guest") {
            SetUserInfo(user_id, user_nick_name);
        } else {
            user.GetUserInfo(requester);
        }
    }

    QObject::connect(requester, &ApiRequester::engaged, this, [&](){
        QMessageBox msgbox(this);
        // msgbox.warning(this, "", "点击太快啦～～～");
        msgbox.setModal(true);
        msgbox.setWindowTitle("");
        msgbox.setText("点击太快啦～～～");
        msgbox.setFocus();
        msgbox.exec();
    });
    QObject::connect(ui->homepageButton, &QPushButton::clicked, this, [&]() {
        ui->tableView->hide();
        ui->imageLabel->hide();
        ui->descriptionLabel->hide();
        ui->downloadCoverPicButton->hide();
        ui->descriptionScrollArea->hide();
        ui->creator_button->hide();
        ui->rcmd_button->hide();
        ui->sub_button->hide();
        ui->gridLayout->removeWidget(ui->tableView);

        if (currentWidget == kHomePage) {
            cursor++;
            SetupHomepageWidget();
        } else {
            undoStack.clearAll();
            undoStack.push(Command{"Refresh", "Homepage", {}});
            currentWidget = kHomePage;
        }

        ui->gridLayout->addWidget(homepageWidget, 1, 0, 3, 8);
        homepageWidget->show();
    });

    QObject::connect(imageLabelEventFilter, &ImageLabelEventFilter::imageLabelDoubleClicked, this, [&](const QString& type, QVariant resourceId) {
        if (type == "Playlist" or type == "toplist") {
            offset = 0;
            undoStack.push(Command{"Refresh", "PlaylistDetail", {resourceId}});
            RefreshPlaylistDetail(resourceId.toLongLong());
        } else if (type == "my_playlist") {
            undoStack.push(Command{"Refresh", "MydefPlaylist", {resourceId}});
            RefreshMydefPlaylist(resourceId.toList());
        } else if (type == "album") {
            undoStack.push(Command{"Refresh", "AlbumDetail", {resourceId}});
            RefreshAlbumDetail(resourceId.toLongLong());
        } else if (type == "song") {
            undoStack.push(Command{"Refresh", "MydefPlaylist", {resourceId}});
            RefreshMydefPlaylist({resourceId.toLongLong()});
        }
    });

    SetupMenu();

    downloader = new Downloader(this);
    QObject::connect(downloader, &Downloader::DownloadMessageGenerated, ui->statusbar, &QStatusBar::showMessage);

    undoStack.clearAll();
    undoStack.push(Command{"Refresh", "Homepage", {}});
    currentWidget = kHomePage;

    commentsDialog->setCommentModel(comments_model);
    commentsDialog->hide();
    show();
    ui->tableView->hide();
    ui->imageLabel->hide();
    ui->descriptionLabel->hide();
    ui->downloadCoverPicButton->hide();
    ui->descriptionScrollArea->hide();
    ui->creator_button->hide();
    ui->rcmd_button->hide();
    ui->sub_button->hide();
    ui->gridLayout->removeWidget(ui->tableView);
    SetupHomepageWidget();
    ui->gridLayout->addWidget(homepageWidget, 1, 0, 3, 6);
    homepageWidget->show();
    isPageButtonEnabled = true;
    QGridLayout *descriptionScrollAreaWidgetContentsLayout = new QGridLayout;
    descriptionScrollAreaWidgetContentsLayout->addWidget(ui->descriptionLabel);
    ui->descriptionScrollAreaWidgetContents->setLayout(descriptionScrollAreaWidgetContentsLayout);
    QObject::connect(requester, &ApiRequester::didntLogin, this, &MainWindow::Relogin);
}

void MainWindow::SetupMenu() {
    QMenuBar *menubar = menuBar();

    QMenu *meMenu = menubar->addMenu("我的");

    dailyRecommendAction = new QAction("每日推荐", this);
    QObject::connect(dailyRecommendAction, &QAction::triggered, this, [&]() {
        isPageButtonEnabled = false;
        undoStack.push(Command{"Refresh", "DailyRecommend", {}});
        RefreshDailyRecommend();
    });
    meMenu->addAction(dailyRecommendAction);

    myFavouriteAction = new QAction("我的喜欢", this);
    QObject::connect(myFavouriteAction, &QAction::triggered, this, [&]() {
        isPageButtonEnabled = false;
        undoStack.push(Command{"Refresh", "MyFavouritePlaylist", {}});
        RefreshMyFavouritePlaylist();
    });
    meMenu->addAction(myFavouriteAction);

    myPlaylistAction = new QAction("我的歌单", this);
    QObject::connect(myPlaylistAction, &QAction::triggered, this, [&]() {
        isPageButtonEnabled = false;
        undoStack.push(Command{"Refresh", "MyPlaylist", {}});
        RefreshMyPlaylist();
    });
    meMenu->addAction(myPlaylistAction);

    createPlaylistAction = new QAction("新建歌单", this);
    QObject::connect(createPlaylistAction, &QAction::triggered, this, [&]() {        
        AddToPlaylistDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            QString newPlaylistName = dialog.getNewPlaylistname();
            bool isPrivate = dialog.isPrivatePlaylist();
            QJsonDocument json = playlist.CreatePlaylist(requester, newPlaylistName, (isPrivate ? 10 : 0));
            if (json["code"] != 200) {
                QMessageBox::warning(this, "", QString("创建歌单失败！\n%1\n%2").arg(json["code"].toInt()).arg(json["message"].toString()));
                return;
            }
        }

    });
    meMenu->addAction(createPlaylistAction);

    myFollowAction = new QAction("我的关注", this);
    QObject::connect(myFollowAction, &QAction::triggered, this, [&]() {
        isPageButtonEnabled = false;
        undoStack.push(Command{"Refresh", "MyFollowedUser", {}});
        RefreshMyFollowedUser();
    });
    meMenu->addAction(myFollowAction);

    myRcmdFollowAction = new QAction("推荐关注", this);
    QObject::connect(myRcmdFollowAction, &QAction::triggered, this, [&]() {
        isPageButtonEnabled = false;
        undoStack.push(Command{"Refresh", "MyRcmdUser", {}});
        RefreshMyRcmdUser();
    });
    meMenu->addAction(myRcmdFollowAction);

    meMenu->addSeparator();

    logoutAction = new QAction("退出登录", this);
    QObject::connect(logoutAction, &QAction::triggered, this, [&]() {
        QMessageBox::StandardButton choice = QMessageBox::warning(this, "", "你真的要退出登录并退出主程序吗？", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
        if (choice == QMessageBox::Ok) {
            QString musicaPath = cwd.absoluteFilePath("music_a.txt");
            QFile::remove(musicaPath);
            is_ending= true;
            qApp->quit();
        }
    });
    meMenu->addAction(logoutAction);

    QMenu *aboutMenu = menubar->addMenu("关于");

    aboutAction = new QAction("关于作者", this);
    QObject::connect(aboutAction, &QAction::triggered, this, [&]() {
        QMessageBox::information(this, "关于作者", "Accelerator Pan\nPeter He\n2025");
    });
    aboutMenu->addAction(aboutAction);

    settingsAction = new QAction("设置", this);
    QObject::connect(settingsAction, &QAction::triggered, this, [&]() {
        SettingsDialog dialog;
        if (dialog.exec() == QDialog::Accepted) {
            TEMP_DIR = dialog.getTempDir();
            TEMP_PIC_DIR = TEMP_DIR.absoluteFilePath("PictureCache");
            QJsonArray prev_HOMEPAGE_BLOCKS_ORDER_LIST = HOMEPAGE_BLOCKS_ORDER_LIST;
            HOMEPAGE_BLOCKS_ORDER_LIST = dialog.getBlockCodeOrderList();
            DOWNLOAD_DIR = dialog.getDownloadDir();
            quality = dialog.getDownloadQuality();

            if (prev_HOMEPAGE_BLOCKS_ORDER_LIST != HOMEPAGE_BLOCKS_ORDER_LIST) {
                ui->tableView->hide();
                ui->imageLabel->hide();
                ui->descriptionLabel->hide();
                ui->downloadCoverPicButton->hide();
                ui->descriptionScrollArea->hide();
                ui->creator_button->hide();
                ui->rcmd_button->hide();
                ui->sub_button->hide();
                ui->gridLayout->removeWidget(ui->tableView);

                currentWidget = kHomePage;
                SetupHomepageWidget();

                ui->gridLayout->addWidget(homepageWidget, 1, 0, 3, 6);
                homepageWidget->show();
            }
            SaveSettings();
        }
    });
    aboutMenu->addAction(settingsAction);
}

void MainWindow::SetupDetailWidget() {
    table_view_old_geometry = table_view_rect = ui->tableView->geometry();
    table_view_old_geometry.setTop(std::min(ui->imageLabel->geometry().y(), ui->descriptionLabel->geometry().y()));
    ui->tableView->setGeometry(table_view_old_geometry);
    detailWidget = ui->centralwidget;

    ui->descriptionScrollArea->setWidgetResizable(true);

    QObject::connect(ui->tableView, &QTableView::doubleClicked, this, &MainWindow::OnCellDoubleClicked);
    QObject::connect(ui->tableView, &QTableView::clicked, this, &MainWindow::OnCellClicked);
    QObject::connect(this, &MainWindow::DownloadButtonClicked, this, [this](int rowNum, qint64 id, QString artistsStr, QString name, QString dir) {
        QModelIndex idx = song_model->index(rowNum, 9);
        if (song_model->data(idx).toDouble() > 0) {
            return;
        }

        Downloader *singleDownloader = new Downloader(this);
        QObject::connect(singleDownloader, &Downloader::DownloadPortionUpdated, this, [this](int rowNum, qint64 songId, double portion, QString type) {
            if (song_model->rowCount() <= rowNum) {
                return;
            }
            QModelIndex idx = song_model->index(rowNum, 0);
            qint64 modelSongId = song_model->data(idx).toLongLong();
            // qDebug() << "DownloadPortionUpdated" << idx << songId << modelSongId;
            if (modelSongId != songId) {
                return;
            }
            QPair<double, QString> info{portion, type};
            idx = song_model->index(rowNum, 9);
            song_model->setData(idx, QVariant::fromValue(info));
            ui->tableView->update(idx);
        });
        // qDebug() << "downloadbutton" << idx << rowNum << id;
        singleDownloader->setProperty("songModelRowNum", rowNum);
        singleDownloader->setProperty("songId", id);
        singleDownloader->setProperty("type", "ButtonDownload");
        singleDownloader->SongDownload(id, artistsStr, name, dir);
    });
    QObject::connect(ui->pushButton, &QPushButton::clicked, this, [&]() {
        if (ui->lineEdit->text() == "") {
            ui->lineEdit->setText(suggest_keywords[ui->lineEdit->placeholderText()]);
        } else if (suggest_keywords.keys().contains(ui->lineEdit->text())) {
            ui->lineEdit->setText(suggest_keywords[ui->lineEdit->text()]);
        }
        offset = 0;
        ComplexSearch();
    });
    QObject::connect(ui->pageDownButton, &QPushButton::clicked, this, [&]() { offset += PAGE_SIZE; ComplexSearch(); });
    QObject::connect(ui->pageUpButton, &QPushButton::clicked, this, [&]() { offset -= PAGE_SIZE; ComplexSearch(); });
    QObject::connect(ui->comboBox, &QComboBox::activated, this, [&](int index) {
        if (index < 6) {
            ui->lineEdit->setEnabled(true);
            // ui->lineEdit->setText("");
        } else {
            // ui->lineEdit->setEnabled(false);
            // switch (index) {
            // case 6:
            //     ui->lineEdit->setText("点击搜索按钮显示每日推荐歌单");
            //     break;
            // case 7:
            //     ui->lineEdit->setText("点击搜索按钮显示我喜欢的歌单");
            //     break;
            // case 8:
            //     ui->lineEdit->setText("点击搜索按钮显示我的歌单");
            //     break;
            // }
        }
    });
    QObject::connect(ui->comboBox, &QComboBox::currentTextChanged, this, [&](QString now) {
        if (now == "歌曲") {
            ui->lineEdit->setCompleter(completer);
        } else {
            ui->lineEdit->setCompleter(nullptr);
        }
    });
    QObject::connect(ui->undoButton, &QPushButton::clicked, &undoStack, &UndoStack::undo);
    QObject::connect(ui->downloadCoverPicButton, &QPushButton::clicked, this, [&]() {
        // qDebug() << "downloadalpic" << DOWNLOAD_DIR.absoluteFilePath(cover_title) << cover_pixmap;

        cover_pixmap.save(DOWNLOAD_DIR.absoluteFilePath(cover_title) + ".jpg", "JPEG");
    });

    // 关键字建议
    ui->lineEdit->setText("");
    GetDefaultKeywords();
    ui->lineEdit->setPlaceholderText(suggest_keywords.keys()[0]);

    //ui->lineEdit->setText("点击搜索按钮显示每日推荐歌单");
    //ui->lineEdit->setEnabled(false);

    ui->pageUpButton->setEnabled(false);
    ui->pageDownButton->setEnabled(false);

    currentModel = kSongModel;
    ui->tableView->setModel(song_model);
    ConfigSongTableView();
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->tableView, &QTableView::customContextMenuRequested, this, &MainWindow::CreateContextMenu);

    ui->descriptionLabel->lower();
    ui->descriptionLabel->setWordWrap(true);
    ui->imageLabel->lower();

    completer->setModel(completeModel);
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    ui->lineEdit->setCompleter(completer);
    // QObject::connect(ui->lineEdit, &QLineEdit::textEdited, this, &MainWindow::RefreshKeywordCompleteModel);

    refreshTimer->setInterval(300);
    refreshTimer->setSingleShot(true);
    // QObject::connect(ui->lineEdit, &QLineEdit::textEdited, refreshTimer, &QTimer::start);
    QObject::connect(ui->lineEdit, &QLineEdit::textEdited, this, [&]() { refreshTimer->start(); });
    QObject::connect(refreshTimer, &QTimer::timeout, this, &MainWindow::RefreshKeywordCompleteModel);

    QObject::connect(qApp, &QApplication::focusChanged, this, [&](QWidget *old, QWidget *now) {
        if (now == ui->lineEdit and ui->lineEdit->text() == "" and ui->comboBox->currentText() == "歌曲") {
            completeModel->clear();
            for (auto suggest_keyword : suggest_keywords.keys()) {
                // // qDebug() << suggest_keyword << suggest_keywords[suggest_keyword];
                QStandardItem *item = new QStandardItem;
                item->setData(suggest_keyword, Qt::DisplayRole);
                // item->setData(suggest_keywords[suggest_keyword], Qt::EditRole);
                completeModel->appendRow(item);
            }
            completer->complete();
        }
        if (now == ui->lineEdit) {
            conn_return_pressed = QObject::connect(ui->lineEdit, &QLineEdit::returnPressed, this, [&]() {
                if (ui->lineEdit->text() == "") {
                    ui->lineEdit->setText(ui->lineEdit->placeholderText());
                }
                offset = 0;
                ComplexSearch();
            });
        } else {
            if (conn_return_pressed != QMetaObject::Connection()) {
                QObject::disconnect(conn_return_pressed);
            }
        }
    });
}

void MainWindow::Relogin() {
    hide();
    QMessageBox::warning(this, "", "检测到登录信息已过期，请重新登录！");

    hide();
    QDialog cookieDialog(this);
    cookieDialog.setModal(true);
    QTextEdit cookieEdit(&cookieDialog);
    cookieEdit.setPlaceholderText("Paste your cookies here...");
    cookieEdit.setAcceptRichText(false);
    QPushButton cookieBtn(&cookieDialog);
    cookieBtn.setText("Confirm");
    QVBoxLayout layout;
    layout.addWidget(&cookieEdit, 5);
    layout.addWidget(&cookieBtn, 1);
    cookieDialog.setLayout(&layout);
    QObject::connect(&cookieBtn, &QPushButton::clicked, this, [&]() {
        loginCookieStr = cookieEdit.toPlainText().trimmed();
        if (user.checkLogin(requester)) {
            isGuest = false;
            cookieDialog.accept();
        } else {
            QMessageBox::warning(&cookieDialog, "Warning", "Cookies invalid!");
        }
    });
    cookieDialog.exec();
    SaveMusicAtoFile();
    user.GetUserInfo(requester);
    show();
}

QJsonValue MainWindow::findKeyInJsonDocument(const QJsonValue &doc, const QString &prefix) {
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON format!";
        return QJsonValue();
    }

    QJsonObject jsonObj = doc.toObject();

    // 获取所有键的列表（比直接遍历整个对象更快）
    QStringList keys = jsonObj.keys();

    // 使用正则表达式匹配符合条件的键
    QRegularExpression regex("^" + QRegularExpression::escape(prefix) + ".*");

    for (const QString &key : keys) {
        if (regex.match(key).hasMatch()) {
            return jsonObj.value(key); // 只需要找到第一个匹配的键，立刻返回，提高效率
        }
    }

    return QJsonValue();
}

ImageLabelEventFilter::ImageLabelEventFilter(QObject *parent) : QObject(parent) {

}

bool ImageLabelEventFilter::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonDblClick) {
        QVariant type = obj->property("type");
        if (type.isValid()) {
            type = type.toString();
            if (type == "my_playlist") {
                QVariant playlistId = obj->property("song_ids");
                emit imageLabelDoubleClicked(type.toString(), playlistId);
            } else if (type == "Playlist") {
                QVariant playlistId = obj->property("resourceId");
                emit imageLabelDoubleClicked(type.toString(), playlistId);
            } else if (type == "album") {
                QVariant playlistId = obj->property("id");
                emit imageLabelDoubleClicked(type.toString(), playlistId);
            } else if (type == "song") {
                QVariant playlistId = obj->property("id");
                emit imageLabelDoubleClicked(type.toString(), playlistId);
            }

            return true;
        }
    }
    return false;
}

void MainWindow::SetupHomepageWidget() {
    currentWidget = kHomePage;
    Refreshing();

    // 移除旧的 layout
    if (homepageWidget->layout()) {
        QLayout *oldLayout = homepageWidget->layout();
        homepageWidget->setLayout(nullptr);  // 解除绑定
        delete oldLayout;  // 释放内存
    }

    if (homepageWidget) {
        delete homepageWidget;
        homepageWidget = new QWidget(this);
    }

    QJsonObject root;
    QJsonDocument json;
    root["refresh"] = 1;
    root["callbackParameters"] = "{\"likePosition\":1}";
    root["pageStyleType"] = "noCutBlock";
    // root["blockCodeOrderList"] = "[\"PAGE_RECOMMEND_SPECIAL_CLOUD_VILLAGE_PLAYLIST\"]";
    // QJsonArray blockCodeOrderList{"PAGE_RECOMMEND_MY_SHEET", "PAGE_RECOMMEND_MONTH_YEAR_PLAYLIST", "PAGE_RECOMMEND_FIRM_PLAYLIST"};
    root["blockCodeOrderList"] = QString::fromUtf8(QJsonDocument(HOMEPAGE_BLOCKS_ORDER_LIST).toJson(QJsonDocument::Compact));
    // root["blockCodeOrderList"] = "[\"PAGE_RECOMMEND_COMBINATION\","
    //                                  "\"PAGE_RECOMMEND_PRIVATE_RCMD_SONG\","
    //                                  "\"PAGE_RECOMMEND_RADAR\","
    //                                  "\"PAGE_RECOMMEND_FEELING_PLAYLIST_LOCATION\","
    //                                  "\"PAGE_RECOMMEND_MY_SHEET\","
    //                                  "\"PAGE_RECOMMEND_SCENE_PLAYLIST_LOCATION\","
    //                                  "\"PAGE_RECOMMEND_RANK\","
    //                                  "\"PAGE_RECOMMEND_ARTIST_TREND\","
    //                                  "\"PAGE_RECOMMEND_STYLE_PLAYLIST_1\","
    //                                  "\"PAGE_RECOMMEND_SPECIAL_CLOUD_VILLAGE_PLAYLIST\","
    //                                  "\"PAGE_RECOMMEND_FIRM_PLAYLIST\","
    //                                  "\"PAGE_RECOMMEND_NEW_SONG_AND_ALBUM\","
    //                                  "\"PAGE_RECOMMEND_SPECIAL_ORIGIN_SONG_LOCATION\","
    //                                  "\"PAGE_RECOMMEND_MONTH_YEAR_PLAYLIST\","
    //                                  "\"PAGE_RECOMMEND_LBS\","
    //                                  "\"PAGE_RECOMMEND_RED_SIMILAR_SONG\"]";
    root["verifyId"] = 1;
    root["cursor"] = cursor;
    root["pageCode"] = "HOME_RECOMMEND_PAGE";
    root["e_r"] = true;
    root["clientCacheBlockCode"] = "[]";
    root["isFirstScreen"] = false;
    QString title, resourceType, coverImg;

    global_layout = new QVBoxLayout();
    global_scroll_area = new QScrollArea();
    global_scroll_area_content = new QWidget();
    global_vbox = new QVBoxLayout();

    int cnt = 0;

    do {
        // // qDebug() << "Supported image formats:" << QImageReader::supportedImageFormats();
        json = QJsonDocument();
        json.setObject(root);
        // // qDebug() << json;
        json = requester->ApiRequest(json.toJson(QJsonDocument::Compact),
                                     "https://interface3.music.163.com/eapi/link/page/rcmd/resource/show",
                                     "/api/link/page/rcmd/resource/show");
        if (json["code"].toInt()/100 != 2) {
            break;
        }
        QJsonValue blocks = json["data"]["blocks"];
        cnt ++;
        for (auto blkRef : blocks.toArray()) {
            QJsonValue block = blkRef;
            if (block["bizCode"] == QJsonValue::Undefined) continue;

            QGroupBox *groupbox = new QGroupBox(homepageWidget);
            groupbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            QVBoxLayout *groupbox_layout = new QVBoxLayout(groupbox);
            groupbox->setLayout(groupbox_layout);

            QScrollArea *scroll_area = new QScrollArea(homepageWidget);
            QWidget *scroll_area_content = new QWidget(scroll_area);

            QGridLayout *grid_layout = new QGridLayout(scroll_area_content);
            grid_layout->setRowStretch(0, 0);
            grid_layout->setRowStretch(1, 0);

            if (block["bizCode"] == "PAGE_RECOMMEND_COMBINATION") {
                QJsonValue block_resource = block["dslData"]["blockResource"];
                title = block_resource["title"].toString();
                groupbox->setTitle(title);
                int cnt = -1;
                for (auto resource : block_resource["resources"].toArray()) {
                    QJsonValue resourceObj = resource.toObject();
                    resourceType = resourceObj["resourceType"].toString();
                    if (resourceType != "playList" and resourceType != "playlist") {
                        continue;
                    }
                    cnt++;
                    title = resourceObj["title"].toString()
                            + (resourceObj["subTitle"] == QJsonValue::Undefined ? "" : "\n" + resourceObj["subTitle"].toString())
                            + (resourceObj["resourceInteractInfo"]["playCount"] == QJsonValue::Undefined ? "" : "\n(" + resourceObj["resourceInteractInfo"]["playCount"].toString() + ")");
                    QLabel *lb = new QLabel(scroll_area_content);
                    lb->setText(title);
                    lb->setMaximumWidth(imglb_size.width());
                    lb->setWordWrap(true);
                    coverImg = resourceObj["coverImg"].toString();
                    QLabel *imglb = new QLabel(scroll_area_content);
                    imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                    imglb->setProperty("type", "Playlist");
                    imglb->setProperty("resourceId", resourceObj["resourceId"].toString().toLongLong());
                    imglb->installEventFilter(imageLabelEventFilter);
                    Downloader *img_downloader = new Downloader;
                    QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                        QString file_name;
                        QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                        // 匹配输入字符串
                        QRegularExpressionMatch match = regex.match(coverImg);
                        if (match.hasMatch()) {
                            file_name = match.captured(1); // 获取匹配的文件名
                        } else {
                            file_name = "temp.jpg";
                        }
                        file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                        // file_name = checkAndFixImageExtension(file_name);

                        // // qDebug() << "[imglabel]" << file_name;
                        QPixmap pixmap(file_name);
                        if (pixmap.isNull()) {
                            qWarning() << "imglb lixmap fail";
                        }
                        imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        imglb->setFixedSize(imglb_size);
                        imglb->setPixmap(pixmap.scaled(pic_size));
                    });
                    img_downloader->PictureDownload(coverImg);
                    grid_layout->addWidget(imglb, 0, cnt);
                    grid_layout->addWidget(lb, 1, cnt);
                }
            } else if (block["bizCode"] == "PAGE_RECOMMEND_FEELING_PLAYLIST_LOCATION") {
                QJsonValue block_resource = block["dslData"]["blockResource"];
                title = block_resource["title"].toString();
                groupbox->setTitle(title);
                int cnt = -1;
                for (auto resource : block_resource["resources"].toArray()) {
                    QJsonValue resourceObj = resource.toObject();
                    resourceType = resourceObj["resourceType"].toString();
                    if (resourceType != "playList" and resourceType != "playlist") {
                        continue;
                    }
                    cnt++;
                    title = resourceObj["title"].toString()
                            + (resourceObj["subTitle"] == QJsonValue::Undefined ? "" : "\n" + resourceObj["subTitle"].toString())
                            + (resourceObj["resourceInteractInfo"]["playCount"] == QJsonValue::Undefined ? "" : "\n(" + resourceObj["resourceInteractInfo"]["playCount"].toString() + ")");
                    QLabel *lb = new QLabel(scroll_area_content);
                    lb->setText(title);
                    lb->setMaximumWidth(imglb_size.width());
                    lb->setWordWrap(true);
                    coverImg = resourceObj["coverImg"].toString();
                    QLabel *imglb = new QLabel(scroll_area_content);
                    imglb->setProperty("type", "Playlist");
                    imglb->setProperty("resourceId", resourceObj["resourceId"].toString().toLongLong());
                    imglb->installEventFilter(imageLabelEventFilter);
                    imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                    Downloader *img_downloader = new Downloader;
                    QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                        QString file_name;
                        QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                        // 匹配输入字符串
                        QRegularExpressionMatch match = regex.match(coverImg);
                        if (match.hasMatch()) {
                            file_name = match.captured(1); // 获取匹配的文件名
                        } else {
                            file_name = "temp.jpg";
                        }
                        file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                        // file_name = checkAndFixImageExtension(file_name);

                        // // qDebug() << "[imglabel]" << file_name;
                        QPixmap pixmap(file_name);
                        if (pixmap.isNull()) {
                            qWarning() << "imglb lixmap fail";
                        }
                        imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        imglb->setFixedSize(imglb_size);
                        imglb->setPixmap(pixmap.scaled(pic_size));
                        // // qDebug() << "[feeling]" << imglb->geometry() << lb->geometry();
                    });
                    img_downloader->PictureDownload(coverImg);
                    grid_layout->addWidget(imglb, 0, cnt);
                    grid_layout->addWidget(lb, 1, cnt);
                }
            } else if (block["bizCode"] == "PAGE_RECOMMEND_RADAR") {
                QJsonValue block_resource = findKeyInJsonDocument(block["dslData"],"home_radar_playlist_module")["blockResource"];
                if (block_resource == QJsonValue() || block_resource == QJsonValue::Undefined) {
                    block_resource = block["dslData"]["blockResource"];
                }
                title = block_resource["title"].toString();
                groupbox->setTitle(title);
                int cnt = -1;
                static bool isFirst;
                isFirst = true;
                for (auto resource : block_resource["resources"].toArray()) {
                    QJsonValue resourceObj = resource.toObject();
                    resourceType = resourceObj["resourceType"].toString();
                    if (resourceType != "playList" and resourceType != "playlist") {
                        continue;
                    }
                    cnt++;
                    title = resourceObj["title"].toString()
                            + (resourceObj["subTitle"] == QJsonValue::Undefined ? "" : "\n" + resourceObj["subTitle"].toString())
                            + (resourceObj["resourceInteractInfo"]["playCount"] == QJsonValue::Undefined ? "" : "\n(" + resourceObj["resourceInteractInfo"]["playCount"].toString() + ")");
                    QLabel *lb = new QLabel(scroll_area_content);
                    lb->setText(title);
                    lb->setMaximumWidth(imglb_size.width());
                    lb->setWordWrap(true);
                    coverImg = resourceObj["coverImg"].toString();
                    QLabel *imglb = new QLabel(scroll_area_content);
                    imglb->setProperty("type", "Playlist");
                    imglb->setProperty("resourceId", resourceObj["resourceId"].toString().toLongLong());
                    imglb->installEventFilter(imageLabelEventFilter);
                    imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                    // // qDebug() << "igmlb type resid coverimg" << imglb->property("type") << imglb->property("resourceId") << coverImg;
                    Downloader *img_downloader = new Downloader;
                    QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                        QString file_name;
                        QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                        // 匹配输入字符串
                        QRegularExpressionMatch match = regex.match(coverImg);
                        if (match.hasMatch()) {
                            file_name = match.captured(1); // 获取匹配的文件名
                        } else {
                            file_name = "temp.jpg";
                        }
                        file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                        // file_name = checkAndFixImageExtension(file_name);

                        // // qDebug() << "[imglabel]" << file_name;
                        QPixmap pixmap(file_name);
                        if (pixmap.isNull()) {
                            qWarning() << "imglb lixmap fail";
                        }
                        imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        imglb->setFixedSize(imglb_size);
                        imglb->setPixmap(pixmap.scaled(pic_size));
                    });
                    img_downloader->PictureDownload(coverImg);
                    grid_layout->addWidget(imglb, 0, cnt);
                    grid_layout->addWidget(lb, 1, cnt);
                }
            } else if (block["bizCode"] == "PAGE_RECOMMEND_SPECIAL_CLOUD_VILLAGE_PLAYLIST") {
                QJsonValue block_resource = findKeyInJsonDocument(block["dslData"], "home_page_common_playlist_module")["blockResource"];
                if (block_resource == QJsonValue::Undefined) {
                    block_resource = block["dslData"]["blockResource"];
                }
                title = block_resource["title"].toString();
                groupbox->setTitle(title);
                int cnt = -1;
                for (auto resource : block_resource["resources"].toArray()) {
                    QJsonValue resourceObj = resource.toObject();
                    resourceType = resourceObj["resourceType"].toString();
                    if (resourceType != "playList" and resourceType != "playlist") {
                        continue;
                    }
                    cnt++;
                    title = resourceObj["title"].toString()
                            + (resourceObj["subTitle"] == QJsonValue::Undefined ? "" : "\n" + resourceObj["subTitle"].toString())
                            + (resourceObj["resourceInteractInfo"]["playCount"] == QJsonValue::Undefined ? "" : "\n(" + resourceObj["resourceInteractInfo"]["playCount"].toString() + ")");
                    QLabel *lb = new QLabel;
                    lb->setText(title);
                    lb->setMaximumWidth(imglb_size.width());
                    lb->setWordWrap(true);
                    coverImg = resourceObj["coverImg"].toString();
                    QLabel *imglb = new QLabel(scroll_area_content);
                    imglb->setProperty("type", "Playlist");
                    imglb->setProperty("resourceId", resourceObj["resourceId"].toString().toLongLong());
                    imglb->installEventFilter(imageLabelEventFilter);
                    imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                    Downloader *img_downloader = new Downloader;
                    QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                        QString file_name;
                        QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                        // 匹配输入字符串
                        QRegularExpressionMatch match = regex.match(coverImg);
                        if (match.hasMatch()) {
                            file_name = match.captured(1); // 获取匹配的文件名
                        } else {
                            file_name = "temp.jpg";
                        }
                        file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                        // file_name = checkAndFixImageExtension(file_name);

                        // // qDebug() << "[imglabel]" << file_name;
                        QPixmap pixmap(file_name);
                        if (pixmap.isNull()) {
                            qWarning() << "imglb lixmap fail";
                        }
                        imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        imglb->setFixedSize(imglb_size);
                        imglb->setPixmap(pixmap.scaled(pic_size));
                        update();
                    });
                    grid_layout->addWidget(imglb, 0, cnt);
                    grid_layout->addWidget(lb, 1, cnt);
                    img_downloader->PictureDownload(coverImg);
                }
            } else if (block["bizCode"] == "PAGE_RECOMMEND_RANK") {
                QJsonValue block_resource = findKeyInJsonDocument(block["dslData"],"rcmd_rank_module");
                if (block_resource == QJsonValue()) {
                    block_resource = block["dslData"]["blockResource"];
                }
                title = block_resource["title"].toString();
                groupbox->setTitle(title);
                int cnt = -1;
                for (auto resource : block_resource["resources"].toArray()) {
                    QJsonValue resourceObj = resource.toObject();
                    resourceType = resourceObj["resourceType"].toString();
                    if (resourceType != "playList" and resourceType != "playlist" and resourceType != "toplist") {
                        continue;
                    }
                    cnt++;
                    title = resourceObj["title"].toString()
                            + (resourceObj["subTitle"] == QJsonValue::Undefined ? "" : "\n" + resourceObj["subTitle"].toString())
                            + (resourceObj["playCount"] == QJsonValue::Undefined ? "" : "\n(" + resourceObj["playCount"].toString() + ")");
                    QLabel *lb = new QLabel;
                    lb->setText(title);
                    lb->setMaximumWidth(imglb_size.width());
                    lb->setWordWrap(true);
                    coverImg = resourceObj["coverImg"].toString();
                    QLabel *imglb = new QLabel(scroll_area_content);
                    imglb->setProperty("type", "Playlist");
                    imglb->setProperty("resourceId", resourceObj["resourceId"].toString().toLongLong());
                    imglb->installEventFilter(imageLabelEventFilter);
                    imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                    Downloader *img_downloader = new Downloader;
                    QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                        QString file_name;
                        QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                        // 匹配输入字符串
                        QRegularExpressionMatch match = regex.match(coverImg);
                        if (match.hasMatch()) {
                            file_name = match.captured(1); // 获取匹配的文件名
                        } else {
                            file_name = "temp.jpg";
                        }
                        file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                        // file_name = checkAndFixImageExtension(file_name);

                        // // qDebug() << "[imglabel]" << file_name;
                        QPixmap pixmap(file_name);
                        if (pixmap.isNull()) {
                            qWarning() << "imglb lixmap fail";
                        }
                        imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        imglb->setFixedSize(imglb_size);
                        imglb->setPixmap(pixmap.scaled(pic_size));
                    });
                    img_downloader->PictureDownload(coverImg);
                    grid_layout->addWidget(imglb, 0, cnt);
                    grid_layout->addWidget(lb, 1, cnt);
                }
            } else if (block["bizCode"] == "PAGE_RECOMMEND_STYLE_PLAYLIST_1") {
                QJsonValue block_resource = block["dslData"]["blockResource"];
                title = block_resource["title"].toString();
                groupbox->setTitle(title);
                int cnt = -1;
                for (auto resource : block_resource["resources"].toArray()) {
                    QJsonValue resourceObj = resource.toObject();
                    resourceType = resourceObj["resourceType"].toString();
                    if (resourceType != "playList" and resourceType != "playlist") {
                        continue;
                    }
                    cnt++;
                    title = resourceObj["title"].toString()
                            + (resourceObj["subTitle"] == QJsonValue::Undefined ? "" : "\n" + resourceObj["subTitle"].toString())
                            + (resourceObj["resourceInteractInfo"]["playCount"] == QJsonValue::Undefined ? "" : "\n(" + resourceObj["resourceInteractInfo"]["playCount"].toString() + ")");
                    QLabel *lb = new QLabel(scroll_area_content);
                    lb->setText(title);
                    lb->setMaximumWidth(imglb_size.width());
                    lb->setWordWrap(true);
                    coverImg = resourceObj["coverImg"].toString();

                    // // qDebug() << title << coverImg;

                    QLabel *imglb = new QLabel(scroll_area_content);
                    imglb->setProperty("type", "Playlist");
                    imglb->setProperty("resourceId", resourceObj["resourceId"].toString().toLongLong());
                    imglb->installEventFilter(imageLabelEventFilter);
                    imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                    Downloader *img_downloader = new Downloader;
                    QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                        QString file_name;
                        QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                        // 匹配输入字符串
                        QRegularExpressionMatch match = regex.match(coverImg);
                        if (match.hasMatch()) {
                            file_name = match.captured(1); // 获取匹配的文件名
                        } else {
                            file_name = "temp.jpg";
                        }
                        file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                        // file_name = checkAndFixImageExtension(file_name);

                        // // qDebug() << "[imglabel]" << file_name;
                        QPixmap pixmap(file_name);
                        if (pixmap.isNull()) {
                            qWarning() << "imglb lixmap fail";
                        }
                        imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        imglb->setFixedSize(imglb_size);
                        imglb->setPixmap(pixmap.scaled(pic_size));
                    });
                    img_downloader->PictureDownload(coverImg);
                    grid_layout->addWidget(imglb, 0, cnt);
                    grid_layout->addWidget(lb, 1, cnt);
                }
            } else if (block["bizCode"] == "PAGE_RECOMMEND_FIRM_PLAYLIST") {
                QJsonValue block_resource = findKeyInJsonDocument(block["dslData"],"home_page_common_playlist_module")["blockResource"];
                if (block_resource == QJsonValue::Undefined) {
                    block_resource = block["dslData"]["blockResource"];
                }
                // assert(block_resource != QJsonValue::Undefined and block_resource != QJsonValue());
                title = block_resource["title"].toString();
                // // qDebug() << title;
                groupbox->setTitle(title);
                int cnt = -1;
                // // qDebug() << "fjlgdjkfghdfs" << block_resource << block_resource.toObject().keys();
                for (auto resource : block_resource["resources"].toArray()) {
                    QJsonValue resourceObj = resource.toObject();
                    resourceType = resourceObj["resourceType"].toString();
                    if (resourceType != "playList" and resourceType != "playlist") {
                        continue;
                    }
                    cnt++;
                    title = resourceObj["title"].toString()
                            + (resourceObj["subTitle"] == QJsonValue::Undefined ? "" : "\n" + resourceObj["subTitle"].toString())
                            + (resourceObj["resourceInteractInfo"]["playCount"] == QJsonValue::Undefined ? "" : "\n(" + resourceObj["resourceInteractInfo"]["playCount"].toString() + ")");
                    QLabel *lb = new QLabel;
                    lb->setText(title);
                    lb->setMaximumWidth(imglb_size.width());
                    lb->setWordWrap(true);
                    coverImg = resourceObj["coverImg"].toString();
                    QLabel *imglb = new QLabel(scroll_area_content);
                    imglb->setProperty("type", "Playlist");
                    imglb->setProperty("resourceId", resourceObj["resourceId"].toString().toLongLong());
                    imglb->installEventFilter(imageLabelEventFilter);
                    imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                    Downloader *img_downloader = new Downloader;
                    QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                        QString file_name;
                        QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                        // 匹配输入字符串
                        QRegularExpressionMatch match = regex.match(coverImg);
                        if (match.hasMatch()) {
                            file_name = match.captured(1); // 获取匹配的文件名
                        } else {
                            file_name = "temp.jpg";
                        }
                        file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                        // file_name = checkAndFixImageExtension(file_name);

                        // // qDebug() << "[imglabel]" << file_name;
                        QPixmap pixmap(file_name);
                        if (pixmap.isNull()) {
                            qWarning() << "imglb lixmap fail";
                        }
                        imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        imglb->setFixedSize(imglb_size);
                        imglb->setPixmap(pixmap.scaled(pic_size));
                        update();
                    });
                    grid_layout->addWidget(imglb, 0, cnt);
                    grid_layout->addWidget(lb, 1, cnt);
                    img_downloader->PictureDownload(coverImg);
                }
            } else if (block["bizCode"] == "PAGE_RECOMMEND_MONTH_YEAR_PLAYLIST") {
                // qDebug() << "PAGE_RECOMMEND_MONTH_YEAR_PLAYLIST";
                QJsonValue block_resource = findKeyInJsonDocument(block["dslData"],"rcmd_annual_and_monthly_playlist_list_module")["blockResource"];
                if (block_resource == QJsonValue::Undefined) {
                    block_resource = block["dslData"]["blockResource"];
                }
                title = block_resource["header"]["title"].toString();
                groupbox->setTitle(title);
                int cnt = -1;
                for (auto resource : block_resource["items"].toArray()) {
                    QJsonValue resourceObj = resource.toObject();
                    resourceType = resourceObj["resourceType"].toString();
                    if (resourceType != "playList" and resourceType != "playlist") {
                        continue;
                    }
                    cnt++;
                    title = resourceObj["title"].toString()
                            + (resourceObj["subTitle"] == QJsonValue::Undefined ? "" : "\n" + resourceObj["subTitle"].toString())
                            + (resourceObj["resourceInteractInfo"]["playCount"] == QJsonValue::Undefined ? "" : "\n(" + resourceObj["resourceInteractInfo"]["playCount"].toString() + ")");
                    QLabel *lb = new QLabel;
                    lb->setText(title);
                    lb->setMaximumWidth(imglb_size.width());
                    lb->setWordWrap(true);
                    coverImg = resourceObj["coverImg"].toString();
                    QLabel *imglb = new QLabel(scroll_area_content);
                    imglb->setProperty("type", "Playlist");
                    imglb->setProperty("resourceId", resourceObj["resourceId"].toString().toLongLong());
                    imglb->installEventFilter(imageLabelEventFilter);
                    imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                    Downloader *img_downloader = new Downloader;
                    QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                        QString file_name;
                        QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                        // 匹配输入字符串
                        QRegularExpressionMatch match = regex.match(coverImg);
                        if (match.hasMatch()) {
                            file_name = match.captured(1); // 获取匹配的文件名
                        } else {
                            file_name = "temp.jpg";
                        }
                        file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                        // file_name = checkAndFixImageExtension(file_name);

                        // // qDebug() << "[imglabel]" << file_name;
                        QPixmap pixmap(file_name);
                        if (pixmap.isNull()) {
                            qWarning() << "imglb lixmap fail";
                        }
                        imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        imglb->setFixedSize(imglb_size);
                        imglb->setPixmap(pixmap.scaled(pic_size));
                        update();
                    });
                    grid_layout->addWidget(imglb, 0, cnt);
                    grid_layout->addWidget(lb, 1, cnt);
                    img_downloader->PictureDownload(coverImg);
                }
            } else if (block["bizCode"] == "PAGE_RECOMMEND_LBS") {
                QJsonValue block_resource = findKeyInJsonDocument(block["dslData"],"home_position_rank_module");
                if (block_resource["blockResourceVO"] != QJsonValue::Undefined) {
                    block_resource = block_resource["blockResourceVO"];
                } else if (block_resource["blockResource"] != QJsonValue::Undefined) {
                    block_resource = block_resource["blockResource"];
                } else {
                    block_resource = block["dslData"]["blockResource"];
                }
                title = block_resource["title"].toString();
                groupbox->setTitle(title);
                int cnt = -1;
                for (auto resource : block_resource["resources"].toArray()) {
                    QJsonValue resourceObj = resource.toObject();
                    resourceType = resourceObj["resourceType"].toString();
                    if (resourceType != "cityStyleCharts") {
                        continue;
                    }
                    cnt++;
                    title = resourceObj["name"].toString()
                            + (resourceObj["subTitle"] == QJsonValue::Undefined ? "" : "\n" + resourceObj["subTitle"].toString())
                            + (resourceObj["playCount"] == QJsonValue::Undefined ? "" : "\n(" + resourceObj["playCount"].toString() + ")");
                    QLabel *lb = new QLabel;
                    lb->setText(title);
                    lb->setMaximumWidth(imglb_size.width());
                    lb->setWordWrap(true);
                    coverImg = resourceObj["coverImg"].toString();
                    QLabel *imglb = new QLabel(scroll_area_content);
                    imglb->setProperty("type", "my_playlist");
                    imglb->setProperty("song_ids", resourceObj["playBtn"]["playAction"]["songIds"].toArray());
                    imglb->installEventFilter(imageLabelEventFilter);
                    imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                    Downloader *img_downloader = new Downloader;
                    QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                        QString file_name;
                        QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                        // 匹配输入字符串
                        QRegularExpressionMatch match = regex.match(coverImg);
                        if (match.hasMatch()) {
                            file_name = match.captured(1); // 获取匹配的文件名
                        } else {
                            file_name = "temp.jpg";
                        }
                        file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                        QPixmap pixmap(file_name);
                        if (pixmap.isNull()) {
                            qWarning() << "imglb lixmap fail";
                        }
                        imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        imglb->setFixedSize(imglb_size);
                        imglb->setPixmap(pixmap.scaled(pic_size));
                        update();
                    });
                    grid_layout->addWidget(imglb, 0, cnt);
                    grid_layout->addWidget(lb, 1, cnt);
                    img_downloader->PictureDownload(coverImg);
                }
            } else if (block["bizCode"] == "PAGE_RECOMMEND_MY_SHEET") {
                QJsonValue block_resource = block["dslData"]["blockResource"];
                title = block_resource["title"].toString();
                groupbox->setTitle(title);
                int cnt = -1;
                for (auto resource : block_resource["resources"].toArray()) {
                    QJsonValue resourceObj = resource.toObject();
                    resourceType = resourceObj["resourceType"].toString();
                    if (resourceType == "more") break;
                    if (resourceType != "playList" and resourceType != "playlist") {
                        continue;
                    }
                    cnt++;
                    title = resourceObj["title"].toString()
                            + (resourceObj["subTitle"] == QJsonValue::Undefined ? "" : "\n" + resourceObj["subTitle"].toString())
                            + (resourceObj["resourceInteractInfo"]["playCount"] == QJsonValue::Undefined ? "" : "\n(" + resourceObj["resourceInteractInfo"]["playCount"].toString() + ")");
                    QLabel *lb = new QLabel(scroll_area_content);
                    lb->setText(title);
                    lb->setMaximumWidth(imglb_size.width());
                    lb->setWordWrap(true);
                    coverImg = resourceObj["coverImg"].toString();
                    QLabel *imglb = new QLabel(scroll_area_content);
                    imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                    imglb->setProperty("type", "Playlist");
                    imglb->setProperty("resourceId", resourceObj["resourceId"].toString().toLongLong());
                    imglb->installEventFilter(imageLabelEventFilter);
                    Downloader *img_downloader = new Downloader;
                    QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                        QString file_name;
                        QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                        // 匹配输入字符串
                        QRegularExpressionMatch match = regex.match(coverImg);
                        if (match.hasMatch()) {
                            file_name = match.captured(1); // 获取匹配的文件名
                        } else {
                            file_name = "temp.jpg";
                        }
                        file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                        // file_name = checkAndFixImageExtension(file_name);

                        // // qDebug() << "[imglabel]" << file_name;
                        QPixmap pixmap(file_name);
                        if (pixmap.isNull()) {
                            qWarning() << "imglb lixmap fail";
                        }
                        imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        imglb->setFixedSize(imglb_size);
                        imglb->setPixmap(pixmap.scaled(pic_size));
                    });
                    img_downloader->PictureDownload(coverImg);
                    grid_layout->addWidget(imglb, 0, cnt);
                    grid_layout->addWidget(lb, 1, cnt);
                }
            } else if (block["bizCode"] == "PAGE_RECOMMEND_PRIVATE_RCMD_SONG") {
                QJsonValue block_resource = findKeyInJsonDocument(block["dslData"],"home_common_rcmd_songs_module");
                if (block_resource == QJsonValue()) {
                    block_resource = block["dslData"];
                }
                title = block_resource["header"]["title"].toString();
                groupbox->setTitle(title);
                if (block_resource["content"] != QJsonValue::Undefined) {
                    block_resource = block_resource["content"];
                } else if(block_resource["blockResource"] != QJsonValue::Undefined) {
                    block_resource = block_resource["blockResource"];
                }
                int cnt = -1;
                for (auto resource1 : block_resource["items"].toArray()) {
                    QJsonValue resourceObj1 = resource1.toObject();
                    for (auto resource : resourceObj1["items"].toArray()) {
                        QJsonValue resourceObj = resource.toObject();
                        resourceType = resourceObj["resourceType"].toString();
                        if (resourceType != "song") {
                            continue;
                        }
                        cnt++;
                        title = resourceObj["title"].toString()
                                + (resourceObj["subTitle"] == QJsonValue::Undefined ? "" : "\n" + resourceObj["subTitle"].toString())
                                + (resourceObj["recReason"] == QJsonValue::Undefined ? "" : "\n(" + resourceObj["recReason"].toString() + ")");
                        QLabel *lb = new QLabel;
                        lb->setText(title);
                        lb->setMaximumWidth(imglb_size.width());
                        lb->setWordWrap(true);
                        coverImg = resourceObj["coverUrl"].toString();
                        QLabel *imglb = new QLabel(scroll_area_content);
                        imglb->setProperty("type", "my_playlist");
                        imglb->setProperty("song_ids", resourceObj["playBtn"]["playAction"]["songIds"].toArray());
                        imglb->installEventFilter(imageLabelEventFilter);
                        imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                        Downloader *img_downloader = new Downloader;
                        QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                            QString file_name;
                            QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                            // 匹配输入字符串
                            QRegularExpressionMatch match = regex.match(coverImg);
                            if (match.hasMatch()) {
                                file_name = match.captured(1); // 获取匹配的文件名
                            } else {
                                file_name = "temp.jpg";
                            }
                            file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                            QPixmap pixmap(file_name);
                            if (pixmap.isNull()) {
                                qWarning() << "imglb lixmap fail";
                            }
                            imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                            imglb->setFixedSize(imglb_size);
                            imglb->setPixmap(pixmap.scaled(pic_size));
                            update();
                        });
                        grid_layout->addWidget(imglb, 0, cnt);
                        grid_layout->addWidget(lb, 1, cnt);
                        img_downloader->PictureDownload(coverImg);
                    }
                }
            } else if (block["bizCode"] == "PAGE_RECOMMEND_RED_SIMILAR_SONG") {
                QJsonValue block_resource = findKeyInJsonDocument(block["dslData"],"home_common_rcmd_songs_module");
                if (block_resource == QJsonValue()) {
                    block_resource = block["dslData"];
                }
                title = block_resource["header"]["title"].toString();
                groupbox->setTitle(title);
                if (block_resource["content"] != QJsonValue::Undefined) {
                    block_resource = block_resource["content"];
                } else if(block_resource["blockResource"] != QJsonValue::Undefined) {
                    block_resource = block_resource["blockResource"];
                }
                int cnt = -1;
                for (auto resource1 : block_resource["items"].toArray()) {
                    QJsonValue resourceObj1 = resource1.toObject();
                    for (auto resource : resourceObj1["items"].toArray()) {
                        QJsonValue resourceObj = resource.toObject();
                        resourceType = resourceObj["resourceType"].toString();
                        if (resourceType != "song") {
                            continue;
                        }
                        cnt++;
                        title = resourceObj["title"].toString()
                                + (resourceObj["subTitle"] == QJsonValue::Undefined ? "" : "\n" + resourceObj["subTitle"].toString())
                                + (resourceObj["artistName"] == QJsonValue::Undefined ? "" : "\n(" + resourceObj["artistName"].toString() + ")");
                        QLabel *lb = new QLabel;
                        lb->setText(title);
                        lb->setMaximumWidth(imglb_size.width());
                        lb->setWordWrap(true);
                        coverImg = resourceObj["coverUrl"].toString();
                        QLabel *imglb = new QLabel(scroll_area_content);
                        imglb->setProperty("type", "my_playlist");
                        imglb->setProperty("song_ids", resourceObj["playBtn"]["playAction"]["songIds"].toArray());
                        imglb->installEventFilter(imageLabelEventFilter);
                        imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                        Downloader *img_downloader = new Downloader;
                        QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                            QString file_name;
                            QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                            // 匹配输入字符串
                            QRegularExpressionMatch match = regex.match(coverImg);
                            if (match.hasMatch()) {
                                file_name = match.captured(1); // 获取匹配的文件名
                            } else {
                                file_name = "temp.jpg";
                            }
                            file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                            QPixmap pixmap(file_name);
                            if (pixmap.isNull()) {
                                qWarning() << "imglb lixmap fail";
                            }
                            imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                            imglb->setFixedSize(imglb_size);
                            imglb->setPixmap(pixmap.scaled(pic_size));
                            update();
                        });
                        grid_layout->addWidget(imglb, 0, cnt);
                        grid_layout->addWidget(lb, 1, cnt);
                        img_downloader->PictureDownload(coverImg);
                    }
                }
            } else if (block["bizCode"] == "PAGE_RECOMMEND_SCENE_PLAYLIST_LOCATION") {
                QJsonValue block_resource = findKeyInJsonDocument(block["dslData"],"home_page_scene_playlist_module")["blockResource"];
                if (block_resource == QJsonValue::Undefined) {
                    block_resource = block["dslData"]["blockResource"];
                }
                title = block_resource["title"].toString();
                groupbox->setTitle(title);
                int cnt = -1;
                static bool isFirst;
                isFirst = true;
                for (auto resource : block_resource["resources"].toArray()) {
                    QJsonValue resourceObj = resource.toObject();
                    resourceType = resourceObj["resourceType"].toString();
                    if (resourceType != "playList" and resourceType != "playlist") {
                        continue;
                    }
                    cnt++;
                    title = resourceObj["title"].toString()
                            + (resourceObj["subTitle"] == QJsonValue::Undefined ? "" : "\n" + resourceObj["subTitle"].toString())
                            + (resourceObj["resourceInteractInfo"]["playCount"] == QJsonValue::Undefined ? "" : "\n(" + resourceObj["resourceInteractInfo"]["playCount"].toString() + ")");
                    QLabel *lb = new QLabel(scroll_area_content);
                    lb->setText(title);
                    lb->setMaximumWidth(imglb_size.width());
                    lb->setWordWrap(true);
                    coverImg = resourceObj["coverImg"].toString();
                    QLabel *imglb = new QLabel(scroll_area_content);
                    imglb->setProperty("type", "Playlist");
                    imglb->setProperty("resourceId", resourceObj["resourceId"].toString().toLongLong());
                    imglb->installEventFilter(imageLabelEventFilter);
                    imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                    // // qDebug() << "igmlb type resid coverimg" << imglb->property("type") << imglb->property("resourceId") << coverImg;
                    Downloader *img_downloader = new Downloader;
                    QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                        QString file_name;
                        QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                        // 匹配输入字符串
                        QRegularExpressionMatch match = regex.match(coverImg);
                        if (match.hasMatch()) {
                            file_name = match.captured(1); // 获取匹配的文件名
                        } else {
                            file_name = "temp.jpg";
                        }
                        file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                        // file_name = checkAndFixImageExtension(file_name);

                        // // qDebug() << "[imglabel]" << file_name;
                        QPixmap pixmap(file_name);
                        if (pixmap.isNull()) {
                            qWarning() << "imglb lixmap fail";
                        }
                        imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        imglb->setFixedSize(imglb_size);
                        imglb->setPixmap(pixmap.scaled(pic_size));
                    });
                    img_downloader->PictureDownload(coverImg);
                    grid_layout->addWidget(imglb, 0, cnt);
                    grid_layout->addWidget(lb, 1, cnt);
                }
            } else if (block["bizCode"] == "PAGE_RECOMMEND_NEW_SONG_AND_ALBUM") {
                QJsonValue block_resource = findKeyInJsonDocument(block["dslData"],"home_page_common_playlist_module")["blockResource"];
                if (block_resource == QJsonValue::Undefined) {
                    block_resource = block["dslData"]["blockResource"];
                }
                title = block_resource["title"].toString();
                groupbox->setTitle(title);
                int cnt = -1;
                static bool isFirst;
                isFirst = true;
                for (auto resource : block_resource["resources"].toArray()) {
                    QJsonValue resourceObj = resource.toObject();
                    resourceType = resourceObj["resourceType"].toString();
                    if (resourceType != "playList" and resourceType != "playlist") {
                        continue;
                    }
                    cnt++;
                    title = resourceObj["title"].toString()
                            + (resourceObj["subTitle"] == QJsonValue::Undefined ? "" : "\n" + resourceObj["subTitle"].toString())
                            + (resourceObj["resourceInteractInfo"]["playCount"] == QJsonValue::Undefined ? "" : "\n(" + resourceObj["resourceInteractInfo"]["playCount"].toString() + ")");
                    QLabel *lb = new QLabel(scroll_area_content);
                    lb->setText(title);
                    lb->setMaximumWidth(imglb_size.width());
                    lb->setWordWrap(true);
                    coverImg = resourceObj["coverImg"].toString();
                    QLabel *imglb = new QLabel(scroll_area_content);
                    imglb->setProperty("type", "Playlist");
                    imglb->setProperty("resourceId", resourceObj["resourceId"].toString().toLongLong());
                    imglb->installEventFilter(imageLabelEventFilter);
                    imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                    Downloader *img_downloader = new Downloader;
                    QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                        QString file_name;
                        QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                        // 匹配输入字符串
                        QRegularExpressionMatch match = regex.match(coverImg);
                        if (match.hasMatch()) {
                            file_name = match.captured(1); // 获取匹配的文件名
                        } else {
                            file_name = "temp.jpg";
                        }
                        file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                        QPixmap pixmap(file_name);
                        if (pixmap.isNull()) {
                            qWarning() << "imglb lixmap fail";
                        }
                        imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        imglb->setFixedSize(imglb_size);
                        imglb->setPixmap(pixmap.scaled(pic_size));
                    });
                    img_downloader->PictureDownload(coverImg);
                    grid_layout->addWidget(imglb, 0, cnt);
                    grid_layout->addWidget(lb, 1, cnt);
                }
            } else if (block["bizCode"] == "PAGE_RECOMMEND_ARTIST_TREND") {
                QJsonValue block_resource = findKeyInJsonDocument(block["dslData"],"artist_new_trends_list");
                if (block_resource == QJsonValue()) {
                    block_resource = block["dslData"];
                }
                QJsonValue block_resource_4_title = findKeyInJsonDocument(block["dslData"],"home_artist_new_trends_title");
                if (block_resource == QJsonValue()) {
                    title = "艺人的最新动向";
                } else {
                    title = block_resource_4_title["blockTitle"].toString();
                }
                groupbox->setTitle(title);
                int cnt = -1;
                for (auto resource : block_resource["items"].toArray()) {
                    QJsonValue resourceObj = resource.toObject();
                    resourceType = resourceObj["resourceType"].toString();
                    if (resourceType == "song") {
                        cnt++;
                        title = resourceObj["title"].toString()
                                + (resourceObj["subTitle"] == QJsonValue::Undefined or resourceObj["subTitle"].toString().isEmpty() ? "" : "\n" + resourceObj["subTitle"].toString())
                                + (resourceObj["description"] == QJsonValue::Undefined or resourceObj["description"].toString().isEmpty() ? "" : "\n" + resourceObj["description"].toString() + "");
                        QLabel *lb = new QLabel;
                        lb->setText(title);
                        lb->setMaximumWidth(imglb_size.width());
                        lb->setWordWrap(true);
                        coverImg = resourceObj["coverUrl"].toString();
                        QLabel *imglb = new QLabel(scroll_area_content);
                        imglb->setProperty("type", "song");
                        imglb->setProperty("id", resourceObj["resourceId"].toString().toLongLong());
                        // qDebug() << resourceObj["resourceId"].toInteger();
                        imglb->installEventFilter(imageLabelEventFilter);
                        imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                        Downloader *img_downloader = new Downloader;
                        QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                            QString file_name;
                            QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                            // 匹配输入字符串
                            QRegularExpressionMatch match = regex.match(coverImg);
                            if (match.hasMatch()) {
                                file_name = match.captured(1); // 获取匹配的文件名
                            } else {
                                file_name = "temp.jpg";
                            }
                            file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                            QPixmap pixmap(file_name);
                            if (pixmap.isNull()) {
                                qWarning() << "imglb lixmap fail";
                            }
                            imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                            imglb->setFixedSize(imglb_size);
                            imglb->setPixmap(pixmap.scaled(pic_size));
                            update();
                        });
                        grid_layout->addWidget(imglb, 0, cnt);
                        grid_layout->addWidget(lb, 1, cnt);
                        img_downloader->PictureDownload(coverImg);
                    } else if (resourceType == "album") {
                        cnt++;
                        title = resourceObj["title"].toString()
                                + (resourceObj["subTitle"] == QJsonValue::Undefined or resourceObj["subTitle"].toString().isEmpty() ? "" : "\n" + resourceObj["subTitle"].toString())
                                + (resourceObj["description"] == QJsonValue::Undefined or resourceObj["description"].toString().isEmpty() ? "" : "\n" + resourceObj["description"].toString() + "");
                        QLabel *lb = new QLabel;
                        lb->setText(title);
                        lb->setMaximumWidth(imglb_size.width());
                        lb->setWordWrap(true);
                        coverImg = resourceObj["coverUrl"].toString();
                        QLabel *imglb = new QLabel(scroll_area_content);
                        imglb->setProperty("type", "album");
                        imglb->setProperty("id", resourceObj["resourceId"].toString().toLongLong());
                        imglb->installEventFilter(imageLabelEventFilter);
                        imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                        Downloader *img_downloader = new Downloader;
                        QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                            QString file_name;
                            QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                            // 匹配输入字符串
                            QRegularExpressionMatch match = regex.match(coverImg);
                            if (match.hasMatch()) {
                                file_name = match.captured(1); // 获取匹配的文件名
                            } else {
                                file_name = "temp.jpg";
                            }
                            file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                            QPixmap pixmap(file_name);
                            if (pixmap.isNull()) {
                                qWarning() << "imglb lixmap fail";
                            }
                            imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                            imglb->setFixedSize(imglb_size);
                            imglb->setPixmap(pixmap.scaled(pic_size));
                            update();
                        });
                        grid_layout->addWidget(imglb, 0, cnt);
                        grid_layout->addWidget(lb, 1, cnt);
                        img_downloader->PictureDownload(coverImg);
                    } else {
                        continue;
                    }
                }
            } else if (block["bizCode"] == "PAGE_RECOMMEND_SPECIAL_ORIGIN_SONG_LOCATION") {
                QJsonValue block_resource = findKeyInJsonDocument(block["dslData"],"home_page_scene_playlist_module")["blockResource"];
                if (block_resource == QJsonValue::Undefined) {
                    block_resource = block["dslData"]["blockResource"];
                }
                title = block_resource["title"].toString();
                groupbox->setTitle(title);
                int cnt = -1;
                static bool isFirst;
                isFirst = true;
                for (auto resource : block_resource["resources"].toArray()) {
                    QJsonValue resourceObj = resource.toObject();
                    resourceType = resourceObj["resourceType"].toString();
                    if (resourceType != "playList" and resourceType != "playlist") {
                        continue;
                    }
                    cnt++;
                    title = resourceObj["title"].toString()
                            + (resourceObj["subTitle"] == QJsonValue::Undefined ? "" : "\n" + resourceObj["subTitle"].toString())
                            + (resourceObj["resourceInteractInfo"]["playCount"] == QJsonValue::Undefined ? "" : "\n(" + resourceObj["resourceInteractInfo"]["playCount"].toString() + ")");
                    QLabel *lb = new QLabel(scroll_area_content);
                    lb->setText(title);
                    lb->setMaximumWidth(imglb_size.width());
                    lb->setWordWrap(true);
                    coverImg = resourceObj["coverImg"].toString();
                    QLabel *imglb = new QLabel(scroll_area_content);
                    imglb->setProperty("type", "Playlist");
                    imglb->setProperty("resourceId", resourceObj["resourceId"].toString().toLongLong());
                    imglb->installEventFilter(imageLabelEventFilter);
                    imglb->setPixmap(QPixmap(":/default_cover.jpg").scaled(pic_size));
                    Downloader *img_downloader = new Downloader;
                    QObject::connect(img_downloader, &Downloader::DownloadFinished, this, [=, this]() {
                        QString file_name;
                        QRegularExpression regex(R"(([^/]+\.(?:jpg|png|jpeg)))");
                        // 匹配输入字符串
                        QRegularExpressionMatch match = regex.match(coverImg);
                        if (match.hasMatch()) {
                            file_name = match.captured(1); // 获取匹配的文件名
                        } else {
                            file_name = "temp.jpg";
                        }
                        file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
                        QPixmap pixmap(file_name);
                        if (pixmap.isNull()) {
                            qWarning() << "imglb lixmap fail";
                        }
                        imglb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        imglb->setFixedSize(imglb_size);
                        imglb->setPixmap(pixmap.scaled(pic_size));
                    });
                    img_downloader->PictureDownload(coverImg);
                    grid_layout->addWidget(imglb, 0, cnt);
                    grid_layout->addWidget(lb, 1, cnt);
                }
            }
            scroll_area_content->setLayout(grid_layout);
            scroll_area->setWidget(scroll_area_content);
            scroll_area->setFixedHeight(pic_size.height() + 100);
            scroll_area->setWidgetResizable(true);
            scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            groupbox_layout->addWidget(scroll_area);
            global_vbox->addWidget(groupbox);
        }

        root["blockCodeOrderList"] = json["data"]["blockCodeOrderList"] == QJsonValue::Undefined ? "[]" : json["data"]["blockCodeOrderList"].toString();
    } while (root["blockCodeOrderList"] != "[]");

    global_scroll_area_content->setLayout(global_vbox);
    global_scroll_area->setWidget(global_scroll_area_content);
    global_scroll_area->setWidgetResizable(true);
    global_layout->addWidget(global_scroll_area);
    homepageWidget->setLayout(global_layout);
    Refreshed();
}

void MainWindow::SetUserInfo(qint64 user_id, QString user_nick_name) {
    this->user_id = user_id;
    this->user_nick_name = user_nick_name;
    this->favourite_id = user.GetFavouriteId(requester, user_id);
    if (user_nick_name.isEmpty()) {
        isGuest = true;
    } else {
        isGuest = false;
    }
    if (isGuest) {
        this->user_nick_name = user_nick_name = "Guest";
    }
    setWindowTitle(QString("Tiny Netease Music(已登录: %1)").arg(user_nick_name));
    SaveSettings();
    // qDebug() << QString("[SetSongInfo] user_id: %1; user_nick_name: %2").arg(user_id).arg(user_nick_name);
}

void MainWindow::OnCellClicked(const QModelIndex &index) {
    QStandardItemModel *model = nullptr;

    if (currentModel == kSongModel) {
        model = song_model;

        if (index.column() == 10) {
            // 喜欢
            QModelIndex idx = model->index(index.row(), 0);
            qint64 id = model->data(idx).toLongLong();
            if (id == 0) {
                return;
            }
            bool is_favourite = model->data(index).toBool();
            bool is_ok = user.SongLike(requester,id,!is_favourite);
            if(is_ok) {
                is_favourite = !is_favourite;
                model->setData(index, is_favourite);
            }
        } else if (index.column() == 9) {
            // 下载
            QModelIndex idx = model->index(index.row(), 0);
            qint64 id = model->data(idx).toLongLong();
            if (id == 0) {
                return;
            }
            idx = model->index(index.row(), 3);
            QString name =model->data(idx).toString();

            idx = model->index(index.row(), 5);
            QString artistsStr =model->data(idx).toString();

            emit DownloadButtonClicked(index.row(), id, artistsStr, name, DOWNLOAD_DIR.absolutePath(), encodeType, quality);
        } else if (index.column() == 11) {
        }
    } else if (currentModel == kArtistModel) {
        model = artist_model;

        if (index.column() == 2) {
            // 喜欢
            QModelIndex idx = model->index(index.row(), 0);
            qint64 id = model->data(idx).toLongLong();
            if (id == 0) {
                return;
            }
            idx = model->index(index.row(), 2);
            bool is_followed = model->data(idx).toBool();
            bool is_ok;
            if (is_followed) {
                is_ok = user.DelfollowArtist(requester, id);
                if (is_ok) {
                    is_followed = false;
                }
            } else {
                is_ok = user.FollowArtist(requester, id);
                if (is_ok) {
                    is_followed = true;
                }
            }
            idx = model->index(index.row(), 2);
            model->setData(idx, is_followed);
        }
    } else if (currentModel == kUserModel) {
        model = user_model;

        if (index.column() == 4) {
            // 喜欢
            QModelIndex idx = model->index(index.row(), 0);
            qint64 id = model->data(idx).toLongLong();
            if (id == 0) {
                return;
            }
            idx = model->index(index.row(), 4);
            bool is_followed = model->data(idx).toBool();
            bool is_ok;
            if (is_followed) {
                is_ok = user.DelfollowUser(requester, id);
                if (is_ok) {
                    is_followed = false;
                }
            } else {
                is_ok = user.FollowUser(requester, id);
                if (is_ok) {
                    is_followed = true;
                }
            }
            idx = model->index(index.row(), 4);
            model->setData(idx, is_followed);
        }

    } else if (currentModel == kPlaylistModel) {
        model = playlist_model;

        if (index.column() == 5) {
            // 喜欢
            QModelIndex idx = model->index(index.row(), 0);
            qint64 id = model->data(idx).toLongLong();
            if (id == 0) {
                return;
            }
            idx = model->index(index.row(), 5);
            int is_followed = model->data(idx).toInt();
            bool is_ok;
            if (is_followed == 1) {
                is_ok = playlist.UnsubscribePlaylist(requester, id);
                if (is_ok) {
                    is_followed = 0;
                }
            } else if (is_followed == 0) {
                is_ok = playlist.SubscribePlaylist(requester, id);
                if (is_ok) {
                    is_followed = 1;
                }
            }
            idx = model->index(index.row(), 5);
            model->setData(idx, is_followed);
        }
    }
}

void MainWindow::OnCellDoubleClicked(const QModelIndex &index) {
    QStandardItemModel *model = static_cast<QStandardItemModel*>(ui->tableView->model());

    if (currentModel == kSongModel) {
        // model = song_model;

        if (index.column() == 3) {
            // 歌名列
            QModelIndex idx = model->index(index.row(), 0);
            qint64 song_id = model->data(idx).toLongLong();

            if (song_id == 0) {
                return;
            }

            idx = model->index(index.row(), 3);
            QString song_name = model->data(idx).toString();
            idx = model->index(index.row(), 5);
            QString artists_str = model->data(idx).toString();
            Downloader *singleDownloader = new Downloader(this);

            QObject::connect(singleDownloader, &Downloader::DownloadPortionUpdated, this, [this](int rowNum, qint64 songId, double portion, QString type) {
                if (song_model->rowCount() <= rowNum) {
                    return;
                }
                QModelIndex idx = song_model->index(rowNum, 0);
                qint64 modelSongId = song_model->data(idx).toLongLong();
                // qDebug() << "DownloadPortionUpdated" << idx << songId << modelSongId;
                if (modelSongId != songId) {
                    return;
                }
                QPair<double, QString> info{portion, type};
                idx = song_model->index(rowNum, 9);
                song_model->setData(idx, QVariant::fromValue(info));
                ui->tableView->update(idx);
            });
            idx = model->index(index.row(), 9);
            auto [portion, type] = model->data(idx).value<QPair<double, QString>>();
            if (portion == 1.0) {
                singleDownloader->setProperty("type", "ButtonDownload");
            } else {
                singleDownloader->setProperty("type", "CacheDownload");
            }
            singleDownloader->setProperty("songModelRowNum", index.row());
            singleDownloader->setProperty("songId", song_id);
            QObject::connect(singleDownloader, &Downloader::DownloadMessageGenerated, ui->statusbar, &QStatusBar::showMessage);
            QObject::connect(singleDownloader, &Downloader::DownloadFinished, this, [](QString path) {
                QUrl url = QUrl::fromLocalFile(path);
                QDesktopServices::openUrl(url);
                // qDebug() << "open cache music" << path;
            }, Qt::SingleShotConnection);
            // QObject::connect(singleDownloader, &Downloader::DownloadFinished, singleDownloader, &Downloader::deleteLater, Qt::SingleShotConnection);
            singleDownloader->SongDownload(song_id, artists_str, song_name);
        } else if (index.column() == 5) {
            // 歌手列
            QModelIndex idx = model->index(index.row(), 2);
            qint64 first_artist_id = model->data(idx).toList()[0].toLongLong();

            if (first_artist_id == 0) {
                return;
            }

            undoStack.push(Command{"Refresh", "ArtistDetail", {first_artist_id}});
            RefreshArtistDetail(first_artist_id);
        } else if (index.column() == 6) {
            QModelIndex idx = model->index(index.row(), 1);
            qint64 album_id = model->data(idx).toLongLong();

            if (album_id == 0) {
                return;
            }

            undoStack.push({"Refresh", "AlbumDetail", {album_id}});
            RefreshAlbumDetail(album_id);
        } else if (index.column() == 11) {
            // 评论
            // QModelIndex idx = model->index(index.row(), 0);
            // 评论列
            QModelIndex idx = model->index(index.row(), 0);
            qint64 song_id = model->data(idx).toLongLong();
            idx = model->index(index.row(), 3);
            QString song_name = model->data(idx).toString();
            idx = model->index(index.row(), 5);
            QString song_artists_str = model->data(idx).toString();
            if (song_id == 0) {
                return;
            }
            // undoStack.push(Command{"Refresh", "Comment", {song_id}});
            RefreshComment(song_id, QString("%1 - %2").arg(song_artists_str, song_name));
        }
    } else if (currentModel == kArtistModel) {
        // model = artist_model;

        if (index.column() == 1) {
            QModelIndex idx = model->index(index.row(), 1);
            QString name = model->data(idx).toString();
            idx = model->index(index.row(), 0);
            qint64 id = model->data(idx).toLongLong();

            if (id == 0) {
                return;
            }

            offset = 0;
            undoStack.push(Command{"Refresh", "ArtistDetail", {id}});
            RefreshArtistDetail(id);
        }
    } else if (currentModel == kAlbumModel) {
        if (index.column() == 1) {
            QModelIndex idx = model->index(index.row(), 1);
            QString name = model->data(idx).toString();
            idx = model->index(index.row(), 0);
            qint64 id = model->data(idx).toLongLong();

            if (id == 0) {
                return;
            }

            offset = 0;
            undoStack.push({"Refresh", "AlbumDetail", {id}});
            RefreshAlbumDetail(id);
        } else if (index.column() == 3) {
            QModelIndex idx = model->index(index.row(), 3);
            QString name = model->data(idx).toString();
            idx = model->index(index.row(), 2);
            qint64 id = model->data(idx).toLongLong();

            if (id == 0) {
                return;
            }

            offset = 0;
            undoStack.push(Command{"Refresh", "ArtistDetail", {id}});
            RefreshArtistDetail(id);
        }
    } else if (currentModel == kUserModel) {
        if (index.column() == 1) {
            QModelIndex idx = model->index(index.row(), 1);
            QString name = model->data(idx).toString();

            idx = model->index(index.row(), 0);
            qint64 id = model->data(idx).toLongLong();

            if (id == 0) {
                return;
            }

            offset = 0;
            undoStack.push(Command{"Refresh", "UserDetail", {id}});
            RefreshUserDetail(id);
        }
    } else if (currentModel == kPlaylistModel) {
        if (index.column() == 1) {
            QModelIndex idx = model->index(index.row(), 1);
            QString name = model->data(idx).toString();
            idx = model->index(index.row(), 0);
            qint64 id = model->data(idx).toLongLong();

            if (id == 0) {
                return;
            }

            offset = 0;
            undoStack.push(Command{"Refresh", "PlaylistDetail", {id}});
            RefreshPlaylistDetail(id);
        } else if (index.column() == 2) {
            QModelIndex idx = model->index(index.row(), 3);
            qint64 creatorId = model->data(idx).toLongLong();
            qint64 prevCreatorId = undoStack.top().args.isEmpty() ? 0 : undoStack.top().args[0].toLongLong();

            if (creatorId == 0 or creatorId == prevCreatorId) {
                return;
            }

            offset = 0;
            undoStack.push(Command{"Refresh", "UserDetail", {creatorId}});
            RefreshUserDetail(creatorId);
        }
    } else if (currentModel == kRcmdPlaylistModel) {
        if (index.column() == 1) {
            QModelIndex idx = model->index(index.row(), 1);
            QString name = model->data(idx).toString();
            idx = model->index(index.row(), 0);
            qint64 id = model->data(idx).toLongLong();

            if (id == 0) {
                return;
            }

            offset = 0;
            undoStack.push(Command{"Refresh", "PlaylistDetail", {id}});
            RefreshPlaylistDetail(id);
        } else if (index.column() == 2) {
            QModelIndex idx = model->index(index.row(), 3);
            qint64 creatorId = model->data(idx).toLongLong();

            if (creatorId == 0) {
                return;
            }

            offset = 0;
            undoStack.push(Command{"Refresh", "UserDetail", {creatorId}});
            RefreshUserDetail(creatorId);
        }
    } else if (currentModel == kFollowedModel) {
        if (index.column() == 2) {
            QModelIndex idx = model->index(index.row(), 0);
            int type = model->data(idx).toInt();
            idx = model->index(index.row(), 1);
            qint64 id = model->data(idx).toLongLong();

            if (id == 0) {
                return;
            }

            if (type == 1) {
                offset = 0;
                undoStack.push(Command{"Refresh", "UserDetail", {id}});
                RefreshUserDetail(id);
            } else if (type == 2 or type == 3){
                offset = 0;
                undoStack.push(Command{"Refresh", "ArtistDetail", {id}});
                RefreshArtistDetail(id);
            }
        }
    }
}

void MainWindow::CreateContextMenu(const QPoint &pos) {
    QModelIndex click_idx = ui->tableView->indexAt(pos);
    QAbstractItemModel *model = nullptr;

    if (currentModel == kSongModel) {
        model = song_model;

        if (click_idx.column() == 3) {
            // 歌名列
            QMenu context_menu(ui->tableView);

            QAction *download_action = new QAction("下载", &context_menu);
            QObject::connect(download_action, &QAction::triggered, this, [&](bool checked) {
                QModelIndex idx = ui->tableView->indexAt(pos);
                int row = idx.row();
                QModelIndex idx_ = model->index(row, 0);
                qint64 song_id = model->data(idx_).toLongLong();

                if (song_id == 0) {
                    return;
                }

                idx_ = model->index(row, 3);
                QString song_name = model->data(idx_).toString();
                idx_ = model->index(row, 5);
                QString artists_str = model->data(idx_).toString();

                Downloader *singleDownloader = new Downloader(this);
                QObject::connect(singleDownloader, &Downloader::DownloadPortionUpdated, this, [this](int rowNum, qint64 songId, double portion, QString type) {
                    if (song_model->rowCount() <= rowNum) {
                        return;
                    }
                    QModelIndex idx = song_model->index(rowNum, 0);
                    qint64 modelSongId = song_model->data(idx).toLongLong();
                    // qDebug() << "DownloadPortionUpdated" << idx << songId << modelSongId;
                    if (modelSongId != songId) {
                        return;
                    }
                    QPair<double, QString> info{portion, type};
                    idx = song_model->index(rowNum, 9);
                    song_model->setData(idx, QVariant::fromValue(info));
                    ui->tableView->update(idx);
                });
                singleDownloader->setProperty("songModelRowNum", row);
                singleDownloader->setProperty("songId", song_id);
                singleDownloader->setProperty("type", "MenuDownload");
                singleDownloader->SongDownload(song_id, artists_str, song_name, DOWNLOAD_DIR.absolutePath());
            });
            context_menu.addAction(download_action);

            QString like_text;
            QModelIndex idx = ui->tableView->indexAt(pos);
            int row = idx.row();
            QModelIndex idx_ = model->index(row, 10);
            bool is_liked = model->data(idx_).toBool();
            if (is_liked) {
                like_text = "从“我喜欢”中移除";
            } else {
                like_text = "添加到“我喜欢”";
            }
            QAction *like_action = new QAction(like_text, &context_menu);
            QObject::connect(like_action, &QAction::triggered, this, [&](bool checked) {
                QModelIndex idx = ui->tableView->indexAt(pos);
                int row = idx.row();
                QModelIndex idx_ = model->index(row, 0);
                qint64 song_id = model->data(idx_).toLongLong();

                if (song_id == 0) {
                    return;
                }

                idx_ = model->index(row, 10);
                bool is_liked = model->data(idx_).toBool();

                bool is_ok = user.SongLike(requester, song_id, !is_liked);
                if(is_ok) {
                    is_liked = !is_liked;
                    model->setData(idx_, is_liked);
                }
            });
            context_menu.addAction(like_action);

            idx_ = model->index(row, 0);
            qint64 songId = model->data(idx_).toLongLong();

            if (songId == 0) {
                return;
            }

            QAction *add_to_playlist_action = new QAction("收藏/取消收藏到歌单", &context_menu);
            QObject::connect(add_to_playlist_action, &QAction::triggered, this, [&](bool checked) {
                AddToPlaylistDialog dialog(this);
                dialog.showMyPlaylist(requester, user_id, songId);
                if (dialog.exec() == QDialog::Accepted) {
                    qint64 playlistId = dialog.getTargetPlaylistId();
                    if (playlistId == 0) {
                        // 新建歌单
                        QString newPlaylistName = dialog.getNewPlaylistname();
                        bool isPrivate = dialog.isPrivatePlaylist();
                        QJsonDocument json = playlist.CreatePlaylist(requester, newPlaylistName, (isPrivate ? 10 : 0));
                        if (json["code"] != 200) {
                            QMessageBox::warning(this, "", QString("创建歌单失败！\n%1\n%2").arg(json["code"].toInt()).arg(json["message"].toString()));
                            return;
                        }
                        playlistId = json["id"].toInteger();
                    }
                    QJsonDocument json = playlist.AddTrackToPlaylist(requester, playlistId, songId);
                    if (json["code"] != 200) {
                        if (json["code"] == 502 and json["message"] == "歌单内歌曲重复") {
                            QMessageBox::StandardButton choice = QMessageBox::information(this, "", "歌单内歌曲重复！\n是否删除？", QMessageBox::Yes | QMessageBox::No);
                            if (choice == QMessageBox::Yes) {
                                json = playlist.DeleteTrackFromPlaylist(requester, playlistId, songId);
                                if (json["code"] != 200) {
                                    QMessageBox::warning(this, "", QString("歌单 %1 删除歌曲失败！\n%2\n%3").arg(playlistId).arg(json["code"].toInt()).arg(json["message"].toString()));
                                    return;
                                }
                            }
                        } else {
                            QMessageBox::warning(this, "", QString("添加到歌单 %1 失败！\n%2\n%3").arg(playlistId).arg(json["code"].toInt()).arg(json["message"].toString()));
                            return;
                        }
                    }
                }
            });
            context_menu.addAction(add_to_playlist_action);

            context_menu.exec(ui->tableView->mapToGlobal(pos));
        } else if (click_idx.column() == 5) {
            // 歌手列
            QMenu context_menu(ui->tableView);

            QModelIndex idx = ui->tableView->indexAt(pos);
            int row = idx.row();
            QModelIndex idx_ = model->index(row, 4);
            QList<QVariant> artists = model->data(idx_).toList();
            idx_ = model->index(row, 2);
            QList<QVariant> artist_ids = model->data(idx_).toList();

            if (artist_ids == QList<QVariant>()) {
                return;
            }

            for(int i = 0; i < artists.length(); i++) {
                QVariant artist = artists[i];
                QAction *select_artist_action = new QAction(artist.toString(), ui->tableView);
                context_menu.addAction(select_artist_action);
                QObject::connect(select_artist_action, &QAction::triggered, this, [this, i, &artist_ids](bool _) {
                    emit SelectArtistActionTriggered(artist_ids[i].toLongLong());
                });
            }
            auto conn = QObject::connect(this, &MainWindow::SelectArtistActionTriggered, this, [&](qint64 id){
                undoStack.push(Command{"Refresh", "ArtistDetail", {id}});
                RefreshArtistDetail(id);
            }, Qt::SingleShotConnection);
            context_menu.exec(ui->tableView->mapToGlobal(pos));
            QObject::disconnect(conn);
        } else if (click_idx.column() == 6) {
            // 专辑列
            QMenu context_menu(ui->tableView);

            QModelIndex idx = ui->tableView->indexAt(pos);
            int row = idx.row();

            QAction *action = new QAction("显示专辑内歌曲列表", ui->tableView);
            context_menu.addAction(action);
            auto conn = QObject::connect(action, &QAction::triggered, this, [&](bool _){
                QModelIndex idx = song_model->index(row, 1);
                qint64 album_id = song_model->data(idx).toLongLong();
                if (album_id == 0) {
                    return;
                }
                undoStack.push({"Refresh", "AlbumDetail", {album_id}});
                RefreshAlbumDetail(album_id);
            });
            context_menu.exec(ui->tableView->mapToGlobal(pos));
            QObject::disconnect(conn);
        }
    } else if (currentModel == kArtistModel) {
        model = artist_model;

        if (click_idx.column() == 1) {
            QMenu context_menu(ui->tableView);

            QModelIndex idx = ui->tableView->indexAt(pos);
            int row = idx.row();
            idx = model->index(row, 0);
            qint64 artistId = model->data(idx).toLongLong();
            if (artistId == 0) {
                return;
            }
            QAction *show_homepage_action = new QAction("显示歌手主页", ui->tableView);
            context_menu.addAction(show_homepage_action);
            auto connn = QObject::connect(show_homepage_action, &QAction::triggered, this, [&](){
                undoStack.push(Command{"Refresh", "ArtistDetail", {artistId}});
                RefreshArtistDetail(artistId);
            }, Qt::SingleShotConnection);

            QAction *show_album_action = new QAction("显示歌手专辑", ui->tableView);
            context_menu.addAction(show_album_action);
            auto conn = QObject::connect(show_album_action, &QAction::triggered, this, [&](){
                undoStack.push(Command{"Refresh", "ArtistAlbum", {artistId}});
                RefreshArtistAlbum(artistId);
            }, Qt::SingleShotConnection);
            context_menu.exec(ui->tableView->mapToGlobal(pos));
            QObject::disconnect(connn);
            QObject::disconnect(conn);
        }

    } else if (currentModel == kUserModel) {
        model = user_model;

        if (click_idx.column() == 1) {
            QMenu context_menu(ui->tableView);

            QModelIndex idx = ui->tableView->indexAt(pos);
            int row = idx.row();
            idx = model->index(row, 0);
            qint64 userId = model->data(idx).toLongLong();
            if (userId == 0) {
                return;
            }
            QAction *show_homepage_action = new QAction("显示用户主页", ui->tableView);
            context_menu.addAction(show_homepage_action);
            auto connn = QObject::connect(show_homepage_action, &QAction::triggered, this, [&](){
                undoStack.push(Command{"Refresh", "UserDetail", {userId}});
                RefreshUserDetail(userId);
            }, Qt::SingleShotConnection);

            QAction *show_sub_artists_action = new QAction("显示用户关注的歌手", ui->tableView);
            context_menu.addAction(show_sub_artists_action);
            auto conn = QObject::connect(show_sub_artists_action, &QAction::triggered, this, [&](){
                undoStack.push(Command{"Refresh", "UserFollowedArtist", {userId}});
                RefreshUserFollowedArtist(userId);
            }, Qt::SingleShotConnection);

            QAction *show_sub_users_action = new QAction("显示用户关注的用户", ui->tableView);
            context_menu.addAction(show_sub_users_action);
            auto connnn = QObject::connect(show_sub_users_action, &QAction::triggered, this, [&](){
                undoStack.push(Command{"Refresh", "UserFollowedUser", {userId}});
                RefreshFollowedUser(userId);
            }, Qt::SingleShotConnection);

            context_menu.exec(ui->tableView->mapToGlobal(pos));
            QObject::disconnect(connn);
            QObject::disconnect(conn);
            QObject::disconnect(connnn);
        }

    } else if (currentModel == kPlaylistModel) {
        model = playlist_model;

        if (click_idx.column() == 1) {
            QModelIndex idx = ui->tableView->indexAt(pos);
            int row = idx.row();

            idx = model->index(row, 3);
            qint64 creatorId = model->data(idx).toLongLong();
            if (creatorId != user_id) {
                return;
            }

            QMenu context_menu(ui->tableView);

            idx = model->index(row, 0);
            qint64 playlistId = model->data(idx).toLongLong();
            if (playlistId == 0) {
                return;
            }
            idx = model->index(row, 1);
            QString name = model->data(idx).toString();
            QAction *delete_playlist_action = new QAction("删除歌单", ui->tableView);
            context_menu.addAction(delete_playlist_action);
            auto conn = QObject::connect(delete_playlist_action, &QAction::triggered, this, [&](){
                QMessageBox::StandardButton choice = QMessageBox::warning(this, "", QString("确定要删除歌单 %1 吗?").arg(name), QMessageBox::Yes | QMessageBox::No);
                if (choice == QMessageBox::No) {
                    return;
                }
                QJsonDocument json = playlist.DeletePlaylist(requester, playlistId);
                if (json["code"] != 200) {
                    QMessageBox::warning(this, "", QString("删除歌单 %1 失败！\n%2\n%3").arg(playlistId).arg(json["code"].toInt()).arg(json["message"].toString()));
                    return;
                }
                RefreshMyPlaylist();
            }, Qt::SingleShotConnection);
            context_menu.exec(ui->tableView->mapToGlobal(pos));
            QObject::disconnect(conn);
        }
    }

}

void MainWindow::ConfigSongTableView() {
    song_model->setHorizontalHeaderLabels({"", "", "", "歌曲","", "歌手", "专辑", "时长",""});
    // QStringList vertical_header_labels(song_model->rowCount(), "Like; Download");
    // song_model->setVerticalHeaderLabels(vertical_header_labels);

    ui->tableView->setItemDelegate(delegate);

    ui->tableView->setColumnHidden(0, true);
    ui->tableView->setColumnHidden(1, true);
    ui->tableView->setColumnHidden(2, true);
    ui->tableView->setColumnHidden(3, false);   // 3
    ui->tableView->setColumnHidden(4, true);
    ui->tableView->setColumnHidden(5, false);   // 5
    ui->tableView->setColumnHidden(6, false);   // 6
    ui->tableView->setColumnHidden(7, false);   // 7
    ui->tableView->setColumnHidden(8, true);

    // ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setEditTriggers(QTableView::NoEditTriggers);

    ui->tableView->setColumnWidth(3, 400);
    ui->tableView->setColumnWidth(5, 250);
    ui->tableView->setColumnWidth(6, 250);
    ui->tableView->setColumnWidth(7, 50);
    ui->tableView->setColumnWidth(10, 100);

    if (ui->tableView->model()->rowCount() > 0) {
        ui->tableView->resizeColumnsToContents();
    }

    ui->tableView->setColumnWidth(9, 100);

    // ui->tableView->setVerticalHeader(header_view);
    ui->tableView->verticalHeader()->update();
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setVisible(true);
    ui->tableView->verticalHeader()->setVisible(true);

    song_model->setHeaderData(0, Qt::Horizontal, "song_model", Qt::EditRole);
    // song_model->headerData(0, Qt::Horizontal, Qt::EditRole);
}

void MainWindow::ConfigArtistTableView() {
    artist_model->setHorizontalHeaderLabels({"", "歌手", "关注"});

    ui->tableView->setItemDelegate(delegate);

    ui->tableView->setColumnHidden(0, true);
    ui->tableView->setColumnHidden(1, false);
    ui->tableView->setColumnHidden(2, false);

    // ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setEditTriggers(QTableView::NoEditTriggers);

    ui->tableView->setColumnWidth(1, 250);

    if (ui->tableView->model()->rowCount() > 0) {
        ui->tableView->resizeColumnsToContents();
    }

    // ui->tableView->setVerticalHeader(header_view);
    ui->tableView->verticalHeader()->update();
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setVisible(true);
    ui->tableView->verticalHeader()->setVisible(true);

    artist_model->setHeaderData(0, Qt::Horizontal, "artist_model", Qt::EditRole);
}

void MainWindow::ConfigAlbumTableView() {
    album_model->setHorizontalHeaderLabels({"", "专辑", "", "歌手"});

    ui->tableView->setItemDelegate(delegate);

    ui->tableView->setColumnHidden(0, true);
    ui->tableView->setColumnHidden(1, false);
    ui->tableView->setColumnHidden(2, true);
    ui->tableView->setColumnHidden(3, false);


    // ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setEditTriggers(QTableView::NoEditTriggers);

    ui->tableView->setColumnWidth(1, 250);

    if (ui->tableView->model()->rowCount() > 0) {
        ui->tableView->resizeColumnsToContents();
    }

    // ui->tableView->setVerticalHeader(header_view);
    ui->tableView->verticalHeader()->update();
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setVisible(true);
    ui->tableView->verticalHeader()->setVisible(true);

    album_model->setHeaderData(0, Qt::Horizontal, "album_model", Qt::EditRole);
}

void MainWindow::ConfigUserTableView() {
    user_model->setHorizontalHeaderLabels({"", "用户", "个性签名", "描述", "关注", ""});

    ui->tableView->setItemDelegate(delegate);

    ui->tableView->setColumnHidden(0, true);
    ui->tableView->setColumnHidden(1, false);
    ui->tableView->setColumnHidden(2, false);
    ui->tableView->setColumnHidden(3, false);
    ui->tableView->setColumnHidden(4, false);
    ui->tableView->setColumnHidden(5, true);


    // ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setEditTriggers(QTableView::NoEditTriggers);

    ui->tableView->setColumnWidth(1, 250);

    if (ui->tableView->model()->rowCount() > 0) {
        ui->tableView->resizeColumnsToContents();
    }

    // ui->tableView->setVerticalHeader(header_view);
    ui->tableView->verticalHeader()->update();
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setVisible(true);
    ui->tableView->verticalHeader()->setVisible(true);

    user_model->setHeaderData(0, Qt::Horizontal, "user_model", Qt::EditRole);
}

void MainWindow::ConfigPlaylistTableView() {
    playlist_model->setHorizontalHeaderLabels({"", "歌单", "创建用户", "", "描述", "关注", "歌曲数目"});

    ui->tableView->setItemDelegate(delegate);

    ui->tableView->setColumnHidden(0, true);
    ui->tableView->setColumnHidden(1, false);
    ui->tableView->setColumnHidden(2, false);
    ui->tableView->setColumnHidden(3, true);
    ui->tableView->setColumnHidden(4, false);
    ui->tableView->setColumnHidden(5, false);
    ui->tableView->setColumnHidden(6, false);



    // ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setEditTriggers(QTableView::NoEditTriggers);

    ui->tableView->setColumnWidth(1, 250);

    if (ui->tableView->model()->rowCount() > 0) {
        ui->tableView->resizeColumnsToContents();
    }

    // ui->tableView->setVerticalHeader(header_view);
    ui->tableView->verticalHeader()->update();
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setVisible(true);
    ui->tableView->verticalHeader()->setVisible(true);

    playlist_model->setHeaderData(0, Qt::Horizontal, "playlist_model", Qt::EditRole);
}

void MainWindow::ConfigRcmdPlaylistTableView() {
    rcmd_playlist_model->setHorizontalHeaderLabels({"", "歌单", "播放量"});

    ui->tableView->setItemDelegate(delegate);

    ui->tableView->setColumnHidden(0, true);
    ui->tableView->setColumnHidden(1, false);
    ui->tableView->setColumnHidden(2, false);

    // ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setEditTriggers(QTableView::NoEditTriggers);

    ui->tableView->setColumnWidth(1, 250);

    if (ui->tableView->model()->rowCount() > 0) {
        ui->tableView->resizeColumnsToContents();
    }

    // ui->tableView->setVerticalHeader(header_view);
    ui->tableView->verticalHeader()->update();
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setVisible(true);
    ui->tableView->verticalHeader()->setVisible(true);

    rcmd_playlist_model->setHeaderData(0, Qt::Horizontal, "rcmd_playlist_model", Qt::EditRole);
}

void MainWindow::ConfigFollowedTableView() {
    followed_model->setHorizontalHeaderLabels({"", "", "关注", "关注时长"});

    ui->tableView->setItemDelegate(delegate);

    ui->tableView->setColumnHidden(0, true);
    ui->tableView->setColumnHidden(1, true);
    ui->tableView->setColumnHidden(2, false);
    ui->tableView->setColumnHidden(3, false);


    // ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setEditTriggers(QTableView::NoEditTriggers);

    ui->tableView->setColumnWidth(1, 250);

    if (ui->tableView->model()->rowCount() > 0) {
        ui->tableView->resizeColumnsToContents();
    }

    // ui->tableView->setVerticalHeader(header_view);
    ui->tableView->verticalHeader()->update();
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setVisible(true);
    ui->tableView->verticalHeader()->setVisible(true);

    playlist_model->setHeaderData(0, Qt::Horizontal, "followed_model", Qt::EditRole);
}

void MainWindow::ConfigCommentTableView() {

}

void MainWindow::Append(SongInfo info) {
    QList <QStandardItem*> row_items(12);

    row_items[0] = new QStandardItem;
    row_items[0]->setData(info.id, Qt::EditRole);

    row_items[1] = new QStandardItem;
    row_items[1]->setData(info.album_id, Qt::EditRole);

    row_items[2] = new QStandardItem;
    row_items[2]->setData(info.artists_id, Qt::EditRole);

    row_items[3] = new QStandardItem;
    row_items[3]->setData(info.name, Qt::EditRole);

    row_items[4] = new QStandardItem;
    row_items[4]->setData(info.artists, Qt::EditRole);

    row_items[5] = new QStandardItem;
    row_items[5]->setData(info.artists_str, Qt::EditRole);

    row_items[6] = new QStandardItem;
    row_items[6]->setData(info.album, Qt::EditRole);

    row_items[7] = new QStandardItem;
    row_items[7]->setData(info.duration_str, Qt::EditRole);

    row_items[8] = new QStandardItem;
    row_items[8]->setData(info.duration, Qt::EditRole);

    row_items[9] = new QStandardItem;
    // row_items[9]->setData(info.download_progress, Qt::EditRole);
    row_items[9]->setData(QVariant::fromValue(QPair<double, QString>{0.0, QString()}));

    row_items[10] = new QStandardItem;
    row_items[10]->setData(info.is_favorite, Qt::EditRole);

    row_items[11] = new QStandardItem;
    row_items[11]->setData(1, Qt::EditRole); // 评论

    song_model->appendRow(row_items);
}

void MainWindow::Append(ArtistInfo info) {
    QList <QStandardItem*> row_items(3);

    row_items[0] = new QStandardItem;
    row_items[0]->setData(info.id, Qt::EditRole);

    row_items[1] = new QStandardItem;
    row_items[1]->setData(info.name, Qt::EditRole);

    row_items[2] = new QStandardItem;
    row_items[2]->setData(info.is_followed, Qt::EditRole);

    artist_model->appendRow(row_items);
}

void MainWindow::Append(AlbumInfo info) {
    QList <QStandardItem*> row_items(4);

    row_items[0] = new QStandardItem;
    row_items[0]->setData(info.id, Qt::EditRole);

    row_items[1] = new QStandardItem;
    row_items[1]->setData(info.name, Qt::EditRole);

    row_items[2] = new QStandardItem;
    row_items[2]->setData(info.artist_id, Qt::EditRole);

    row_items[3] = new QStandardItem;
    row_items[3]->setData(info.artist, Qt::EditRole);

    album_model->appendRow(row_items);
}

void MainWindow::Append(UserInfo info) {
    QList <QStandardItem*> row_items(6);

    row_items[0] = new QStandardItem;
    row_items[0]->setData(info.id, Qt::EditRole);

    row_items[1] = new QStandardItem;
    if (info.gender == 2) {
        row_items[1]->setData(info.name.toString() + " ♀", Qt::EditRole);
        row_items[1]->setData(QBrush(QColor(247, 45, 126)), Qt::ForegroundRole);
    } else if (info.gender == 1) {
        row_items[1]->setData(info.name.toString() + " ♂", Qt::EditRole);
        row_items[1]->setData(QBrush(QColor(82, 82, 255)), Qt::ForegroundRole);
    } else {
        row_items[1]->setData(info.name.toString() + " ⚧", Qt::EditRole);
    }

    row_items[2] = new QStandardItem;
    row_items[2]->setData(info.signature, Qt::EditRole);

    row_items[3] = new QStandardItem;
    row_items[3]->setData(info.description, Qt::EditRole);

    row_items[4] = new QStandardItem;
    row_items[4]->setData(info.is_followed, Qt::EditRole);


    user_model->appendRow(row_items);
}

void MainWindow::Append(PlaylistInfo info) {
    QList <QStandardItem*> row_items(7);

    row_items[0] = new QStandardItem;
    row_items[0]->setData(info.id, Qt::EditRole);

    row_items[1] = new QStandardItem;
    row_items[1]->setData(info.name, Qt::EditRole);

    row_items[2] = new QStandardItem;
    row_items[2]->setData(info.creator, Qt::EditRole);

    row_items[3] = new QStandardItem;
    row_items[3]->setData(info.creator_id, Qt::EditRole);

    row_items[4] = new QStandardItem;
    row_items[4]->setData(info.description, Qt::EditRole);

    row_items[5] = new QStandardItem;
    row_items[5]->setData(info.is_like, Qt::EditRole);

    row_items[6] = new QStandardItem;
    row_items[6]->setData(info.trackCount, Qt::EditRole);

    playlist_model->appendRow(row_items);
}

void MainWindow::Append(FollowedInfo info) {
    QList <QStandardItem*> row_items(4);

    row_items[0] = new QStandardItem;
    row_items[0]->setData(info.type, Qt::EditRole);

    row_items[1] = new QStandardItem;
    row_items[1]->setData(info.id, Qt::EditRole);

    row_items[2] = new QStandardItem;
    if (info.type == 1 or info.type == 3) {
        if (info.gender == 2) {
            row_items[2]->setData(info.name.toString() + " ♀", Qt::EditRole);
            row_items[2]->setData(QBrush(QColor(247, 45, 126)), Qt::ForegroundRole);
        } else if (info.gender == 1) {
            row_items[2]->setData(info.name.toString() + " ♂", Qt::EditRole);
            row_items[2]->setData(QBrush(QColor(82, 82, 255)), Qt::ForegroundRole);
        } else {
            row_items[2]->setData(info.name.toString() + " ⚧", Qt::EditRole);
        }
    } else {
        row_items[2]->setData(info.name.toString(), Qt::EditRole);
    }

    row_items[3] = new QStandardItem;
    row_items[3]->setData(info.follow_day, Qt::EditRole);


    followed_model->appendRow(row_items);
}

void MainWindow::Append(RcmdPlaylistInfo info) {
    QList <QStandardItem*> row_items(3);

    row_items[0] = new QStandardItem;
    row_items[0]->setData(info.id, Qt::EditRole);

    row_items[1] = new QStandardItem;
    row_items[1]->setData(info.name, Qt::EditRole);

    row_items[2] = new QStandardItem;
    row_items[2]->setData(info.play_count, Qt::EditRole);

    rcmd_playlist_model->appendRow(row_items);
}

void MainWindow::Append(CommentInfo info) {
    QList <QStandardItem*> row_items(7);

    row_items[0] = new QStandardItem;
    row_items[0]->setData(info.id, Qt::EditRole);

    row_items[1] = new QStandardItem;
    row_items[1]->setData(info.userId, Qt::EditRole);

    row_items[2] = new QStandardItem;
    row_items[2]->setData(info.userName, Qt::EditRole);

    row_items[3] = new QStandardItem;
    row_items[3]->setData(info.songId, Qt::EditRole);

    row_items[4] = new QStandardItem;
    row_items[4]->setData(info.content, Qt::EditRole);

    row_items[5] = new QStandardItem;
    row_items[5]->setData(info.likedCount, Qt::EditRole);

    row_items[6] = new QStandardItem;
    row_items[6]->setData(info.replyCount, Qt::EditRole);

    comments_model->appendRow(row_items);
}

void MainWindow::UpdateInfoVisibility(bool visible) {
    homepageWidget->hide();
    ui->gridLayout->removeWidget(homepageWidget);
    currentWidget = kDetail;
    if (visible) {
        // ui->tableView->setGeometry(table_view_rect);
        ui->descriptionLabel->show();
        ui->descriptionScrollArea->show();
        ui->imageLabel->show();
        ui->gridLayout->removeWidget(ui->tableView);
        ui->gridLayout->addWidget(ui->tableView, 4, 0, 2, 6);
    } else {
        ui->descriptionLabel->hide();
        ui->downloadCoverPicButton->hide();
        ui->descriptionScrollArea->hide();
        ui->imageLabel->hide();
        ui->creator_button->hide();
        ui->rcmd_button->hide();
        ui->downloadCoverPicButton->hide();
        ui->sub_button->hide();
        ui->gridLayout->removeWidget(ui->tableView);
        ui->gridLayout->addWidget(ui->tableView, 1, 0, 3, 6);
    }
    ui->tableView->show();
}

void MainWindow::RefreshArtistAlbum(qint64 artist_id) {
    Refreshing();

    currentModel = kAlbumModel;
    ui->tableView->setModel(album_model);
    album_model->clear();
    QJsonDocument json = playlist.GetArtistAlbums(requester, artist_id, offset, PAGE_SIZE);
    QJsonArray hotAlbums = json["hotAlbums"].toArray();
    for (auto hotAlbumRef : hotAlbums) {
        QJsonValue hotAlbum = hotAlbumRef;
        AlbumInfo albumInfo;
        albumInfo.id = hotAlbum["id"].toInteger();
        albumInfo.artist_id = hotAlbum["artist"]["id"].toInteger();
        albumInfo.artist = hotAlbum["artist"]["name"].toString();
        if (hotAlbum["artist"]["alias"] != QJsonValue::Undefined && hotAlbum["artist"]["alias"].toArray().size() > 0) {
            albumInfo.name = hotAlbum["name"].toString() + "(" + hotAlbum["artist"]["alias"][0].toString() + ")";
        } else {
            albumInfo.name = hotAlbum["name"].toString();
        }
        albumInfo.is_like = hotAlbum["isSub"].toBool();
        Append(albumInfo);
    }
    ConfigAlbumTableView();

    QJsonValue artist_homepage = user.GetArtistHomePage(requester, artist_id)["data"]["artist"];
    int i = 0;
    QString name = artist_homepage["name"].toString() + (artist_homepage["transNames"][0] == QJsonValue::Undefined or artist_homepage["transNames"][0].toString().isEmpty() ? "" : QString("(%1)").arg(artist_homepage["transNames"][0].toString()));
    QString artist_pic_url, artist_description;
    artist_pic_url = artist_homepage["cover"].toString();
    artist_description = artist_homepage["briefDesc"].toString();
    ui->descriptionLabel->setText(name + "\n" + artist_description);
    QObject::connect(downloader, &Downloader::DownloadFinished, this, [artist_pic_url, this]() { DisplayAlbumPicture(artist_pic_url); }, Qt::SingleShotConnection);
    downloader->PictureDownload(artist_pic_url);

    QJsonValue follow = user.GetArtistFollowerCount(requester, artist_id)["data"];
    qint64 subscribedCount = follow["fansCnt"].toInteger();
    QString followDay = follow["followDay"].toString();
    bool sub = follow["follow"].toBool();

    // static QMetaObject::Connection prevSubBtnConn, prevRcmdBtnConn;
    if (prevRcmdBtnConn != QMetaObject::Connection()) {
        QObject::disconnect(prevRcmdBtnConn);
    }
    prevRcmdBtnConn = QObject::connect(ui->rcmd_button, &QPushButton::clicked, this, [=, this]() {
        undoStack.push(Command{"Refresh", "RecommendArtist", {artist_id}});
        RefreshRcmdArtist(artist_id);
    });
    Refreshed();
    UpdateInfoVisibility(true);
    ui->pageUpButton->setEnabled(false);
    ui->pageDownButton->setEnabled(false);
    ui->creator_button->hide();
    if (name == user_nick_name) {
        ui->sub_button->hide();
        ui->rcmd_button->hide();
    } else {
        ui->sub_button->show();
        ui->rcmd_button->show();
        ui->sub_button->setText((sub? followDay : "关注") + (subscribedCount == 0 ? "" : QString("(%1)").arg(subscribedCount)));
        ui->sub_button->adjustSize();
        if (prevSubBtnConn != QMetaObject::Connection()) {
            QObject::disconnect(prevSubBtnConn);
        }
        prevSubBtnConn = QObject::connect(ui->sub_button, &QPushButton::clicked, this, [=, this]() mutable {
            if (sub) {
                bool code = user.DelfollowArtist(requester, artist_id);
                if (code) {
                    sub = !sub;
                    subscribedCount--;
                    ui->sub_button->setText((sub ? followDay : "关注") + (subscribedCount == 0 ? "" : QString("(%1)").arg(subscribedCount)));
                    ui->sub_button->adjustSize();
                } else {
                    // qDebug() << "Failing while unsub artist!";
                }
            } else {
                bool code = user.FollowArtist(requester, artist_id);
                if (code) {
                    sub = !sub;
                    subscribedCount++;
                    ui->sub_button->setText((sub ? followDay : "关注") + QString("(%1)").arg(subscribedCount));
                    ui->sub_button->adjustSize();
                } else {
                    // qDebug() << "Failing while sub artist!";
                }
            }
        });
    }
}

void MainWindow::RefreshArtistDetail(qint64 artist_id) {
    Refreshing();
    QJsonDocument json = playlist.GetArtistHot(requester, artist_id);

    song_model->clear();
    RefreshPlaylist(json["songs"].toArray());

    QJsonValue artist_homepage = user.GetArtistHomePage(requester, artist_id)["data"]["artist"];
    int i = 0;
    QString name = artist_homepage["name"].toString() + (artist_homepage["transNames"][0] == QJsonValue::Undefined or artist_homepage["transNames"][0].toString().isEmpty() ? "" : QString("(%1)").arg(artist_homepage["transNames"][0].toString()));
    QString artist_pic_url, artist_description;
    artist_pic_url = artist_homepage["cover"].toString();
    artist_description = artist_homepage["briefDesc"].toString();
    ui->descriptionLabel->setText(name + "\n" + artist_description);
    Downloader *artist_profile_img_downloader = new Downloader();
    QObject::connect(artist_profile_img_downloader, &Downloader::DownloadFinished, this, [=, this]() { DisplayAlbumPicture(artist_pic_url); artist_profile_img_downloader->deleteLater(); }, Qt::SingleShotConnection);
    artist_profile_img_downloader->PictureDownload(artist_pic_url);

    QJsonValue follow = user.GetArtistFollowerCount(requester, artist_id)["data"];
    qint64 subscribedCount = follow["fansCnt"].toInteger();
    QString followDay = follow["followDay"].toString();
    bool sub = follow["follow"].toBool();

    // static QMetaObject::Connection prevSubBtnConn, prevRcmdBtnConn;
    if (prevRcmdBtnConn != QMetaObject::Connection()) {
        QObject::disconnect(prevRcmdBtnConn);
    }
    prevRcmdBtnConn = QObject::connect(ui->rcmd_button, &QPushButton::clicked, this, [=, this]() {
        undoStack.push(Command{"Refresh", "RecommendArtist", {artist_id}});
        RefreshRcmdArtist(artist_id);
    });
    Refreshed();
    UpdateInfoVisibility(true);
    ui->pageUpButton->setEnabled(false);
    ui->pageDownButton->setEnabled(false);
    ui->creator_button->hide();
    cover_title = name;
    ui->downloadCoverPicButton->show();
    if (name == user_nick_name) {
        ui->sub_button->hide();
        ui->rcmd_button->hide();
    } else {
        ui->sub_button->show();
        ui->rcmd_button->show();
        ui->sub_button->setText((sub? followDay : "关注") + (subscribedCount == 0 ? "" : QString("(%1)").arg(subscribedCount)));
        ui->sub_button->adjustSize();
        if (prevSubBtnConn != QMetaObject::Connection()) {
            QObject::disconnect(prevSubBtnConn);
        }
        prevSubBtnConn = QObject::connect(ui->sub_button, &QPushButton::clicked, this, [=, this]() mutable {
            if (sub) {
                bool code = user.DelfollowArtist(requester, artist_id);
                if (code) {
                    sub = !sub;
                    subscribedCount--;
                    ui->sub_button->setText((sub ? followDay : "关注") + (subscribedCount == 0 ? "" : QString("(%1)").arg(subscribedCount)));
                    ui->sub_button->adjustSize();
                } else {
                    // qDebug() << "Failing while unsub artist!";
                }
            } else {
                bool code = user.FollowArtist(requester, artist_id);
                if (code) {
                    sub = !sub;
                    subscribedCount++;
                    ui->sub_button->setText((sub ? followDay : "关注") + QString("(%1)").arg(subscribedCount));
                    ui->sub_button->adjustSize();
                } else {
                    // qDebug() << "Failing while sub artist!";
                }
            }
        });
    }
}

void MainWindow::RefreshRcmdArtist(qint64 artist_id) {
    Refreshing();
    currentModel = kArtistModel;
    ui->tableView->setModel(artist_model);
    artist_model->clear();

    QJsonArray artists = user.GetRcmdArtist(requester, artist_id)["artists"].toArray();

    for (QJsonValueRef artist : artists) {
        ArtistInfo artistInfo;
        QJsonObject artistObj = artist.toObject();
        artistInfo.id = artistObj["id"].toInteger();
        artistInfo.name = artistObj["name"].toString() + (artistObj["trans"] == QJsonValue::Undefined or artistObj["trans"].toString().isEmpty() ? "" : "(" + artistObj["trans"].toString() + ")");
        artistInfo.is_followed = artistObj["followed"].toBool();
        Append(artistInfo);
    }
    ConfigArtistTableView();
    Refreshed();
    UpdateInfoVisibility(false);
}

void MainWindow::RefreshMydefPlaylist(QList<QVariant> song_ids) {
    Refreshing();
    song_model->clear();
    RefreshPlaylist(playlist.GetSongDetail(requester, song_ids));
    Refreshed();
    UpdateInfoVisibility(false);
    ui->pageUpButton->setEnabled(false);
    ui->pageDownButton->setEnabled(false);
    ui->sub_button->hide();
    ui->creator_button->hide();
    ui->rcmd_button->hide();
}

void MainWindow::RefreshUserPlaylists(QJsonDocument json) {
    Refreshing();

    currentModel = kPlaylistModel;
    ui->tableView->setModel(playlist_model);
    playlist_model->clear();

    // // qDebug() << json;

    QJsonArray playlists = json["playlist"].toArray();

    for (QJsonValueRef playlist : playlists) {
        PlaylistInfo playlistInfo;
        QJsonValue playlistObj = playlist;
        playlistInfo.id = playlistObj["id"].toInteger();
        playlistInfo.name = playlistObj["name"].toString();
        playlistInfo.creator = playlistObj["creator"]["nickname"].toString();
        playlistInfo.creator_id = playlistObj["creator"]["userId"].toInteger();
        playlistInfo.description = playlistObj["description"].toString();
        playlistInfo.is_like = (playlistInfo.creator_id == user_id ? 2 : (playlistObj["subscribed"].toBool() ? 1 : 0));
        playlistInfo.trackCount = playlistObj["trackCount"].toInt();

        Append(playlistInfo);
    }
    ConfigPlaylistTableView();
    Refreshed();

    if (offset > 0) {
        ui->pageUpButton->setEnabled(true);
    } else {
        ui->pageUpButton->setEnabled(false);
    }
    bool has_more = json["more"].toBool();
    // // qDebug() << has_more;
    if (has_more) {
        ui->pageDownButton->setEnabled(true);
    } else {
        ui->pageDownButton->setEnabled(false);
    }

    UpdateInfoVisibility(true);
}

void MainWindow::RefreshUserFollowedArtist(qint64 userId) {
    currentModel = kArtistModel;
    ui->tableView->setModel(artist_model);
    artist_model->clear();

    QJsonArray artists = user.GetUserFollowedArtist(requester , userId);

    for (QJsonValueRef artist : artists) {
        ArtistInfo artistInfo;
        QJsonObject artistObj = artist.toObject();
        artistInfo.id = artistObj["id"].toInteger();

        QString tmp_name;
        tmp_name = artistObj["name"].toString();
        if (artistObj["trans"] != QJsonValue::Undefined && artistObj["trans"].toString() != "") {
            tmp_name += "(" + artistObj["trans"].toString() + ")";
        }
        artistInfo.name = tmp_name;
        artistInfo.is_followed = artistObj["followed"].toBool();

        Append(artistInfo);

    }
    ConfigArtistTableView();
    Refreshed();
    UpdateInfoVisibility(false);
}

void MainWindow::RefreshFollowedUser(qint64 userId) {
    currentModel = kUserModel;
    ui->tableView->setModel(user_model);
    user_model->clear();

    QJsonArray records = user.GetFollowedUser(requester, userId);
    for (QJsonValueRef user : records) {
        UserInfo userInfo;
        QJsonValue userObj = user.toObject()["userProfile"];
        userInfo.id = userObj["userId"].toInteger();
        userInfo.name = userObj["nickname"].toString();
        userInfo.signature = userObj["signature"].toString();
        userInfo.description = userObj["description"].toString();
        userInfo.is_followed = userObj["followed"].toBool();
        userInfo.gender = userObj["gender"].toInt();

        Append(userInfo);

    }
    ConfigUserTableView();
    Refreshed();
    UpdateInfoVisibility(false);
}

void MainWindow::RefreshUserDetail(qint64 user_id) {
    Refreshing();

    QJsonDocument json = user.GetUserDetail(requester, user_id);
    // qDebug() << json;
    QString username = json["profile"]["nickname"].toString();
    QString signature = json["profile"]["signature"].toString();
    QString detailDescription = json["profile"]["detailDescription"].toString();
    QString level = QString::number(json["level"].toInt());
    QString listenSongs = QString::number(json["listenSongs"].toInteger());
    qint64 followeds = json["profile"]["followeds"].toInteger();
    qint64 follows = json["profile"]["follows"].toInteger();
    bool sub = json["profile"]["followed"].toBool();
    bool followMe = json["profile"]["followMe"].toBool();
    QString ta = json["profile"]["privacyItemUnlimit"]["gender"].toBool() ? (json["profile"]["gender"] == 1 ? "他" : (json["profile"]["gender"] == 2 ? "她" : "Ta")) : "Ta";

    ui->descriptionLabel->setText("用户名：" + username + "\n" +
                                  (level.isEmpty() ? "" : "Level: " + level + "\n") +
                                  (listenSongs.isEmpty() or listenSongs == "0" ? "" : ta + "已经听了" + listenSongs + "首歌了呢！\n") +
                                  (signature != "" ? ta + "的个性签名：" + signature + "\n" : "") +
                                  (detailDescription != "" ? ta + "的简介：" + detailDescription + "\n" : "") +
                                  (followeds != 0 ? ta + QString("被%1个人关注了哦\n").arg(followeds) : "") +
                                  (follows != 0 ? ta + QString("关注了%1个人啦\n").arg(follows) : "") +
                                  (followMe ? QString("Wow～～～%1正在默默守护着你哦～～～\n").arg(ta) : QString("人家还没有关注你哦，再努力一点让%1看到吧～～～\n").arg(ta)));

    QJsonDocument playlistsJson = user.GetUserPlaylists(requester, user_id);
    RefreshUserPlaylists(playlistsJson);

    QString avatarPicUrl = json["profile"]["avatarUrl"].toString();
    Downloader *user_profile_img_downloader = new Downloader();
    QObject::connect(user_profile_img_downloader, &Downloader::DownloadFinished, this, [=, this]() { DisplayAlbumPicture(avatarPicUrl); user_profile_img_downloader->deleteLater(); }, Qt::SingleShotConnection);
    user_profile_img_downloader->PictureDownload(avatarPicUrl);

    // static QMetaObject::Connection prevSubBtnConn;
    Refreshed();
    UpdateInfoVisibility(true);
    ui->pageUpButton->setEnabled(false);
    ui->pageDownButton->setEnabled(false);
    ui->creator_button->hide();
    ui->rcmd_button->hide();
    if (username == user_nick_name) {
        ui->sub_button->hide();
    } else {
        ui->sub_button->show();
        ui->sub_button->setText((sub? QString("你已经关注%1啦～").arg(ta) : "关注"));
        ui->sub_button->adjustSize();
        if (prevSubBtnConn != QMetaObject::Connection()) {
            QObject::disconnect(prevSubBtnConn);
        }
        prevSubBtnConn = QObject::connect(ui->sub_button, &QPushButton::clicked, this, [=, this]() mutable {
            if (sub) {
                bool code = user.DelfollowUser(requester, user_id);
                if (code) {
                    sub = !sub;
                    ui->sub_button->setText((sub? QString("你已经关注%1啦～").arg(ta) : "关注"));
                    ui->sub_button->adjustSize();
                } else {
                    // qDebug() << "Failing while unsub user!";
                }
            } else {
                bool code = user.FollowUser(requester, user_id);
                if (code) {
                    sub = !sub;
                    ui->sub_button->setText((sub? QString("你已经关注%1啦～").arg(ta) : "关注"));
                    ui->sub_button->adjustSize();
                } else {
                    // qDebug() << "Failing while sub user!";
                }
            }
        });
    }
    cover_title = username;
    ui->downloadCoverPicButton->show();
}

void MainWindow::RefreshAlbumDetail(qint64 album_id) {
    Refreshing();

    QJsonDocument json = playlist.GetAlbumPlaylist(requester, album_id);
    QString album_description = json["album"].toObject()["description"].toString();
    ui->descriptionLabel->setText(album_description);

    cover_title = json["album"]["name"].toString();

    song_model->clear();
    RefreshPlaylist(json["songs"].toArray());

    QString album_pic_url = json["album"].toObject()["picUrl"].toString();
    Downloader *album_img_downloader = new Downloader();
    QObject::connect(album_img_downloader, &Downloader::DownloadFinished, this, [=, this]() { DisplayAlbumPicture(album_pic_url); album_img_downloader->deleteLater(); }, Qt::SingleShotConnection);
    album_img_downloader->PictureDownload(album_pic_url);

    json = playlist.GetAlbumSub(requester, album_id);
    bool sub = json["isSub"].toBool();
    qint64 subscribedCount = json["subCount"].toInteger();
    // static QMetaObject::Connection prevSubBtnConn;
    ui->sub_button->setText((sub == true ? "已收藏" : "收藏") + (subscribedCount == 0 ? "" : QString("(%1)").arg(subscribedCount)));
    ui->sub_button->adjustSize();
    if (prevSubBtnConn != QMetaObject::Connection()) {
        QObject::disconnect(prevSubBtnConn);
    }
    prevSubBtnConn = QObject::connect(ui->sub_button, &QPushButton::clicked, this, [=, this]() mutable {
        if (sub) {
            bool code = playlist.UnsubscribeAlbum(requester, album_id);
            if (code) {
                sub = !sub;
                subscribedCount--;
                ui->sub_button->setText((sub == true ? "已收藏" : "收藏") + (subscribedCount == 0 ? "" : QString("(%1)").arg(subscribedCount)));
                ui->sub_button->adjustSize();
            } else {
                // qDebug() << "Failing while unsub album!";
            }
        } else {
            bool code = playlist.SubscribeAlbum(requester, album_id);
            if (code) {
                sub = !sub;
                subscribedCount++;
                ui->sub_button->setText((sub == true ? "已收藏" : "收藏") + QString("(%1)").arg(subscribedCount));
                ui->sub_button->adjustSize();
            } else {
                // qDebug() << "Failing while sub album!";
            }
        }
    });
    Refreshed();
    UpdateInfoVisibility(true);
    ui->sub_button->show();
    ui->downloadCoverPicButton->show();
    ui->pageUpButton->setEnabled(false);
    ui->pageDownButton->setEnabled(false);
}

void MainWindow::RefreshDailyRecommend() {
    Refreshing();

    QJsonDocument json = user.GetDailyRecommend(requester);
    song_model->clear();
    RefreshPlaylist(json["data"]["dailySongs"].toArray());

    Refreshed();
    UpdateInfoVisibility(false);
    ui->pageUpButton->setEnabled(false);
    ui->pageDownButton->setEnabled(false);
}

void MainWindow::RefreshComment(qint64 song_id, QString song_info) {
    commentsDialog->clearComments();
    commentsDialog->refreshComments(requester, song_id, song_info);
    commentsDialog->show();
    commentsDialog->raise();
}

void MainWindow::RefreshMyRcmdUser() {
    Refreshing();

    currentModel = kUserModel;
    ui->tableView->setModel(user_model);
    user_model->clear();

    QJsonDocument json = user.GetMyFlavor(requester);
    QJsonArray users = json["users"].toArray();

    for (QJsonValueRef user : users) {
        UserInfo userInfo;
        QJsonValue userObj = user;
        // qDebug() << userObj;
        userInfo.id = userObj["userId"].toInteger();
        userInfo.name = userObj["nickname"].toString();
        userInfo.signature = userObj["signature"].toString();
        userInfo.description = userObj["description"].toString();
        userInfo.is_followed = userObj["followed"].toBool();
        userInfo.gender = userObj["gender"].toInt();
        Append(userInfo);

    }
    ConfigUserTableView();
    Refreshed();
    UpdateInfoVisibility(false);
}

void MainWindow::RefreshPlaylistDetail(qint64 playlist_id) {
    Refreshing();

    QJsonValue json = playlist.GetPlaylistDetail(requester, playlist_id)["playlist"];
    QString playlist_name = json["name"].toString();
    cover_title = playlist_name;
    QJsonValue playlist_description = json["description"];
    int trackCount = json["trackCount"].toInt();
    QList <QString> tagsList;
    for (auto t : json["tags"].toArray()) {
        tagsList.append(t.toString());
    }
    QString tags = tagsList.join("/");
    QString playlist_comp_desc = playlist_name
            + QString("\n共%1首歌").arg(trackCount)
            + (tags == "" ? "" : "·" + tags)
            + (playlist_description == QJsonValue::Null ? "" : "\n" + playlist_description.toString());
    ui->descriptionLabel->setText(playlist_comp_desc);

    bool sub = json["subscribed"].toBool();
    qint64 subscribedCount = json["subscribedCount"].toInteger();
    QString creator = json["creator"]["nickname"].toString();
    qint64 userId = json["creator"]["userId"].toInteger();
    qint64 id = json["id"].toInteger();

    QString playlist_pic_url = json.toObject()["coverImgUrl"].toString();

    Downloader *album_img_downloader = new Downloader();
    QObject::connect(album_img_downloader, &Downloader::DownloadFinished, this, [=, this]() { DisplayAlbumPicture(playlist_pic_url); album_img_downloader->deleteLater(); }, Qt::SingleShotConnection);
    album_img_downloader->PictureDownload(playlist_pic_url);
    // static QMetaObject::Connection prevSubBtnConn, prevRcmdBtnConn, prevCrBtnConn;
    if (prevRcmdBtnConn != QMetaObject::Connection()) {
        QObject::disconnect(prevRcmdBtnConn);
    }
    prevRcmdBtnConn = QObject::connect(ui->rcmd_button, &QPushButton::clicked, this, [=, this]() {
        undoStack.push(Command{"Refresh", "RecommendPlaylist", {id}});
        RefreshRcmdPlaylist(id);
    });

    int cursor = 0;
    QJsonArray trackIds = json["trackIds"].toArray();

    song_model->clear();
    if (trackCount <= 200) {   
        RefreshPlaylist(playlist.GetSongDetail(requester, trackIds));
        ui->playlistPageDownButton->hide();
    } else {
        QJsonArray trackId_block;
        for (;cursor < 200; cursor++) {
            trackId_block.append(trackIds[cursor]);
        }
        RefreshPlaylist(playlist.GetSongDetail(requester, trackId_block));
        ui->playlistPageDownButton->show();
        ui->playlistPageDownButton->setEnabled(true);
        if (playlistPageDownConn != QMetaObject::Connection()) {
            QObject::disconnect(playlistPageDownConn);
        }
        playlistPageDownConn = QObject::connect(ui->playlistPageDownButton, &QPushButton::clicked, this, [=, this]() mutable {
            trackId_block = QJsonArray();
            for (int t = cursor + 200; cursor < t; cursor++) {
                trackId_block.append(trackIds[cursor]);
                if (cursor == trackCount - 1) {
                    ui->playlistPageDownButton->hide();
                    QObject::disconnect(playlistPageDownConn);
                    break;
                }
            }
            Refreshing();
            RefreshPlaylist(playlist.GetSongDetail(requester, trackId_block));
            UpdateInfoVisibility(true);
            if (cursor < trackCount - 1) {
                ui->playlistPageDownButton->show();
            }
            Refreshed();
            ui->pageDownButton->setEnabled(false);
            ui->pageUpButton->setEnabled(false);

            if (userId == user_id) {
                ui->sub_button->hide();
                ui->creator_button->hide();
                ui->rcmd_button->hide();
            } else {
                ui->sub_button->show();
                ui->creator_button->show();
                ui->rcmd_button->show();
                ui->downloadCoverPicButton->show();
            }
        });
    }

    Refreshed();
    UpdateInfoVisibility(true);
    ui->pageUpButton->setEnabled(false);
    ui->pageDownButton->setEnabled(false);
    ui->playlistPageDownButton->setEnabled(true);

    if (userId == user_id) {
        ui->sub_button->hide();
        ui->creator_button->hide();
        ui->rcmd_button->hide();
    } else {
        ui->sub_button->show();
        ui->creator_button->show();
        ui->rcmd_button->show();
        ui->downloadCoverPicButton->show();
        ui->sub_button->setText((sub == true ? "已收藏" : "收藏") + (subscribedCount == 0 ? "" : QString("(%1)").arg(subscribedCount)));
        ui->sub_button->adjustSize();
        if (prevSubBtnConn != QMetaObject::Connection()) {
            QObject::disconnect(prevSubBtnConn);
        }
        prevSubBtnConn = QObject::connect(ui->sub_button, &QPushButton::clicked, this, [=, this]() mutable {
            if (sub) {
                bool code = playlist.UnsubscribePlaylist(requester, id);
                if (code) {
                    sub = !sub;
                    subscribedCount--;
                    ui->sub_button->setText((sub == true ? "已收藏" : "收藏") + (subscribedCount == 0 ? "" : QString("(%1)").arg(subscribedCount)));
                    ui->sub_button->adjustSize();
                } else {
                    // qDebug() << "Failing while unsub playlist!";
                }
            } else {
                bool code = playlist.SubscribePlaylist(requester, id);
                if (code) {
                    sub = !sub;
                    subscribedCount++;
                    ui->sub_button->setText((sub == true ? "已收藏" : "收藏") + QString("(%1)").arg(subscribedCount));
                    ui->sub_button->adjustSize();
                } else {
                    // qDebug() << "Failing while sub playlist!";
                }
            }
        });
        if (creator == "") {
            ui->creator_button->hide();
        } else {
            ui->creator_button->setText(creator);
            ui->creator_button->adjustSize();
            if (prevCrBtnConn != QMetaObject::Connection()) {
                QObject::disconnect(prevCrBtnConn);
            }
            prevCrBtnConn = QObject::connect(ui->creator_button, &QPushButton::clicked, this, [=, this](){
                undoStack.push(Command{"Refresh", "UserDetail", {userId}});
                RefreshUserDetail(userId);
            });

        }
    }
}

void MainWindow::RefreshMyPlaylist() {
    Refreshing();

    QJsonDocument json = user.GetUserPlaylists(requester, user_id);
    RefreshUserPlaylists(json);
    UpdateInfoVisibility(false);

    Refreshed();
}

void MainWindow::RefreshRcmdPlaylist(qint64 playlist_id) {
    Refreshing();
    ui->tableView->setModel(rcmd_playlist_model);
    currentModel = kRcmdPlaylistModel;
    rcmd_playlist_model->clear();

    QJsonArray playlists = playlist.GetRcmdPlaylist(requester, playlist_id);

    for (QJsonValueRef playlist : playlists) {
        RcmdPlaylistInfo playlistInfo;
        QJsonValue playlistObj = playlist.toObject()["playlist"];
        playlistInfo.id = playlistObj["id"].toInteger();
        playlistInfo.name = playlistObj["name"].toString();
        playlistInfo.play_count = playlistObj["playCount"].toInteger();
        Append(playlistInfo);
    }

    ConfigRcmdPlaylistTableView();
    Refreshed();
    UpdateInfoVisibility(false);
    ui->pageUpButton->setEnabled(false);
    ui->pageUpButton->setEnabled(false);
}

void MainWindow::RefreshPlaylist(QJsonArray songs) {
    Refreshing();
    ui->tableView->setModel(song_model);
    currentModel = kSongModel;

    if (songs.size() == 0) {
        ConfigSongTableView();
        Refreshed();
        ui->pageDownButton->setEnabled(false);
        ui->pageUpButton->setEnabled(false);
        return;
    }

    favourites = user.GetFavouriteIds(requester, favourite_id);
    for (QJsonValueRef song : songs) {
        QJsonObject songObj = song.toObject();
        SongInfo songInfo;

        songInfo.id = songObj["id"].toInteger();

        QString tmp_name;
        tmp_name = songObj["name"].toString();
        if (songObj["alias"] != QJsonValue::Undefined) {
            QJsonArray tns_name_list = songObj["alias"].toArray();
            if (tns_name_list.size() > 0 && tns_name_list[0] != QJsonValue::Undefined && tns_name_list[0].toString() != "") {
                tmp_name += "(" + tns_name_list[0].toString() + ")";
            }
        }
        songInfo.name = tmp_name;

        songInfo.duration = songObj["dt"].toInt();
        songInfo.duration_str = QString("%1:%2").arg(songInfo.duration.toInt() / 1000 / 60).arg((int)(songInfo.duration.toInt() / 1000 % 60), 2, 10, (QChar)u'0');
        QString tmp_artists_str;
        for (QJsonValueRef artist : songObj["ar"].toArray()) {
            QJsonObject artist_obj = artist.toObject();

            songInfo.artists.append(artist_obj["name"].toString());
            tmp_artists_str += artist_obj["name"].toString() + ", ";
            songInfo.artists_id.append(artist_obj["id"].toInteger());
        }
        tmp_artists_str.chop(2);
        songInfo.artists_str = tmp_artists_str;

        songInfo.album_id = songObj["al"].toObject()["id"].toInteger();
        songInfo.album = songObj["al"].toObject()["name"].toString();
        songInfo.download_progress = 0;

        if(favourites.contains(songInfo.id.toLongLong())) {
            songInfo.is_favorite = true;
        } else {
            songInfo.is_favorite = false;
        }
        Append(songInfo);
    }

    ConfigSongTableView();
    Refreshed();
    ui->pageDownButton->setEnabled(false);
    ui->pageUpButton->setEnabled(false);
}

void MainWindow::DisplayAlbumPicture(QString album_pic_url) {
    QString file_name;
    if (album_pic_url.contains(QChar('?'))) {
        int i = album_pic_url.lastIndexOf(QChar('/'));
        int j = album_pic_url.lastIndexOf(QChar('?'));
        file_name = album_pic_url.sliced(i + 1, j - i - 1);
    } else {
        int i = album_pic_url.lastIndexOf(QChar('/'));
        file_name = album_pic_url.sliced(i + 1);
    }
    file_name = TEMP_PIC_DIR.absoluteFilePath(file_name);
    // // qDebug() << "[DisplayAlbumPicture]" << file_name;
    QPixmap pixmap(file_name);
    if (pixmap.isNull()) {
        // qDebug() << "Failed to load image.";
    }
    cover_pixmap = pixmap;
    ui->imageLabel->setPixmap(pixmap.scaled(ui->imageLabel->size()));
}

void MainWindow::RefreshSearchSong() {
    Refreshing();
    ui->comboBox->setCurrentIndex(0);

    currentModel = kSongModel;
    ui->tableView->setModel(song_model);
    song_model->clear();

    favourites = user.GetFavouriteIds(requester, favourite_id);

    QJsonDocument json = SongSearch(requester, ui->lineEdit->displayText(), offset, PAGE_SIZE);
    QJsonArray songs = json["result"]["songs"].toArray();

    for (QJsonValueRef song : songs) {
        SongInfo songInfo;
        QJsonObject songObj = song.toObject();

        songInfo.id = songObj["id"].toInteger();
        QString tmp_name;
        tmp_name = songObj["name"].toString();
        if (songObj["alias"] != QJsonValue::Undefined) {
            QJsonArray tns_name_list = songObj["alias"].toArray();
            if (tns_name_list.size() > 0 && tns_name_list[0] != QJsonValue::Undefined && tns_name_list[0].toString() != "") {
                tmp_name += "(" + tns_name_list[0].toString() + ")";
            }
        }
        songInfo.name = tmp_name;
        songInfo.duration = songObj["duration"].toInt();
        songInfo.duration_str = QString("%1:%2").arg(songInfo.duration.toInt() / 1000 / 60).arg((int)(songInfo.duration.toInt() / 1000 % 60), 2, 10, (QChar)u'0');
        QString tmp_artists_str;
        for (QJsonValueRef artist : songObj["artists"].toArray()) {
            QJsonObject artist_obj = artist.toObject();

            songInfo.artists.append(artist_obj["name"].toString());
            tmp_artists_str += artist_obj["name"].toString() + ", ";
            songInfo.artists_id.append(artist_obj["id"].toInteger());
        }
        tmp_artists_str.chop(2);
        songInfo.artists_str = tmp_artists_str;
        songInfo.album_id = songObj["album"].toObject()["id"].toInteger();
        songInfo.album = songObj["album"].toObject()["name"].toString();
        songInfo.download_progress = 0;
        if(favourites.contains(songInfo.id.toLongLong())) {
            songInfo.is_favorite = true;
        } else {
            songInfo.is_favorite = false;
        }
        Append(songInfo);
    }
    ConfigSongTableView();
    Refreshed();
    UpdateInfoVisibility(false);

    if (offset > 0) {
        ui->pageUpButton->setEnabled(true);
    } else {
        ui->pageUpButton->setEnabled(false);
    }

    int count = json["result"].toObject()["songCount"].toInt();
    if (count == 0) {
        ui->pageDownButton->setEnabled(false);
        return;
    }

    bool has_more = json["result"].toObject()["hasMore"].toBool();
    if (has_more) {
        ui->pageDownButton->setEnabled(true);
    } else {
        ui->pageDownButton->setEnabled(false);
    }
}

void MainWindow::RefreshSearchArtist() {
    Refreshing();
    ui->comboBox->setCurrentIndex(1);

    currentModel = kArtistModel;
    ui->tableView->setModel(artist_model);
    artist_model->clear();

    QJsonDocument json = search.SearchArtist(requester, ui->lineEdit->displayText(), offset, PAGE_SIZE);
    QJsonArray artists = json["result"]["artists"].toArray();

    for (QJsonValueRef artist : artists) {
        ArtistInfo artistInfo;
        QJsonObject artistObj = artist.toObject();
        artistInfo.id = artistObj["id"].toInteger();

        QString tmp_name;
        tmp_name = artistObj["name"].toString();
        if (artistObj["trans"] != QJsonValue::Undefined && artistObj["trans"].toString() != "") {
            tmp_name += "(" + artistObj["trans"].toString() + ")";
        }
        artistInfo.name = tmp_name;
        artistInfo.is_followed = artistObj["followed"].toBool();

        Append(artistInfo);

    }
    ConfigArtistTableView();
    Refreshed();
    UpdateInfoVisibility(false);
    if (offset > 0) {
        ui->pageUpButton->setEnabled(true);
    } else {
        ui->pageUpButton->setEnabled(false);
    }

    int count = json["result"].toObject()["artistCount"].toInt();
    if (count == 0) {
        ui->pageDownButton->setEnabled(false);
        return;
    }

    bool has_more = json["result"].toObject()["hasMore"].toBool();
    if (has_more) {
        ui->pageDownButton->setEnabled(true);
    } else {
        ui->pageDownButton->setEnabled(false);
    }
}

void MainWindow::RefreshSearchAlbum() {
    Refreshing();
    ui->comboBox->setCurrentIndex(2);

    currentModel = kAlbumModel;
    ui->tableView->setModel(album_model);
    album_model->clear();

    QJsonDocument json = search.SearchAlbum(requester, ui->lineEdit->displayText(), offset, PAGE_SIZE);
    QJsonArray albums = json["result"]["albums"].toArray();
    for (QJsonValueRef album : albums) {
        AlbumInfo albumInfo;
        QJsonValue albumObj = album;
        albumInfo.id = albumObj["id"].toInteger();
        albumInfo.artist_id = albumObj["artist"]["id"].toInteger();
        albumInfo.artist = albumObj["artist"]["name"].toString();
        if (albumObj["artist"]["alias"] != QJsonValue::Undefined && albumObj["artist"]["alias"].toArray().size() > 0) {
            albumInfo.name = albumObj["name"].toString() + "(" + albumObj["artist"]["alias"][0].toString() + ")";
        } else {
            albumInfo.name = albumObj["name"].toString();
        }
        albumInfo.is_like = albumObj["isSub"].toBool();
        Append(albumInfo);
    }
    ConfigAlbumTableView();
    Refreshed();
    UpdateInfoVisibility(false);
}

void MainWindow::RefreshMyFavouritePlaylist() {
    Refreshing();
    QJsonDocument json = user.GetFavouritePlayListJson(requester, favourite_id);
    QJsonArray favorite_songs;
    favorite_songs = json["playlist"].toObject()["tracks"].toArray();
    song_model->clear();
    RefreshPlaylist(favorite_songs);
    Refreshed();
    UpdateInfoVisibility(false);
    ui->pageUpButton->setEnabled(false);
    ui->pageDownButton->setEnabled(false);
}

void MainWindow::RefreshSearchUser() {
    Refreshing();
    ui->comboBox->setCurrentIndex(4);

    currentModel = kUserModel;
    ui->tableView->setModel(user_model);
    user_model->clear();

    QJsonDocument json = search.SearchUser(requester, ui->lineEdit->displayText(), offset, PAGE_SIZE);
    QJsonArray users = json["result"]["userprofiles"].toArray();

    for (QJsonValueRef user : users) {
        UserInfo userInfo;
        QJsonValue userObj = user;
        userInfo.id = userObj["userId"].toInteger();
        userInfo.name = userObj["nickname"].toString();
        userInfo.signature = userObj["signature"].toString();
        userInfo.description = userObj["description"].toString();
        userInfo.is_followed = userObj["followed"].toBool();
        userInfo.gender = userObj["gender"].toInt();

        Append(userInfo);

    }
    ConfigUserTableView();
    Refreshed();
    UpdateInfoVisibility(false);
    if (offset > 0) {
        ui->pageUpButton->setEnabled(true);
    } else {
        ui->pageUpButton->setEnabled(false);
    }

    int count = json["result"].toObject()["userprofileCount"].toInt();
    if (count == 0) {
        ui->pageDownButton->setEnabled(false);
        return;
    }

    bool has_more = json["result"].toObject()["hasMore"].toBool();
    if (has_more) {
        ui->pageDownButton->setEnabled(true);
    } else {
        ui->pageDownButton->setEnabled(false);
    }
}

void MainWindow::RefreshMyFollowedUser() {
    Refreshing();

    currentModel = kFollowedModel;
    ui->tableView->setModel(followed_model);
    followed_model->clear();

    QJsonDocument json = user.GetMyFollowedUser(requester);
    ui->pageUpButton->setEnabled(false);
    ui->pageDownButton->setEnabled(false);

    QJsonArray users = json["data"]["records"].toArray();

    for (QJsonValueRef user : users) {
        FollowedInfo userInfo;

        QJsonValue userObj = user.toObject();
        userInfo.type = userObj["type"].toInt();
        userInfo.follow_day = userObj["followDay"];
        // // qDebug() << userInfo.type;
        if (userInfo.type == 1) {
            userInfo.id = userObj["userProfile"]["userId"].toInteger();
            userInfo.name = userObj["userProfile"]["nickname"].toString();
        } else if (userInfo.type == 2 or userInfo.type == 3){
            userInfo.id = userObj["artistInfo"]["id"].toInteger();
            userInfo.name = userObj["artistInfo"]["name"].toString();
            if (userObj["artistInfo"]["trans"] != QJsonValue::Undefined and userObj["artistInfo"]["trans"].toString() != "") {
                userInfo.name = userInfo.name.toString() + "(" + userObj["artistInfo"]["trans"].toString() + ")";
            }
        }
        if (userInfo.type == 1 or userInfo.type == 3) {
            userInfo.gender = userObj["userProfile"]["gender"].toInteger();
        } else {
            userInfo.gender = 0;
        }

        Append(userInfo);
    }
    ConfigFollowedTableView();
    Refreshed();
    UpdateInfoVisibility(false);
    ui->pageUpButton->setEnabled(false);
    ui->pageDownButton->setEnabled(false);
}

void MainWindow::RefreshSearchPlaylist() {
    Refreshing();
    ui->comboBox->setCurrentIndex(3);

    currentModel = kPlaylistModel;
    ui->tableView->setModel(playlist_model);
    playlist_model->clear();

    QJsonDocument json = search.SearchPlaylist(requester, ui->lineEdit->displayText(), offset, PAGE_SIZE);
    QJsonArray playlists = json["result"]["playlists"].toArray();

    for (QJsonValueRef playlist : playlists) {
        PlaylistInfo playlistInfo;
        QJsonValue playlistObj = playlist;
        playlistInfo.id = playlistObj["id"].toInteger();
        playlistInfo.name = playlistObj["name"].toString();
        playlistInfo.creator = playlistObj["creator"]["nickname"].toString();
        playlistInfo.creator_id = playlistObj["creator"]["userId"].toInteger();
        playlistInfo.description = playlistObj["description"].toString();
        playlistInfo.is_like = playlistObj["subscribed"].toBool();
        playlistInfo.trackCount = playlistObj["trackCount"].toInt();


        Append(playlistInfo);
    }
    ConfigPlaylistTableView();
    Refreshed();
    UpdateInfoVisibility(false);
    if (offset > 0) {
        ui->pageUpButton->setEnabled(true);
    } else {
        ui->pageUpButton->setEnabled(false);
    }

    int count = json["result"]["playlistCount"].toInt();
    if (count == 0) {
        ui->pageDownButton->setEnabled(false);
        return;
    }

    bool has_more = json["result"]["hasMore"].toBool();
    // // qDebug() << has_more;
    if (has_more) {
        ui->pageDownButton->setEnabled(true);
    } else {
        ui->pageDownButton->setEnabled(false);
    }
}

void MainWindow::ComplexSearch() {
    isPageButtonEnabled = true;

    QString currentType = ui->comboBox->currentText();
    QString keyword = ui->lineEdit->displayText().normalized(QString::NormalizationForm_KC); // 规范化字符;
    static const QRegularExpression regex("[\\p{So}\\p{Sk}]"); // 移除符号 (So) 和修饰符 (Sk)
    keyword = keyword.remove(regex);
    // keyword = keyword.remove(QChar(0x2764));  // 移除 `U+2764` (♥)
    // keyword = keyword.remove(QChar(0xFE0F));  // 移除 `U+FE0F` (变体选择符)
    // keyword = keyword.remove(QChar(0x2757));  // 移除 `U+2757` (❗)
    ui->lineEdit->setText(keyword);

    if (currentType == "歌曲") {
        undoStack.push(Command{"Refresh", "SearchSong", {keyword}});
        RefreshSearchSong();
    } else if (currentType == "歌手") {
        undoStack.push(Command{"Refresh", "SearchArtist", {keyword}});
        RefreshSearchArtist();
    } else if (currentType == "专辑") {
        undoStack.push(Command{"Refresh", "SearchAlbum", {keyword}});
        RefreshSearchAlbum();
    } else if (currentType == "用户") {
        undoStack.push(Command{"Refresh", "SearchUser", {keyword}});
        RefreshSearchUser();
    } else if (currentType == "歌单") {
        undoStack.push(Command{"Refresh", "SearchPlaylist", {keyword}});
        RefreshSearchPlaylist();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (is_ending) {
        event->accept();
        return;
    }
    QMessageBox::StandardButton choice = QMessageBox::warning(this, "", "确定要退出主程序吗？", QMessageBox::Yes | QMessageBox::No);
    if (choice == QMessageBox::Yes) {
        event->accept();
    } else {
        event->ignore();
    }
}

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("设置");
    // resize(QSize{650, 150});
    setMinimumWidth(650);

    layout = new QGridLayout;

    tempDirLabel = new QLabel("缓存路径", this);
    tempDirLabel->adjustSize();
    layout->addWidget(tempDirLabel, 0, 0);

    tempDirEdit = new QLineEdit(this);
    // tempDirEdit->setEnabled(false);
    tempDirEdit->setText(TEMP_DIR.absolutePath());
    layout->addWidget(tempDirEdit, 0, 1);

    tempDirButton = new QPushButton("...", this);
    tempDirButton->adjustSize();
    QObject::connect(tempDirButton, &QPushButton::clicked, this, [&]() {
        QString path = QFileDialog::getExistingDirectory(this, "临时文件目录");
        if (not path.isEmpty()) {
            tempDirEdit->setText(path);
        }

    });
    layout->addWidget(tempDirButton, 0, 2);

    openCacheButton = new QPushButton("打开", this);
    QObject::connect(openCacheButton, &QPushButton::clicked, this, [&]() {
        QString path = tempDirEdit->text();
        QUrl url = QUrl::fromLocalFile(path);
        QDesktopServices::openUrl(url);
    });
    layout->addWidget(openCacheButton, 0, 3);

    downloadDirLabel = new QLabel("下载路径", this);
    downloadDirLabel->adjustSize();
    layout->addWidget(downloadDirLabel, 1, 0);

    downloadDirEdit = new QLineEdit(this);
    // downloadDirEdit->setEnabled(false);
    downloadDirEdit->setText(DOWNLOAD_DIR.absolutePath());
    layout->addWidget(downloadDirEdit, 1, 1);

    downloadDirButton = new QPushButton("...", this);
    QObject::connect(downloadDirButton, &QPushButton::clicked, this, [&]() {
        QString path = QFileDialog::getExistingDirectory(this, "下载文件目录");
        if (not path.isEmpty()) {
            downloadDirEdit->setText(path);
        }
    });
    downloadDirButton->adjustSize();
    layout->addWidget(downloadDirButton, 1, 2);

    openDownloadButton = new QPushButton("打开", this);
    QObject::connect(openDownloadButton, &QPushButton::clicked, this, [&]() {
        QString path = downloadDirEdit->text();
        QUrl url = QUrl::fromLocalFile(path);
        QDesktopServices::openUrl(url);
    });
    layout->addWidget(openDownloadButton, 1, 3);

    homePageblocksOrderLabel = new QLabel("主页显示", this);
    homePageblocksOrderLabel->adjustSize();
    layout->addWidget(homePageblocksOrderLabel, 2, 0);

    checkBoxLabelsAndJsonValue = {{"我的歌单", "PAGE_RECOMMEND_MY_SHEET"},
                                  // {"专属推荐", "PAGE_RECOMMEND_COMBINATION"},
                                  {"私人推荐", "PAGE_RECOMMEND_PRIVATE_RCMD_SONG"},
                                  {"雷达歌单", "PAGE_RECOMMEND_RADAR"},
                                  {"氛围歌单", "PAGE_RECOMMEND_FEELING_PLAYLIST_LOCATION"},
                                  {"场景歌单", "PAGE_RECOMMEND_SCENE_PLAYLIST_LOCATION"},
                                  {"排行榜", "PAGE_RECOMMEND_RANK"},
                                  {"艺人最新动向", "PAGE_RECOMMEND_ARTIST_TREND"},
                                  {"根据你的听歌风格推荐", "PAGE_RECOMMEND_STYLE_PLAYLIST_1"},
                                  {"推荐歌单", "PAGE_RECOMMEND_SPECIAL_CLOUD_VILLAGE_PLAYLIST"},
                                  {"影视原声音乐", "PAGE_RECOMMEND_FIRM_PLAYLIST"},
                                  {"每周新热趋势", "PAGE_RECOMMEND_NEW_SONG_AND_ALBUM"},
                                  {"原创歌曲", "PAGE_RECOMMEND_SPECIAL_ORIGIN_SONG_LOCATION"},
                                  // {"回忆歌单", "PAGE_RECOMMEND_MONTH_YEAR_PLAYLIST"},
                                  {"地方特色", "PAGE_RECOMMEND_LBS"},
                                  {"根据你喜爱的歌曲推荐", "PAGE_RECOMMEND_RED_SIMILAR_SONG"},
                                  };
    checkBoxLayout = new QGridLayout;
    static const int ROWSIZE = 7;
    int cnt = 0;
    for (auto [label, jsonValue] : checkBoxLabelsAndJsonValue) {
        auto *checkBox = new QCheckBox(label, this);
        checkBox->setProperty("jsonValue", jsonValue);
        checkBox->adjustSize();
        if (HOMEPAGE_BLOCKS_ORDER_LIST.contains(jsonValue)) {
            checkBox->setChecked(true);
        }
        checkBoxLayout->addWidget(checkBox, cnt / ROWSIZE, cnt % ROWSIZE);
        cnt++;
        checkBoxes.append(checkBox);
    }
    layout->addLayout(checkBoxLayout, 2, 1);

    downloadQualityLabel = new QLabel("下载音质", this);
    layout->addWidget(downloadQualityLabel, 3, 0);
    downloadQualityCombo = new QComboBox(this);
    downloadQualityCombo->addItem("标准");
    downloadQualityCombo->addItem("高品");
    downloadQualityCombo->addItem("最佳");
// downloadQualityCombo set default quality
    if (quality == kStandard) {
        downloadQualityCombo->setCurrentIndex(0);
    } else if (quality == kHigher) {
        downloadQualityCombo->setCurrentIndex(1);
    } else {
        downloadQualityCombo->setCurrentIndex(2);
    }
    layout->addWidget(downloadQualityCombo, 3, 1);

    clearCacheButton = new QPushButton("清除缓存", this);
    clearCacheButton->adjustSize();
    clearCacheButton->setStyleSheet("color: red;");
    connect(clearCacheButton, &QPushButton::clicked, this, [&]() {
        QDir(TEMP_DIR).removeRecursively();
        QMessageBox::information(this, "", "缓存已经清除！");
    });
    layout->addWidget(clearCacheButton, 4, 0);

    okButton = new QPushButton("确定", this);
    okButton->adjustSize();
    connect(okButton, &QPushButton::clicked, this, [&]() {
        bool ok = false;
        for (QCheckBox *checkBox : checkBoxes) {
            if (checkBox->isChecked()) {
                ok = true;
                break;
            }
        }
        if (not ok) {
            QMessageBox::warning(this, "警告", "主页不能为空！");
        } else {
            accept();
        }
    });
    layout->addWidget(okButton, 4, 2, Qt::AlignRight);

    cancelButton = new QPushButton("取消", this);
    cancelButton->adjustSize();
    connect(cancelButton, &QPushButton::clicked, this, &SettingsDialog::reject);
    layout->addWidget(cancelButton, 4, 3);

    setLayout(layout);
}

QString SettingsDialog::getDownloadDir() {
    return downloadDirEdit->text();
}

QString SettingsDialog::getTempDir() {
    return tempDirEdit->text();
}

QJsonArray SettingsDialog::getBlockCodeOrderList() {
    QJsonArray arr;
    for (QCheckBox *checkBox : checkBoxes) {
        if (checkBox->isChecked()) {
            QString jsonV = checkBox->property("jsonValue").toString();
            arr.append(jsonV);
        }
    }
    return arr;
}

Level SettingsDialog::getDownloadQuality() {
    int curIdx = downloadQualityCombo->currentIndex();
    if (curIdx == 0) {
        return kStandard;
    } else if (curIdx == 1) {
        return kHigher;
    } else {
        return kBest;
    }
}


CommentsDialog::CommentsDialog(QWidget *parent) : QDialog(parent) {
    resize(QSize{800, 600});

    commentsModel = new QStandardItemModel(this);

    layout = new QVBoxLayout;

    title = new QLabel(this);
    title->setText("");
    layout->addWidget(title);

    commentView = new QTableView(this);
    commentView->setWordWrap(true);
    commentView->setColumnWidth(2, 50);
    commentView->setColumnWidth(4, 500);
    commentView->setModel(commentsModel);
    layout->addWidget(commentView);
    layout->setStretch(1, 2);

    pageButtonLayout = new QHBoxLayout;
    pageDownButton = new QPushButton(this);
    pageDownButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoDown));
    pageButtonLayout->addWidget(pageDownButton);
    layout->addLayout(pageButtonLayout);

    quitButton = new QPushButton("关闭", this);
    quitButton->adjustSize();
    layout->addWidget(quitButton);

    setLayout(layout);


    mainWindow = static_cast<MainWindow*>(parent);


    QObject::connect(quitButton, &QPushButton::clicked, this, &QDialog::hide);
    if (pageDownButtonConn != QMetaObject::Connection()) {
        QObject::disconnect(pageDownButtonConn);
    }
    pageDownButtonConn = QObject::connect(pageDownButton, &QPushButton::clicked, this, [&]() {
        refreshComments(m_requester, 0, "" ,m_type, m_cursor);
    });

    QObject::connect(commentView, &QTableView::doubleClicked, this, [&](const QModelIndex &index) {
        int row = index.row();
        QModelIndex idx = commentsModel->index(row, 0);
        qint64 commentId = commentsModel->data(idx, Qt::EditRole).toLongLong();
        idx = commentsModel->index(row, 1);
        qint64 userId = commentsModel->data(idx, Qt::EditRole).toLongLong();
        idx = commentsModel->index(row, 3);
        qint64 songId = commentsModel->data(idx, Qt::EditRole).toLongLong();

        if (index.column() == 2) {
            mainWindow->undoStack.push(Command{"Refresh", "UserDetail", {userId}});
            mainWindow->RefreshUserDetail(userId);
        } else {
            CommentFloorDialog commentFloorDialog(this);
            commentFloorDialog.getCommentFloor(m_requester, songId, commentId);
            commentFloorDialog.exec();
        }
        // CommentFloorDialog commentFloorDialog(this);
        // commentFloorDialog.getCommentFloor(m_requester, songId, commentId);
        // commentFloorDialog.exec();
    });
}

void CommentsDialog::setCommentModel(QStandardItemModel *model) {
}

void CommentsDialog::refreshComments(ApiRequester *requester, qint64 song_id, QString songInfo ,int sortType, QString cursor, int pageSize) {
    if (requester != nullptr) {
        m_requester = requester;
    }
    if (song_id != 0) {
        m_songId = song_id;
    }
    if (sortType == 0) {
        m_cursor = "0";
        m_page = 1;
        m_type = 0;
    } else {
        m_cursor = cursor;
        m_page = 2;
    }
    if (songInfo != "") {
        m_songInfo = songInfo;
    }
    refreshing();
    QJsonValue json;
    QString text = R"({"\/api\/v2\/resource\/comments":"{\"cursor\":\"%1\",\"pageNo\":\"%2\",\"scene\":\"SONG_COMMENT\",\"pageSize\":\"%3\",\"preloadExpGroupName\":\"t1\",\"sortType\":\"%4\",\"showInner\":\"0\",\"threadId\":\"R_SO_4_%5\"}","os":"iOS","verifyId":1,"e_r":true})";
    text = text.arg(m_cursor).arg(m_page).arg(pageSize).arg(m_type).arg(m_songId);
    // qDebug() << text;
    json = m_requester->ApiRequest(text, "https://interface3.music.163.com/eapi/batch", "/batch")["/api/v2/resource/comments"];


    if (json["code"] != 200) {
        qDebug() << "refreshComments err!";
        return;
    }

    bool hasMore = json["data"]["hasMore"].toBool();
    if (hasMore) {
        pageDownButton->show();
    } else {
        pageDownButton->hide();
    }

    // // qDebug() << "totalcount" <<  json["data"]["totalCount"].toInt() << "cursor" << json["data"]["cursor"] << "hasMore" << json["data"]["hasMore"];
    m_cursor = json["data"]["cursor"].toString();
    m_type = json["data"]["sortType"].toInt();

    for (auto val : json["data"]["comments"].toArray()) {
        QJsonValue comment = val;

        CommentInfo commentInfo;
        commentInfo.id = comment["commentId"].toInteger();
        commentInfo.userId = comment["user"]["userId"].toInteger();
        commentInfo.userName = comment["user"]["nickname"].toString();
        commentInfo.songId = m_songId;
        commentInfo.content = comment["content"].toString();
        commentInfo.likedCount = comment["likedCount"].toInt();
        commentInfo.replyCount = comment["replyCount"].toInt();
        commentInfo.floorRefreshCount = 20;
        commentInfo.songInfo = songInfo;

        append(commentInfo);
    }

    title->setText(songInfo);
    // commentsDialog->setCommentModel(comments_model);
    configCommentView();

    refreshed();
}

void CommentsDialog::refreshing() {
    MainWindow *mainWindow = static_cast<MainWindow*>(parent());
    pageDownButton->setEnabled(false);
}

void CommentsDialog::refreshed() {
    MainWindow *mainWindow = static_cast<MainWindow*>(parent());
    pageDownButton->setEnabled(true);
}

void CommentsDialog::append(CommentInfo info) {
    QList <QStandardItem*> row_items(7);

    row_items[0] = new QStandardItem;
    row_items[0]->setData(info.id, Qt::EditRole);

    row_items[1] = new QStandardItem;
    row_items[1]->setData(info.userId, Qt::EditRole);

    row_items[2] = new QStandardItem;
    row_items[2]->setData(info.userName, Qt::EditRole);

    row_items[3] = new QStandardItem;
    row_items[3]->setData(info.songId, Qt::EditRole);

    row_items[4] = new QStandardItem;
    row_items[4]->setData(info.content, Qt::EditRole);

    row_items[5] = new QStandardItem;
    row_items[5]->setData(info.likedCount, Qt::EditRole);

    row_items[6] = new QStandardItem;
    row_items[6]->setData(info.replyCount, Qt::EditRole);

    commentsModel->appendRow(row_items);

}

void CommentsDialog::configCommentView() {
    QStandardItemModel *model = static_cast<QStandardItemModel*>(commentView->model());
    model->setHorizontalHeaderLabels({"评论ID", "用户ID", "用户名", "歌曲ID", "评论", "点赞数", "回复数",});

    commentView->setColumnHidden(0, true);
    commentView->setColumnHidden(1, true);
    commentView->setColumnHidden(2, false);
    commentView->setColumnHidden(3, true);
    commentView->setColumnHidden(4, false);
    commentView->setColumnHidden(5, false);
    commentView->setColumnHidden(6, false);

    commentView->setColumnWidth(2, 50);
    commentView->setColumnWidth(4, 500);

    commentView->setWordWrap(true);
    commentView->setEditTriggers(QTableView::NoEditTriggers);
}

void CommentsDialog::clearComments() {
    commentsModel->clear();
    m_cursor = "0";
}



CommentFloorDialog::CommentFloorDialog(QWidget *parent) : QDialog(parent) {
    resize(QSize{800, 600});
    m_pageNo = 0;

    commentFloorModel = new QStandardItemModel(this);

    layout = new QVBoxLayout(this);

    parentCommentLabel = new QLabel(this);
    parentCommentLabel->setText("-");
    parentCommentLabel->setWordWrap(true);
    layout->addWidget(parentCommentLabel);

    commentFloorView = new QTableView(this);
    commentFloorView->setWordWrap(true);
    commentFloorView->setColumnWidth(2, 50);
    commentFloorView->setColumnWidth(3, 500);
    commentFloorView->setModel(commentFloorModel);
    layout->addWidget(commentFloorView);
    layout->setStretch(2, 2);

    pageDownButton = new QPushButton(this);
    pageDownButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoDown));
    layout->addWidget(pageDownButton);

    quitButton = new QPushButton("关闭", this);
    quitButton->adjustSize();
    layout->addWidget(quitButton);

    setLayout(layout);


    mainWindow = static_cast<CommentsDialog*>(parent)->mainWindow;

    QObject::connect(quitButton, &QPushButton::clicked, this, &QDialog::accept);
    QObject::connect(pageDownButton, &QPushButton::clicked, this, [&]() {
        getCommentFloor(nullptr, 0, 0, m_time, m_cursor);
        m_pageNo++;
    });
    QObject::connect(commentFloorView, &QTableView::doubleClicked, this, [&](const QModelIndex &index) {
        int row = index.row();
        if (index.column() == 2) {
            QModelIndex idx = commentFloorModel->index(row, 1);
            qint64 userId = commentFloorModel->data(idx, Qt::EditRole).toLongLong();
            mainWindow->undoStack.push(Command{"Refresh", "UserDetail", {userId}});
            mainWindow->RefreshUserDetail(userId);
        }
    });
}

void CommentFloorDialog::configCommentFloorView() {
    QStandardItemModel *model = static_cast<QStandardItemModel*>(commentFloorView->model());
    model->setHorizontalHeaderLabels({"评论ID", "用户ID", "用户名", "歌曲ID", "评论", "点赞数", "回复数",});

    commentFloorView->setColumnHidden(0, true);
    commentFloorView->setColumnHidden(1, true);
    commentFloorView->setColumnHidden(2, false);
    commentFloorView->setColumnHidden(3, true);
    commentFloorView->setColumnHidden(4, false);
    commentFloorView->setColumnHidden(5, false);
    commentFloorView->setColumnHidden(6, false);

    commentFloorView->setColumnWidth(2, 50);
    commentFloorView->setColumnWidth(4, 500);

    commentFloorView->setWordWrap(true);
    commentFloorView->setEditTriggers(QTableView::NoEditTriggers);
}

void CommentFloorDialog::append(CommentInfo info) {
    QList <QStandardItem*> row_items(7);

    row_items[0] = new QStandardItem;
    row_items[0]->setData(info.id, Qt::EditRole);

    row_items[1] = new QStandardItem;
    row_items[1]->setData(info.userId, Qt::EditRole);

    row_items[2] = new QStandardItem;
    row_items[2]->setData(info.userName, Qt::EditRole);

    row_items[3] = new QStandardItem;
    row_items[3]->setData(info.songId, Qt::EditRole);

    row_items[4] = new QStandardItem;
    row_items[4]->setData(info.content, Qt::EditRole);

    row_items[5] = new QStandardItem;
    row_items[5]->setData(info.likedCount, Qt::EditRole);

    row_items[6] = new QStandardItem;
    row_items[6]->setData(info.replyCount, Qt::EditRole);

    commentFloorModel->appendRow(row_items);
}

void CommentFloorDialog::getCommentFloor(ApiRequester *requester, qint64 songId, qint64 parentCommentId, qint64 time, QString cursor, int limit) {
    if (requester != nullptr) {
        m_requester = requester;
    }
    if (songId != 0) {
        m_songId = songId;
    }
    if (cursor == "") {
        m_cursor = "";
        m_time = time;
    } else {
        m_cursor = cursor;
        m_time = -1;
    }
    if (parentCommentId != 0) {
        m_parentCommentId = parentCommentId;
    }
    refreshing();
    QJsonDocument json;
    QString text = R"({"e_r":true,"limit":%1,"order":0,"os":"iOS","parentCommentId":"%2","scene":"SONG_COMMENT","source":"","threadId":"R_SO_4_%3","time":%4,"verifyId":1, "cursor":"%5"})";
    text = text.arg(limit).arg(m_parentCommentId).arg(m_songId).arg(m_time).arg(m_cursor.replace("\"", "\\\""));
    // qDebug() << text;
    json = m_requester->ApiRequest(text, "https://interface3.music.163.com/eapi/v2/resource/comment/floor/get", "/api/v2/resource/comment/floor/get");
    if (json["code"].toInt()/100 != 2) {
        return;
    }
    refreshing();
    parentCommentLabel->setText(json["data"]["ownerComment"]["content"].toString());
    // parentCommentLabel->adjustSize();
    m_time = json["data"]["time"].toInteger();
    m_cursor = json["data"]["cursor"].toString();
    // qDebug() << "m_time" << m_time;
    bool hasMore = json["data"]["hasMore"].toBool();
    if ((m_cursor == "" and m_time == 9223372036854775807ll) or !hasMore) {
        pageDownButton->hide();
    } else {
        pageDownButton->show();
    }

    for (auto val : json["data"]["comments"].toArray()) {
        QJsonValue comment = val;

        CommentInfo commentInfo;
        commentInfo.id = comment["commentId"].toInteger();
        commentInfo.userId = comment["user"]["userId"].toInteger();
        commentInfo.userName = comment["user"]["nickname"].toString();
        commentInfo.songId = songId;
        commentInfo.content = comment["content"].toString();
        commentInfo.likedCount = comment["likedCount"].toInt();
        commentInfo.replyCount = comment["replyCount"].toInt();
        append(commentInfo);
    }

    // commentsDialog->setCommentModel(comments_model);
    configCommentFloorView();

    refreshed();
}

void CommentFloorDialog::refreshing() {
    pageDownButton->setEnabled(false);

}

void CommentFloorDialog::refreshed() {
    pageDownButton->setEnabled(true);
}


AddToPlaylistDialog::AddToPlaylistDialog(QWidget *parent) : QDialog(parent) {
    layout = new QGridLayout(this);

    comboLabel = new QLabel("请选择歌单", this);
    layout->addWidget(comboLabel, 0, 0);

    playlistCombo = new QComboBox(this);
    playlistCombo->addItem("新建歌单...", 0ll);
    playlistCombo->insertSeparator(1);
    layout->addWidget(playlistCombo, 0, 1);

    newPlaylistNameLabel = new QLabel("歌单名称", this);
    layout->addWidget(newPlaylistNameLabel, 1, 0);

    newPlaylistNameEdit = new QLineEdit(this);
    layout->addWidget(newPlaylistNameEdit, 1, 1);

    isPrivatePlaylistCheck = new QCheckBox("设为私密歌单", this);
    isPrivatePlaylistCheck->adjustSize();
    layout->addWidget(isPrivatePlaylistCheck, 2, 1);

    cancelButton = new QPushButton("取消", this);
    layout->addWidget(cancelButton, 3, 0);

    okButton = new QPushButton("确定", this);
    layout->addWidget(okButton, 3, 1);

    QObject::connect(playlistCombo, &QComboBox::currentIndexChanged, this, [&](int curIndex) {
        if (curIndex == 0) {
            newPlaylistNameLabel->show();
            newPlaylistNameEdit->show();
            isPrivatePlaylistCheck->show();
        } else {
            newPlaylistNameLabel->hide();
            newPlaylistNameEdit->hide();
            isPrivatePlaylistCheck->hide();
        }
    });
    QObject::connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    QObject::connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    setLayout(layout);
}

void AddToPlaylistDialog::showMyPlaylist(ApiRequester *requester, qint64 userId, qint64 trackId) {
    Playlist playlist(this);
    playlistCombo->clear();
    playlistCombo->addItem("新建歌单...", 0ll);
    playlistCombo->insertSeparator(1);

    int offset = 0;
    QJsonDocument json;
    do {
        json = playlist.RequestTrackStatus(requester, userId, trackId, offset++);
        for (QJsonValueRef playlistR : json["playlist"].toArray()) {
            QJsonValue my_playlist = playlistR;
            playlistCombo->addItem(my_playlist["name"].toString() + (my_playlist["containsTracks"].toBool() ? "(已添加)": ""), my_playlist["id"].toInteger());
        }
    } while (json["more"].toBool());
}

QString AddToPlaylistDialog::getNewPlaylistname() {
    return newPlaylistNameEdit->text();
}

qint64 AddToPlaylistDialog::getTargetPlaylistId() {
    int curIndex = playlistCombo->currentIndex();
    return playlistCombo->itemData(curIndex).toLongLong();
}

bool AddToPlaylistDialog::isPrivatePlaylist() {
    return isPrivatePlaylistCheck->isChecked();
}



UndoStack::UndoStack(MainWindow *parent, int limit) : limit(limit + 1), parent(parent), QObject((QObject*)parent) {
    push(Command{"RESERVED", "Homepage", {}});
}

void UndoStack::setLimit(int limit) {
    this->limit = limit + 1;
}

void UndoStack::push(Command command) {
    if (stk.size() >= limit) {
        stk.pop_front();
        stk.push_front(Command{"RESERVED", "Homepage", {}});
    }
    stk.push_back(command);
}

void UndoStack::pop() {
    if (stk.size() > 0) {
        stk.pop_back();
    }
}

Command UndoStack::top() {
    if (stk.size() > 0) {
        return stk.back();
    }  else {
        return Command{};
    }
}

void UndoStack::clearAll() {
    stk.clear();
    push(Command{"RESERVED", "Homepage", {}});
}

void UndoStack::undo(bool unused) {
    Q_UNUSED(unused);
    Command cmd = top();
    if (cmd.verb == "RESERVED") {
        if (cmd.type == "Homepage") {
            parent->ui->tableView->hide();
            parent->ui->imageLabel->hide();
            parent->ui->descriptionLabel->hide();
            parent->ui->downloadCoverPicButton->hide();
            parent->ui->descriptionScrollArea->hide();
            parent->ui->creator_button->hide();
            parent->ui->rcmd_button->hide();
            parent->ui->sub_button->hide();
            parent->ui->gridLayout->removeWidget(parent->ui->tableView);

            parent->currentWidget = parent->kHomePage;

            parent->ui->gridLayout->addWidget(parent->homepageWidget, 1, 0, 3, 6);
            parent->homepageWidget->show();
        }
        return;
    }
    pop();

    cmd = top();
    if (cmd.verb == "RESERVED") {
        if (cmd.type == "Homepage") {
            parent->ui->tableView->hide();
            parent->ui->imageLabel->hide();
            parent->ui->descriptionLabel->hide();
            parent->ui->downloadCoverPicButton->hide();
            parent->ui->descriptionScrollArea->hide();
            parent->ui->creator_button->hide();
            parent->ui->rcmd_button->hide();
            parent->ui->sub_button->hide();
            parent->ui->gridLayout->removeWidget(parent->ui->tableView);

            parent->currentWidget = parent->kHomePage;

            parent->ui->gridLayout->addWidget(parent->homepageWidget, 1, 0, 3, 6);
            parent->homepageWidget->show();
        }
        return;
    }

    if (cmd.verb == "Refresh") {
        bool flg = false;

        static const QList<QPair<QString,  void(MainWindow::*)()>>
            funcMap = {{"SearchSong", &MainWindow::RefreshSearchSong},
                {"SearchArtist", &MainWindow::RefreshSearchArtist},
                {"SearchAlbum", &MainWindow::RefreshSearchAlbum},
                {"SearchUser", &MainWindow::RefreshSearchUser},
                {"SearchPlaylist", &MainWindow::RefreshSearchPlaylist},

                {"DailyRecommend", &MainWindow::RefreshDailyRecommend},
                {"MyFavouritePlaylist", &MainWindow::RefreshMyFavouritePlaylist},
                {"MyFollowedUser", &MainWindow::RefreshMyFollowedUser},
                {"MyRcmdUser", &MainWindow::RefreshMyRcmdUser},
                {"MyPlaylist", &MainWindow::RefreshMyPlaylist}};
        for (auto& [type, func] : funcMap) {
            if (type == cmd.type) {
                if (cmd.args.size() > 0) {
                    QString keyword = cmd.args[0].toString();
                    // parent->ui->lineEdit->setText(keyword);
                }
                (parent->*func)();
                flg = true;
                break;
            }
        }

        if (flg) {
            return;
        }
        /*
         *                undoStack.push(Command{"Refresh", "ArtistAlbum", {artistId}});
                RefreshArtistAlbum(artistId);
*/

        static const QList<QPair<QString,  void(MainWindow::*)(qint64)>>
            funcMap2 = {{"AlbumDetail", &MainWindow::RefreshAlbumDetail},
                {"ArtistDetail", &MainWindow::RefreshArtistDetail},
                {"UserDetail", &MainWindow::RefreshUserDetail},
                {"PlaylistDetail", &MainWindow::RefreshPlaylistDetail},
                {"RecommendPlaylist", &MainWindow::RefreshRcmdPlaylist},
                {"RecommendArtist", &MainWindow::RefreshRcmdArtist},
                {"ArtistAlbum", &MainWindow::RefreshArtistAlbum},
                {"UserFollowedArtist", &MainWindow::RefreshUserFollowedArtist},
                {"UserFollowedUser", &MainWindow::RefreshFollowedUser},};
        for (auto& [type, func] : funcMap2) {
            if (type == cmd.type) {
                qint64 id = cmd.args[0].toLongLong();
                (parent->*func)(id);
                flg = true;
                break;
            }
        }

        if (flg) {
            return;
        }

        static const QList<QPair<QString,  void(MainWindow::*)(QJsonDocument)>>
            funcMap3 = {{"UserPlaylists", &MainWindow::RefreshUserPlaylists}};
        for (auto& [type, func] : funcMap3) {
            if (type == cmd.type) {
                QJsonDocument json = cmd.args[0].toJsonDocument();
                (parent->*func)(json);
                flg = true;
                break;
            }
        }

        if (flg) {
            return;
        }

        static const QList<QPair<QString,  void(MainWindow::*)(QList<QVariant>)>>
            funcMap4 = {{"MydefPlaylist", &MainWindow::RefreshMydefPlaylist}};
        for (auto& [type, func] : funcMap4) {
            if (type == cmd.type) {
                QList<QVariant> list = cmd.args[0].toList();
                (parent->*func)(list);
                flg = true;
                break;
            }
        }

        if (flg) {
            return;
        }

        if (cmd.type == "Homepage") {
            parent->ui->tableView->hide();
            parent->ui->imageLabel->hide();
            parent->ui->descriptionLabel->hide();
            parent->ui->downloadCoverPicButton->hide();
            parent->ui->descriptionScrollArea->hide();
            parent->ui->creator_button->hide();
            parent->ui->rcmd_button->hide();
            parent->ui->sub_button->hide();
            parent->ui->gridLayout->removeWidget(parent->ui->tableView);

            parent->currentWidget = parent->kHomePage;

            parent->ui->gridLayout->addWidget(parent->homepageWidget, 1, 0, 3, 6);
            parent->homepageWidget->show();
        }

    }
}

void UndoStack::redo(bool unused) {
    Q_UNUSED(unused);

}
