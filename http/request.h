#ifndef REQUEST_H
#define REQUEST_H

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include "utils//decrypt.h"

#define STR_VALUE(arg) #arg
#define STRINGIFY_MACRO(x) STR_VALUE(x)

class Request : public QObject
{
    Q_OBJECT
private:
    static Request* m_instance;
    QNetworkAccessManager *manager;
    explicit Request(QObject *parent = nullptr);
    ~Request();
public:
    static Request* instance();

public:

    // Get请求API
    void getAPI(const QString path);

    // Post请求API
    template<typename T>
    void postAPI(const QString &path, const T &postData) {
        QByteArray rawData = toByteArray(postData);
        sendPostRequest(path, rawData);
    }

private:

    QByteArray toByteArray(const QUrlQuery &data) {
        return data.toString(QUrl::FullyEncoded).toUtf8();
    }

    QByteArray toByteArray(const QJsonObject &data) {
        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    QByteArray toByteArray(const QJsonArray &data) {
        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    void sendPostRequest(const QString &path, const QByteArray &rawData);
signals:
    void apiResponse(const QByteArray &responseData);
    void apiError(const QString &errorString);
private slots:
    void onReplyFinished(QNetworkReply *reply);
};

#endif // REQUEST_H
