#include "decrypt.h"
#include <openssl/aes.h>
#include <openssl/rand.h>

#include <QString>


QByteArray Decrypt::encrypt(const QByteArray &data)
{
    QString key_str = "TuYtstb9JsoeOpHA2925hdKHFu8uWa7J";
    QByteArray key = key_str.toUtf8();

    if (key.size() != 32) {
        return QByteArray();
    }

    // IV参数随机化
    QByteArray iv(AES_BLOCK_SIZE, 0);
    if (!RAND_bytes(reinterpret_cast<unsigned char*>(iv.data()), AES_BLOCK_SIZE)) {
        return QByteArray();
    }

    QByteArray iv_origin = iv;

    // 内存对齐
    QByteArray paddedData = data;
    int padding = AES_BLOCK_SIZE - (paddedData.size() % AES_BLOCK_SIZE);
    paddedData.append(QByteArray(padding, padding));

    // 设置加密长度
    QByteArray encryptedData;
    encryptedData.resize(paddedData.size());

    // 设置密钥
    AES_KEY aesKey;
    AES_set_encrypt_key(reinterpret_cast<const unsigned char*>(key.constData()), 256, &aesKey);

    // 加密字符串
    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(paddedData.constData()),
                    reinterpret_cast<unsigned char*>(encryptedData.data()),
                    paddedData.size(),
                    &aesKey,
                    reinterpret_cast<unsigned char*>(iv.data()),
                    AES_ENCRYPT);

    // 填充IV原始字节数据
    encryptedData.prepend(iv_origin);

    return encryptedData.toBase64();
}

QJsonObject Decrypt::decrypt(QString encode_string)
{
    QJsonObject jsonObject;

    QString iv_string = encode_string.left(AES_BLOCK_SIZE);

    QByteArray iv = iv_string.toUtf8();
    QByteArray iv_origin = iv;

    QByteArray encode_bytes = encode_string.mid(AES_BLOCK_SIZE).toUtf8();
    QByteArray cipherText = QByteArray::fromBase64(encode_bytes);;

    if (cipherText.isEmpty())
    {
        return QJsonObject();
    }

    QString key_str = "TuYtstb9JsoeOpHA2925hdKHFu8uWa7J";
    QByteArray key = key_str.toUtf8();

    AES_KEY decryptKey;
    AES_set_decrypt_key(reinterpret_cast<const unsigned char *>(key.constData()), 256, &decryptKey);

    // 准备解密输出
    QByteArray decryptedData(cipherText.size(), 0);

    // 执行 AES CBC 解密
    AES_cbc_encrypt(reinterpret_cast<const unsigned char *>(cipherText.constData()),
                    reinterpret_cast<unsigned char *>(decryptedData.data()),
                    cipherText.size(),
                    &decryptKey,
                    reinterpret_cast<unsigned char *>(iv.data()),
                    AES_DECRYPT);

    // 移除填充
    int paddingLength = decryptedData.at(decryptedData.size() - 1);
    if (paddingLength > 0 && paddingLength <= AES_BLOCK_SIZE) {
        decryptedData.chop(paddingLength);
    }

    // 数据转换
    QJsonDocument jsonDoc = QJsonDocument::fromJson(decryptedData);
    if (jsonDoc.isNull()) {
        return QJsonObject();
    }

    if (!jsonDoc.isObject()) {
        return QJsonObject();
    }

    jsonObject = jsonDoc.object();

    return jsonObject;
}
