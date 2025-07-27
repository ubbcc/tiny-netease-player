#ifndef COMMON_H
#define COMMON_H

#include <QString>

#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonValue>

#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QTimer>

#include <QtCore/QVariant>
#include <QSize>
#include <QDir>
#include <QString>
#include <QRegularExpression>

enum EncodeType { kMp3, kAac };
enum Level { kLossless, kHigher, kStandard, kHires, kJyeffect, kSky, kJymaster , kBest};

inline QString MUSIC_U_value, nmtid_value,MUSIC_U;
inline QString cookies_str = "";
const QSize pic_size{200, 200}, imglb_size{210, 210};
static int PAGE_SIZE = 20;
inline QDir cwd = QDir();
inline QString defaultAlbumImgPath = ":/default_cover.jpg";
inline QDir TEMP_DIR = QDir::temp().absoluteFilePath("TinyNeteaseMusicCache");
inline QDir TEMP_PIC_DIR = TEMP_DIR.absoluteFilePath("PictureCache");
// inline QDir DOWNLOAD_DIR("~/Desktop/TinyNeteaseMusicDownload");
inline QDir DOWNLOAD_DIR = QDir::home().absoluteFilePath("Desktop/TinyNeteaseMusicDownload");
inline QJsonArray HOMEPAGE_BLOCKS_ORDER_LIST{"PAGE_RECOMMEND_SPECIAL_CLOUD_VILLAGE_PLAYLIST"};
inline Level quality{kStandard};
inline EncodeType encodeType{kMp3};
inline QRegularExpression sanitizeFileNameregex(R"([\\/:*?"<>|])");
const QString cookiesKey{"L2g&9@rZ!mb7X#Hp"};
inline bool isGuest = true;

#endif // COMMON_H
