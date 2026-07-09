#ifndef DECRYPT_H
#define DECRYPT_H

#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>

class Decrypt
{
public:
    static QByteArray encrypt(const QByteArray &data);

    static QJsonObject decrypt(QString encode_string);
};

#endif // DECRYPT_H
