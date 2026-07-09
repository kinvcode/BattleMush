#include "apirequest.h"
#include "global/controller.h"

int APIRequest::getVersion()
{
    QJsonObject data = RequestSync::getAPI("lastversion");
    if(data.contains("version"))
    {
        int version = data.value("version").toInt();
        return version;
    }
    return 0;
}


void APIRequest::sendAccountInfo()
{
    // // 发送账号信息
    // QString machine = Config::getStringValue("Machine");
    // QString account = Config::getStringValue("GameAccount");
    // QString password = Config::getStringValue("GamePassword");
    // QString email_password = Config::getStringValue("EmailPassword");
    // QString otp_secret = Config::getStringValue("OTP_SECRET");

    // QUrlQuery postData;
    // postData.addQueryItem("machine", machine);
    // postData.addQueryItem("account", account);
    // postData.addQueryItem("password", password);
    // postData.addQueryItem("email_password", email_password);
    // postData.addQueryItem("otp_secret", otp_secret);

    // Request::instance()->postAPI("accountinfo", postData);
}

int APIRequest::login(QString name, QString password)
{
    QString machine_code = Hardware::generateMachineCode();
    QUrlQuery postData;
    postData.addQueryItem("name", name);
    postData.addQueryItem("password", password);
    postData.addQueryItem("machine_code", machine_code);
    QJsonObject data = RequestSync::postAPI("login",postData,false);

    if(data.empty())
    {
        return 999;
    }

    if(data.contains("session"))
    {
        QString token = data.value("session").toString();
        Gcontroller.m_http_token = token;
        Gcontroller.m_http_token_timestamp = QDateTime::currentSecsSinceEpoch();
    }

    if(data.contains("error"))
    {
        if(data.contains("expired_date"))
        {
            Gcontroller.m_vip_expired_date = data.value("expired_date").toString();
        }

        if(data.contains("expired_timestamp"))
        {
            Gcontroller.m_vip_expired_timestamp = data.value("expired_timestamp").toInteger();
        }

        return data.value("error").toInt();
    }

    return 999;
}

int APIRequest::verifyUserInfo()
{
    // 自定义返回值类型
    /**
     * -1 未知错误
     * 0 VIP未到期
     * 1 VIP已到期
     * 2 VIP不存在 - 会话过期
     */

    QString machine_code = Hardware::generateMachineCode();
    QUrlQuery postData;
    postData.addQueryItem("machine_code", machine_code);
    QJsonObject data = RequestSync::postAPI("userinfo",postData);

    if(data.empty())
    {
        return 2;
    }

    if(data.contains("expired"))
    {
        int expired = data.value("expired").toInt();
        return expired;
    }

    return -1;
}

void APIRequest::flushToken()
{
    QUrlQuery postData;
    QJsonObject data = RequestSync::postAPI("refresh",postData,false);

    if(data.empty())
    {
        return;
    }

    if(data.contains("session"))
    {
        QString token = data.value("session").toString();
        Gcontroller.m_http_token = token;
        Gcontroller.m_http_token_timestamp = QDateTime::currentSecsSinceEpoch();
    }
}

void APIRequest::verifyLog(int status)
{
    QUrlQuery postData;
    QString machine_code = Hardware::generateMachineCode();
    postData.addQueryItem("machine_code", machine_code);
    postData.addQueryItem("status", QString::number(status));
    QJsonObject data = RequestSync::postAPI("verify",postData,false);
}

int APIRequest::unbind(QString name, QString password)
{
    QUrlQuery postData;
    postData.addQueryItem("name", name);
    postData.addQueryItem("password", password);
    QJsonObject data = RequestSync::postAPI("unbind",postData,true);

    if(data.empty())
    {
        return -1;
    }

    if(data.contains("error"))
    {
        return data.value("error").toInt();
    }

    return -1;
}

int APIRequest::querycard(QString card)
{
    QUrlQuery postData;
    postData.addQueryItem("card", card);
    QJsonObject data = RequestSync::postAPI("querycard",postData,true);

    if(data.empty())
    {
        return -1;
    }

    if(data.contains("error"))
    {
        return data.value("error").toInt();
    }

    return -1;
}

int APIRequest::registeruser(QString name, QString password, QString card, QString machine_code)
{
    QUrlQuery postData;
    postData.addQueryItem("name", name);
    postData.addQueryItem("password", password);
    postData.addQueryItem("card", card);
    postData.addQueryItem("machine_code", machine_code);
    QJsonObject data = RequestSync::postAPI("registeruser",postData,true);

    if(data.empty())
    {
        return -1;
    }

    if(data.contains("error"))
    {
        return data.value("error").toInt();
    }

    return -1;
}

int APIRequest::renewal(QString name, QString card, QString machine_code)
{
    QUrlQuery postData;
    postData.addQueryItem("name", name);
    postData.addQueryItem("card", card);
    postData.addQueryItem("machine_code", machine_code);
    QJsonObject data = RequestSync::postAPI("renewal",postData,true);

    if(data.empty())
    {
        return -1;
    }

    if(data.contains("error"))
    {
        return data.value("error").toInt();
    }

    return -1;
}
