#ifndef CRYPTO_H
#define CRYPTO_H


#include "../common.h"

#include "Qt-AES-master/qaesencryption.h"
#include <QString>
#include <QByteArray>
#include <QVector>
#include <QCryptographicHash>
#include <QRandomGenerator64>

QByteArray EapiEncrypt(const QByteArray& data);
// QByteArray MarkerEncrypt(const QByteArray& data);
QByteArray CacheKeyEncrypt(const QByteArray& data);
QByteArray EapiDecrypt(const QByteArray& encrypted);
QByteArray MarkerDecrypt(const QByteArray& encrypted);
QByteArray MyDec(const QByteArray &encrypted, const QString &keyStr);
QByteArray MyEnc(const QByteArray &encrypted, const QString &keyStr);

QString xorEncrypt(const QString &input, const QString &key);
QString md5Base64(const QString &input);
QString getRandomDeviceId();

#endif // CRYPTO_H
