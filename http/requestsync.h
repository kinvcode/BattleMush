#ifndef REQUESTSYNC_H
#define REQUESTSYNC_H

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QSslError>
#include <QFile>
#include "utils/decrypt.h"

#define STR_VALUE(arg) #arg
#define STRINGIFY_MACRO(x) STR_VALUE(x)

class PersistentCookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    explicit PersistentCookieJar(QObject *parent = nullptr);

    // 获取所有 Cookie（包括 HttpOnly 的）
    QList<QNetworkCookie> getAllCookies() const;

    // 设置 cookies（如从外部加载）
    void setAllCookies(const QList<QNetworkCookie> &cookies);
};

class RequestSync
{
public:
    static QJsonObject getAPI(const QString path, bool is_encrypt = true);
    static QJsonObject postAPI(const QString path, const QUrlQuery &postData, bool is_encrypt = true);
    // 检查账号是否存在
    static bool checkAccount(QString path);
    // API登录游戏
    static QJsonObject apiLoginGame(QString game_account,QString game_password);

    // API获取超级鹰的题分余额
    static QJsonObject cjyTifen(QString account,QString password);
    // API超级鹰识别验证码
    static QJsonObject cjyVerifyCode(QString account,QString password,QString img_path);
};

#endif // REQUESTSYNC_H
