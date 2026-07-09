#include "request.h"

Request* Request::m_instance = nullptr;

Request::Request(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this); // 指定父对象为当前 Request 实例
    manager->setProxy(QNetworkProxy::NoProxy);
    connect(manager, &QNetworkAccessManager::finished, this, &Request::onReplyFinished);
}

Request::~Request()
{
    delete manager;
}

Request* Request::instance()
{
    if (!m_instance)
    {
        m_instance = new Request();
    }
    return m_instance;
}

void Request::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        emit apiResponse(responseData);
    } else {
        QString errorString = reply->errorString();
        emit apiError(errorString);
    }

    reply->deleteLater();
}

void Request::getAPI(const QString path)
{
    QString domain = QString(STRINGIFY_MACRO(API_URL));
    QString url = domain + path;

    QNetworkRequest request(url);
    manager->get(request);
}

void Request::sendPostRequest(const QString &path, const QByteArray &rawData)
{
    QString domain = QString(STRINGIFY_MACRO(API_URL));
    QString url = domain + path;

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 添加自定义的 Cookie
    request.setRawHeader("Cookie", "XDEBUG_SESSION=PHPSTORM");

    // 加密
    QByteArray encryptedData = Decrypt::encrypt(rawData);

    // 封装 JSON
    QJsonObject json;
    json["cookie"] = QString(encryptedData);
    QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);

    manager->post(request, jsonData);
}
