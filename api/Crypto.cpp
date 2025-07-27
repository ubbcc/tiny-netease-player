#include "Crypto.h"

const QString kEapiKey = "e82ckenh8dichen8";
const QString kCacheKey = ")(13daqP@ssw0rd~";
const QString kMarkerKey = "#14ljk_!\\]&0U<'(";

QByteArray GenerateKey(const QByteArray &key) {
    QByteArray newKey = key.left(16);
    return newKey;
}

QByteArray MyEnc(const QByteArray &data, const QString &keyStr) {
    // QByteArray origData = data.toUtf8();
    QByteArray origData = data;
    QByteArray key = keyStr.toUtf8();

    QByteArray aesKey = GenerateKey(key);

    int padding = aesKey.size() - origData.size() % aesKey.size();
    QByteArray plain = origData;
    plain.append(QByteArray(padding, char(padding)));

    QAESEncryption aes(QAESEncryption::AES_128, QAESEncryption::ECB);
    QByteArray encrypted = aes.encode(plain, aesKey);

    return encrypted;
}

QByteArray MyDec(const QByteArray &encrypted, const QString &keyStr) {
    bool isPlain = true;
    if (encrypted.size() % 16 == 0) {
        // for (int i = 0; i < 5; i++) {
        //     if (not (32 <= encrypted[i] and encrypted[i] <= 126)) {
        //         isPlain = false;
        //         break;
        //     }
        // }
        isPlain = false;
    }
    if (isPlain) {
        return encrypted;
    }

    QByteArray key = keyStr.toUtf8();
    QByteArray aesKey = GenerateKey(key);

    QAESEncryption aes(QAESEncryption::AES_128, QAESEncryption::ECB);
    QByteArray decrypted = aes.decode(encrypted, aesKey);
    int padding = decrypted.at(decrypted.size() - 1); // 填充的字节数
    decrypted.chop(padding);
    return decrypted;
}

QByteArray EapiEncrypt(const QByteArray &data) {
    return MyEnc(data, kEapiKey);
}

QByteArray CacheKeyEncrypt(const QByteArray &data) {
    return MyEnc(data, kCacheKey);
}

QByteArray EapiDecrypt(const QByteArray &data) {;
    return MyDec(data, kEapiKey);
}

// XOR 加密
QString xorEncrypt(const QString &input, const QString &key) {
    QString result;
    for (int i = 0; i < input.length(); i++) {
        QChar encryptedChar = QChar(input.at(i).unicode() ^ key.at(i % key.length()).unicode());
        result.append(encryptedChar);
    }
    return result;
}

// 计算 MD5 并转为 Base64
QString md5Base64(const QString &input) {
    QByteArray hash = QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Md5);
    return hash.toBase64();
}

// 随机获取设备 ID
QString getRandomDeviceId() {
    // if (deviceIdList.isEmpty()) return "";
    // int index = QRandomGenerator::global()->bounded(deviceIdList.size());
    // return deviceIdList.at(index);
    // 313CD3C6D39148E94A6CD885B40E7C489AC9504078A7513928CE

    static const QString charSet = "0123456789ABCDEF";
    QString res;
    for (int i = 0; i < 52; i++) {
        int x = QRandomGenerator64::global()->bounded(16);
        res += QChar(charSet[x]);
    }
    return res;
}
