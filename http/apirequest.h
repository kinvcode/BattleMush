#ifndef APIREQUEST_H
#define APIREQUEST_H
#include "http/request.h"
#include "http/requestsync.h"
#include <QTimeZone>
#include <QCryptographicHash>
#include "utils/hardware.h"

class APIRequest
{
public:
    // 获取版本号
    static int getVersion();

    // 发送剧情信息
    static void sendStoryQuest(QUrlQuery postData);
    // 发送账号信息
    static void sendAccountInfo();
    // 登录账号
    static int login(QString name, QString password);
    // 校验账号信息
    static int verifyUserInfo();
    // 刷新token
    static void flushToken();
    // 验证日志
    static void verifyLog(int status);
    // 解绑机器码
    static int unbind(QString name, QString password);
    // 查询卡密
    static int querycard(QString card);
    // 注册一个用户
    static int registeruser(QString name, QString password, QString card, QString machine_code);
    // 续费
    static int renewal(QString name, QString card, QString machine_code);
};

#endif // APIREQUEST_H
