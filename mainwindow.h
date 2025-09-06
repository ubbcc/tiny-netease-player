#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "common.h"
#include "ui_mainwindow.h"
// #include "songtableheaderview.h"
#include "api/Comment.h"
#include "api/Crypto.h"
#include "api/SongSearch.h"
#include "api/User.h"
#include "api/Request.h"
#include "api/Download.h"
#include "api/Search.h"
#include "api/User.h"
#include "api/Playlist.h"
#include "api/Lyric.h"
#include "LoginWidget.h"

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkCookie>
#include <QtNetwork/QNetworkCookieJar>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonValue>

#include <QtGui/QStandardItemModel>
#include <QtCore/QAbstractProxyModel>
#include <QtCore/QSortFilterProxyModel>
#include <QtWidgets/QTableView>
#include <QStyledItemDelegate>
#include <QCompleter>
#include <QGroupBox>
#include <QTextEdit>
#include <QGridLayout>
#include <QScrollArea>
#include <QVBoxLayout>

#include <QSize>
#include <QMenu>
#include <QAction>
#include <QPixmap>
#include <QTimer>
#include <QEventLoop>
#include <QMouseEvent>
#include <QHash>

#include <QImageReader>
#include <QFileInfo>

#include <QDesktopServices>
#include <QFileDialog>

#include <QSettings>
#include <QCheckBox>
#include <QGroupBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct SongInfo {
    QVariant id;                // int
    QVariant album_id;          // int
    QList <QVariant> artists_id;// QList <int>
    QVariant name;              //  QString
    QVariant duration_str;      // QString
    QVariant duration;          // int
    QList <QVariant> artists;   // QList <QString>
    QVariant artists_str;       // QString
    QVariant album;             // QString
    QVariant download_progress;     // double (0.0 ~ 1.0)
    QVariant is_favorite;       // bool
};

struct ArtistInfo {
    QVariant id;                // int
    QVariant name;              //  QString
    QVariant is_followed;              //  bool
};

struct AlbumInfo {
    QVariant id;                // int
    QVariant name;              //  QString
    QVariant artist_id;     // qint64
    QVariant artist;       // QString
    QVariant is_like;              //  bool
};

struct UserInfo {
    QVariant id;                // int
    QVariant name;              //  QString
    QVariant signature;     // QString
    QVariant description;   // QString
    QVariant is_followed;       // bool
    QVariant gender;            // int
};

struct PlaylistInfo {
    QVariant id;                // int
    QVariant name;              //  QString
    QVariant creator;     // QString
    QVariant creator_id;     // qint64
    QVariant description;   // QString
    QVariant is_like;       // int 0 not sub 1 sub 2 self
    QVariant trackCount;       // int
};

struct FollowedInfo {
    QVariant type;              //int
    QVariant id;                // int
    QVariant name;              //  QString
    QVariant follow_day;         //Qstring
    QVariant gender;            // int
};

struct RcmdPlaylistInfo {
    QVariant id;                // int
    QVariant name;          // qstring
    QVariant play_count;
};

struct CommentInfo {
    QVariant id;            // qint64
    QVariant userId;        // qint64
    QVariant userName;      // qstring
    QVariant songId;        // qint64
    QVariant content;       // qstring
    QVariant likedCount;    // qint
    QVariant replyCount;    // qint
    QVariant songInfo;      //QString
    QVariant floorRefreshCount;  //int
};

struct Command {
    QString verb, type;
    QList<QVariant> args;
    bool operator==(const Command &x) {
        return x.args == x.args && x.type == x.type && x.verb == x.verb;
    }
};

class MainWindow;


class UndoStack : public QObject {
    Q_OBJECT

    QList<Command> stk;
    int limit;
    MainWindow *parent;

public:
    UndoStack(MainWindow *parent, int limit = 30);
    void setLimit(int limit);
    void push(Command command);
    void pop();
    void clearAll();
    Command top();



public slots:
    void undo(bool unused);
    void redo(bool unused);
};


class SongItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    SongItemDelegate(QObject *parent = nullptr);

public:
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QIcon *favourite_icon;
    QIcon *not_favourite_icon;
    QIcon *download_icon, *downloaded_icon;
    QIcon *comment_icon;

};


class ImageLabelEventFilter : public QObject {
    Q_OBJECT

public:
    explicit ImageLabelEventFilter(QObject *parent = nullptr);

signals:
    void imageLabelDoubleClicked(const QString& type, QVariant resourceId);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};


class PersistentCookieJar : public QNetworkCookieJar {
    Q_OBJECT
    QString MUSIC_U_value, nmtid_value;

public:
    explicit PersistentCookieJar(QObject *parent = nullptr) : QNetworkCookieJar(parent) {
    }

    ~PersistentCookieJar() {
    }

    // 创建一个公开的函数来设置所有 Cookies
    void setAllCookiesPublic(const QList<QNetworkCookie> &cookies) {
        setAllCookies(cookies);  // 这里可以访问 setAllCookies
    }

    // **保存 cookies 到文件**
    void saveCookiesToFile(const QString &fileName) {
        QString filePath = cwd.absoluteFilePath(fileName);
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "无法打开文件保存 cookies:" << file.errorString();
            return;
        }

        QByteArray plainCookies;
        QDataStream out(&plainCookies, QIODevice::WriteOnly);
        QList<QNetworkCookie> cookies = allCookies();
        out << cookies.size();
        for (const QNetworkCookie &cookie : cookies) {
            out << cookie.toRawForm();
        }
        plainCookies = MyEnc(plainCookies, cookiesKey);
        file.write(plainCookies);
        file.close();

        // qDebug() << cookies.size() << "Cookies 已保存到" << filePath;
    }

    QString GetTime10Digits() { // 获取当前时间的 Unix 时间戳（秒级）
        qint64 unixTimestamp = QDateTime::currentSecsSinceEpoch();

        // 将时间戳转换为字符串（十进制表示）
        QString timestampStr = QString::number(unixTimestamp);

        // 截取前 10 位字符
        QString truncatedTimestamp = timestampStr.left(10);

        return truncatedTimestamp;
    }

    // **从文件加载 cookies**
    void loadCookiesFromFile(const QString &fileName) {
        QString filePath = cwd.absoluteFilePath(fileName);
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "无法打开文件加载 cookies:" << file.errorString();
            return;
        }
        QByteArray encryptedCookies;
        encryptedCookies = file.readAll();
        encryptedCookies = MyDec(encryptedCookies, cookiesKey);
        QDataStream in(&encryptedCookies, QIODevice::ReadOnly);
        qsizetype count;
        in >> count;

        QList<QNetworkCookie> cookies;
        for (qsizetype i = 0; i < count; ++i) {
            QByteArray raw;
            in >> raw;

            QList<QNetworkCookie> parsed = QNetworkCookie::parseCookies(raw);
            for (const QNetworkCookie &cookie : parsed) {
                cookies.append(cookie);
            }
        }

        // qDebug() << "cookies loaded:" << cookies;

        QNetworkCookie cookie;
        cookie = QNetworkCookie("appver", "3.0.18.203152"); /*cookie.setDomain(".music.163.com");*/ cookies.append(cookie);
        cookie = QNetworkCookie("resulution", "1920x1080"); /*cookie.setDomain(".music.163.com");*/ cookies.append(cookie);
        cookie = QNetworkCookie("os", "pc"); /*cookie.setDomain(".music.163.com");*/ cookies.append(cookie);
        cookie = QNetworkCookie("osver", "Microsoft-Windows-10-Professional-build-22631-64bit"); /*cookie.setDomain(".music.163.com");*/ cookies.append(cookie);
        cookie = QNetworkCookie("channel", "netease"); /*cookie.setDomain(".music.163.com");*/ cookies.append(cookie);
        cookie = QNetworkCookie("packageType", "release"); /*cookie.setDomain(".music.163.com");*/ cookies.append(cookie);
        // cookie = QNetworkCookie("versionCode", "release"); /*cookie.setDomain(".music.163.com");*/ cookies.append(cookie);
        cookie = QNetworkCookie("mode", ""); /*cookie.setDomain(".music.163.com");*/ cookies.append(cookie);
        cookie = QNetworkCookie("buildver", GetTime10Digits().toUtf8()); /*cookie.setDomain(".music.163.com");*/  cookies.append(cookie);
        // 确保所有 Cookie 适用于整个 music.163.com 域名
        // for (QNetworkCookie &cookie : cookies) {
        //     /*cookie.setDomain(".music.163.com");*/  // 注意前面的 "."
        // }

        setAllCookies(cookies);
        file.close();
    }

signals:
    void didntLogin();

};


class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

public:
    QString getTempDir();
    QString getDownloadDir();
    QJsonArray getBlockCodeOrderList();
    Level getDownloadQuality();

private:
    QLabel *tempDirLabel, *downloadDirLabel, *homePageblocksOrderLabel, *downloadQualityLabel;
    QLineEdit *tempDirEdit, *downloadDirEdit;
    QPushButton *tempDirButton, *downloadDirButton;
    QPushButton *okButton, *cancelButton, *clearCacheButton, *openCacheButton, *openDownloadButton;
    QList<QPair<QString, QString>> checkBoxLabelsAndJsonValue;
    QList<QCheckBox*> checkBoxes;
    QComboBox *downloadQualityCombo;

    QGridLayout *layout;
    QGridLayout *checkBoxLayout;
};


class CommentsDialog : public QDialog {
    Q_OBJECT

public:
    explicit CommentsDialog(QWidget *parent = nullptr);

public:
    void setCommentModel(QStandardItemModel *model);
    void configCommentView();

private slots:
    // void onQuitButtonClicked();

private:
    void refreshing();
    void refreshed();
    void append(CommentInfo info);

public slots:
    void refreshComments(ApiRequester *requester = nullptr, qint64 song_id = 0, QString songInfo = "" ,int sortType = 0, QString cursor = "0", int pageSize = PAGE_SIZE);
    void clearComments();

private:
    friend class CommentFloorDialog;

private:
    QLabel *title;
    QTableView *commentView;
    QStandardItemModel *commentsModel;
    QPushButton *pageUpButton, *pageDownButton, *quitButton;
    QVBoxLayout *layout;
    QHBoxLayout *pageButtonLayout;
    Comment comment;
    ApiRequester *m_requester;
    QString m_cursor = "0";
    int m_type = 0, m_page = 1;
    qint64 m_songId;
    QString m_songInfo;
    QMetaObject::Connection pageDownButtonConn;
    MainWindow *mainWindow;
};


class CommentFloorDialog : public QDialog {
    Q_OBJECT

public:
    explicit CommentFloorDialog(QWidget *parent = nullptr);

public:
    void setCommentModel(QStandardItemModel *model);
    void configCommentFloorView();
    void setPageNo(int pageNo);

public:
    void refreshing();
    void refreshed();
    void append(CommentInfo info);

public slots:
    void getCommentFloor(ApiRequester *requester, qint64 songId, qint64 parentCommentId, qint64 time = -1, QString cursor = "", int limit = PAGE_SIZE);

private:
    QLabel *parentCommentLabel;
    QTableView *commentFloorView;
    QHBoxLayout *pageButtonLayout;
    QPushButton *pageUpButton, *pageDownButton;
    QStandardItemModel *commentFloorModel;
    QPushButton *quitButton;
    QVBoxLayout *layout;
    Comment comment;
    MainWindow *mainWindow;
    ApiRequester *m_requester;
    int m_pageNo;
    QString m_cursor;
    qint64 m_songId, m_parentCommentId;
    qint64 m_time = -1;
    friend class CommentFloorModelFilter;
};


class AddToPlaylistDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddToPlaylistDialog(QWidget *parent = nullptr);

public:
    void showMyPlaylist(ApiRequester *requester, qint64 userId, qint64 trackId);
    qint64 getTargetPlaylistId();
    QString getNewPlaylistname();
    bool isPrivatePlaylist();

public:
    QLabel *comboLabel, *newPlaylistNameLabel;
    QComboBox *playlistCombo;
    QLineEdit *newPlaylistNameEdit;
    QCheckBox *isPrivatePlaylistCheck;
    QGridLayout *layout;
    QPushButton *okButton, *cancelButton;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    ApiRequester *requester, *user_requester, *suggestion_requester;
    Downloader *downloader;


    enum CurrentWidget { kHomePage, kDetail } currentWidget;
    int cursor;

    QHash<QString, QString> suggest_keywords;

    enum CurrentModel { kSongModel, kArtistModel, kPlaylistModel, kAlbumModel, kUserModel, kFollowedModel, kRcmdPlaylistModel, kCommentsModel } currentModel;

    /*  |-----------|-----------|
     *  |   col     |   name    |
     *  |-----------|-----------|
     *      0           qint64 id;
            1           qint64 album_id;
            2           QList <qint64> artists_id;
            3           QString name;
            4           QList <QString> artists;
            5           QString artists_str;
            6           QString album;
            7           QString duration_str;
            8           int duration;
            9           bool is_downloaded;     // bool
            10          bool is_favorite;       // bool
     *
     */
    QStandardItemModel *song_model,*artist_model,*album_model,*user_model,*playlist_model,*followed_model,*rcmd_playlist_model, *comments_model;

    SongItemDelegate *delegate;

    User user;
    bool isGuest, is_login;
    qint64 user_id;
    QString user_nick_name;
    qint64 favourite_id;
    QList<qint64> favourites;

    Playlist playlist;
    Lyric lyric;
    Search search;
    Comment comment;

    int offset, comment_offset;

    bool info_visible;
    QRect table_view_rect, table_view_old_geometry;

    QList<QPair<QString, QString>> defaultKeywords, suggestKeywords;
    QWidget *keyword_suggest_widget;
    QCompleter *completer;
    QStandardItemModel *completeModel;
    QTimer *refreshTimer;
    QMetaObject::Connection conn_return_pressed;
    bool isPageButtonEnabled;

    friend class SongItemDelegate;
    friend class SearchCommand;
    friend class CommentsDialog;
    friend class CommentFloorDialog;

    ImageLabelEventFilter *imageLabelEventFilter;

public:
    bool is_ending = false;

public slots:
    void ComplexSearch();
    void RefreshAlbumDetail(qint64 album_id);
    void RefreshArtistDetail(qint64 artist_id);
    void RefreshUserDetail(qint64 user_id);
    void RefreshPlaylistDetail(qint64 playlist_id);
    void UpdateInfoVisibility(bool visible);
    void RefreshUserPlaylists(QJsonDocument json);
    void RefreshComment(qint64 song_id, QString song_info);
    void RefreshArtistAlbum(qint64 artist_id);
    void Relogin();

private slots:
    void OnCellClicked(const QModelIndex &index);
    void OnCellDoubleClicked(const QModelIndex &index);
    void SetUserInfo(qint64 user_id, QString user_nick_name);
    void CreateContextMenu(const QPoint &pos);
    void GetDefaultKeywords();

signals:
    void DownloadButtonClicked(int rowNum, qint64 id, QString artistsStr, QString name,  QString dir, EncodeType encode_type, Level level);
    void FavouritePlaylistGot(QJsonDocument json);
    void SelectArtistActionTriggered(qint64 selected_artists_id);


private:
    Ui::MainWindow *ui;
    QMetaObject::Connection prevSubBtnConn, prevRcmdBtnConn, prevCrBtnConn, playlistPageDownConn;
    QWidget *homepageWidget;
    QWidget *detailWidget;
    QScrollArea *global_scroll_area;
    QWidget *global_scroll_area_content;
    QVBoxLayout *global_layout;
    LoginWidget *loginWidget;
    QPixmap cover_pixmap;
    QString cover_title;

    QAction *dailyRecommendAction, *myFavouriteAction, *myPlaylistAction, *createPlaylistAction, *myFollowAction, *myRcmdFollowAction, *logoutAction;
    QAction *aboutAction, *settingsAction;

    CommentsDialog *commentsDialog;

private:
    void LoadSettings();
    void SaveSettings();
    void SetupDetailWidget();
    void SetupHomepageWidget();
    void SetupMenu();
    void Setup();
    void Refreshing();
    void Refreshed();
    QString GetTime10Digits();
    // QString checkAndFixImageExtension(const QString &filePath);

private:
    QVBoxLayout *global_vbox;

private:
    void Append(SongInfo info);
    void Append(ArtistInfo info);
    void Append(AlbumInfo info);
    void Append(UserInfo info);
    void Append(PlaylistInfo info);
    void Append(FollowedInfo info);
    void Append(RcmdPlaylistInfo info);
    void Append(CommentInfo info);
    void ConfigSongTableView();
    void ConfigArtistTableView();
    void ConfigAlbumTableView();
    void ConfigUserTableView();
    void ConfigPlaylistTableView();
    void ConfigFollowedTableView();
    void ConfigRcmdPlaylistTableView();
    void ConfigCommentTableView();
    void DisplayAlbumPicture(QString album_pic_url);
    void RefreshPlaylist(QJsonArray songs);
    void RefreshSearchSong();
    void RefreshSearchArtist();
    void RefreshSearchAlbum();
    void RefreshMyFavouritePlaylist();
    void RefreshDailyRecommend();
    void RefreshSearchUser();
    void RefreshSearchPlaylist();
    void RefreshMyFollowedUser();
    void RefreshRcmdPlaylist(qint64 playlist_id);
    void RefreshRcmdArtist(qint64 artist_id);
    void RefreshMyRcmdUser();
    void RefreshMyPlaylist();
    void RefreshMydefPlaylist(QList<QVariant> song_ids);
    void RefreshUserFollowedArtist(qint64 userId);
    void RefreshFollowedUser(qint64 userId);


    QJsonValue findKeyInJsonDocument(const QJsonValue &doc, const QString &prefix);
    QString ReadMusicU(const QString &filePath);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void RefreshKeywordCompleteModel();

// Undo Stack
public:
    friend class UndoStack;

    UndoStack undoStack;
    PersistentCookieJar *cookieJar;
};



#endif // MAINWINDOW_H
