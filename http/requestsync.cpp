#include "requestsync.h"
#include "global/controller.h"

PersistentCookieJar::PersistentCookieJar(QObject *parent)
    : QNetworkCookieJar(parent)
{
}

QList<QNetworkCookie> PersistentCookieJar::getAllCookies() const
{
    return allCookies();
}

void PersistentCookieJar::setAllCookies(const QList<QNetworkCookie> &cookies)
{
    setCookiesFromUrl(cookies, QUrl()); // 空 URL 表示全局设置
}

QJsonObject RequestSync::getAPI(const QString path, bool is_encrypt)
{
    QJsonObject jsonObject;

    QString domain = QString(STRINGIFY_MACRO(API_URL));
    QString api = domain + path;

    // 创建QNetworkAccessManager对象
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    manager->setProxy(QNetworkProxy::NoProxy);

    // 设置请求URL
    QUrl url(api);

    // 创建HTTP请求
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    // request.setRawHeader("Cookie", "XDEBUG_SESSION=PHPSTORM");
    // 设置Authorization请求头，带上Bearer Token
    QString bearerToken = "Bearer " + Gcontroller.m_http_token;
    request.setRawHeader("Authorization", bearerToken.toUtf8());

    // 发送GET请求
    QNetworkReply *reply = manager->get(request);

    // 创建事件循环
    QEventLoop loop;

    // 连接请求完成的信号
    QObject::connect(reply, &QNetworkReply::finished, [&]() {
        if (reply->error() == QNetworkReply::NoError) {
            // 读取响应数据
            QByteArray responseData = reply->readAll();

            // 解析JSON数据
            QJsonDocument jsonDocument = QJsonDocument::fromJson(responseData);
            jsonObject = jsonDocument.object();
        } else {
            // 处理错误
            qDebug() << "HTTP请求错误: " << reply->errorString();
        }

        // 释放资源
        reply->deleteLater();
        manager->deleteLater();

        // 退出事件循环
        loop.quit();
    });

    // 启动事件循环
    loop.exec();

    if(!jsonObject.contains("session"))
    {
        return QJsonObject();
    }

    if(is_encrypt)
    {
        QString encode_string = jsonObject["session"].toString();
        jsonObject = Decrypt::decrypt(encode_string);
    }

    return jsonObject;
}

QJsonObject RequestSync::postAPI(const QString path, const QUrlQuery &postData, bool is_encrypt)
{
    QJsonObject jsonObject;

    QString domain = QString(STRINGIFY_MACRO(API_URL));
    QString api = domain + path;

    // 创建QNetworkAccessManager对象
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    manager->setProxy(QNetworkProxy::NoProxy);

    // 设置请求URL
    QUrl url(api);

    // 创建HTTP请求
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Cookie", "XDEBUG_SESSION=PHPSTORM");
    // 设置Authorization请求头，带上Bearer Token
    QString bearerToken = "Bearer " + Gcontroller.m_http_token;
    request.setRawHeader("Authorization", bearerToken.toUtf8());

    // 转换参数
    QByteArray postDataByteArray = postData.toString(QUrl::FullyEncoded).toUtf8();
    // 加密数据
    QByteArray encryptedData = Decrypt::encrypt(postDataByteArray);
    // 封装为 JSON 对象
    QJsonObject json;
    json["cookie"] = QString(encryptedData); // 将加密后的字符串作为 JSON 字段
    QJsonDocument jsonDoc(json);
    QByteArray jsonData = jsonDoc.toJson();

    // 发送GET请求
    QNetworkReply *reply = manager->post(request,jsonData);

    // 创建事件循环
    QEventLoop loop;

    // 连接请求完成的信号
    QObject::connect(reply, &QNetworkReply::finished, [&]() {
        if (reply->error() == QNetworkReply::NoError) {
            // 读取响应数据
            QByteArray responseData = reply->readAll();

            // 解析JSON数据
            QJsonDocument jsonDocument = QJsonDocument::fromJson(responseData);
            jsonObject = jsonDocument.object();
        } else {
            // 处理错误
            qDebug() << "HTTP请求错误: " << reply->errorString();
        }

        // 释放资源
        reply->deleteLater();
        manager->deleteLater();

        // 退出事件循环
        loop.quit();
    });

    // 启动事件循环
    loop.exec();

    if(!jsonObject.contains("session"))
    {
        return QJsonObject();
    }

    if(is_encrypt)
    {
        QString encode_string = jsonObject["session"].toString();
        jsonObject = Decrypt::decrypt(encode_string);
    }

    return jsonObject;
}

QJsonObject RequestSync::apiLoginGame(QString game_account,QString game_password)
{
    QString path = "https://member.dfoneople.com/launcher/restapi/login";
    QString email = game_account;
    QString password = game_password;
    QString keep = "false";
    QString newbie = "false";

    QJsonObject jsonObject = {{"code",-1},{"id",""},{"hash",""}};

    QUrl url(path);
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    manager->setProxy(QNetworkProxy::DefaultProxy);

    // 创建并设置 cookie jar
    PersistentCookieJar *cookieJar = new PersistentCookieJar();
    manager->setCookieJar(cookieJar);
    cookieJar->setParent(manager);

    // 构造 multipart/form-data 对象
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // 添加 email 字段
    QHttpPart emailPart;
    emailPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"email\""));
    emailPart.setBody(email.toUtf8());
    multiPart->append(emailPart);

    // 添加 password 字段
    QHttpPart passwordPart;
    passwordPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"password\""));
    passwordPart.setBody(password.toUtf8());
    multiPart->append(passwordPart);

    QHttpPart keepPart;
    keepPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"keep\""));
    keepPart.setBody(keep.toUtf8());
    multiPart->append(keepPart);

    QHttpPart newbiePart;
    newbiePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"newbie\""));
    newbiePart.setBody(newbie.toUtf8());
    multiPart->append(newbiePart);

    QNetworkRequest request(url);
    // request.setHeader(QNetworkRequest::UserAgentHeader, "QtClient");
    // QString bearerToken = "Bearer " + Global::http_token;
    // request.setRawHeader("Authorization", bearerToken.toUtf8());

    // 发送 POST 请求
    QNetworkReply *reply = manager->post(request, multiPart);

    // multiPart 会在 reply 被删除时自动删除
    multiPart->setParent(reply);

    QEventLoop loop;
    QString redirectUrl;

    QObject::connect(reply, &QNetworkReply::finished, [&]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QString responseText = QString::fromUtf8(responseData);

            qDebug() << "响应文本:" << responseText;

            QJsonDocument jsonDocument = QJsonDocument::fromJson(responseData);
            QJsonObject responseObject = jsonDocument.object();
            // jsonObject = jsonDocument.object();

            if(responseObject.contains("code")){
                if(responseObject["code"].toInt() == 83){
                    jsonObject["code"] = 2; // VPN未设置，Our sincerest apologies, we are unable to offer this service in your region
                }
            }

            if(responseObject.contains("message") && responseObject.value("message").toString() == "success")
            {
                qDebug() << "获取到跳转界面";
                redirectUrl = responseObject.value("nextUrl").toString();
            }
        } else {
            qDebug() << "POST 请求错误: " << reply->errorString();
        }

        reply->deleteLater();

        loop.quit();
    });

    loop.exec();

    if (redirectUrl.isEmpty()) {
        qDebug() << "未获取到跳转地址";
        jsonObject["code"] = 1;
        return jsonObject;
    }

    // 请求跳转页面（B页面），自动携带 cookie
    redirectUrl = "https://member.dfoneople.com" + redirectUrl;
    QNetworkRequest request2{ QUrl(redirectUrl) };
    QNetworkReply *reply2 = manager->get(request2);

    QString finalResponse;

    QObject::connect(reply2, &QNetworkReply::finished, [&]() {
        QByteArray responseData = reply2->readAll();
        finalResponse = QString::fromUtf8(responseData);

        // qDebug() << "响应界面:" << finalResponse;


        QRegularExpression re(R"(dfoglobal://\d+/(\d+)/([a-fA-F0-9]{64}))");
        QRegularExpressionMatch match = re.match(finalResponse);

        if (match.hasMatch()) {
            QString id = match.captured(1);
            QString hash = match.captured(2);

            qDebug() << "ID:" << id;
            qDebug() << "Hash:" << hash;
            qDebug() << "启动游戏中...";
            // Sleep(500);
            jsonObject["id"] = id;
            jsonObject["hash"] = hash;
            jsonObject["code"] = 0;

            // // 要执行的命令行，EXE 路径 + 参数（必须是可修改的字符串）
            // // 获取游戏路径
            // QString cmd_start_str = "D:\\soft\\game\\DFO\\DFO.exe 9?3.211.150.198?7101?" + id + "?" + hash + "?0?0?0?0?0?2?0?0?0?0?0?0?0?0?0?0";
            // // 转换为 Windows API 需要的 wchar_t*（CreateProcess 需要一个可修改的缓冲区）
            // std::wstring wcmd = cmd_start_str.toStdWString();
            // wchar_t* cmdLine = wcmd.data();  // 注意：CreateProcess 会修改这个字符串，不能是 const！

            // QString workingDir = "D:\\soft\\game\\DFO";
            // std::wstring wDir = workingDir.toStdWString();

            // STARTUPINFO si = { sizeof(si) };
            // PROCESS_INFORMATION pi;

            // BOOL success = CreateProcess(
            //     NULL,         // 应用程序名称（NULL 表示用 cmdLine 的第一个参数）
            //     cmdLine,      // 命令行（可包含参数）
            //     NULL,         // 进程安全属性
            //     NULL,         // 线程安全属性
            //     FALSE,        // 是否继承句柄
            //     0,            // 创建标志
            //     NULL,         // 环境变量
            //     wDir.c_str(), // 设置工作目录
            //     &si,          // STARTUPINFO 结构
            //     &pi           // PROCESS_INFORMATION 结构
            //     );

            // if (success) {
            //     qDebug() << "游戏启动成功！";
            //     CloseHandle(pi.hProcess);
            //     CloseHandle(pi.hThread);
            // } else {
            //     qDebug() << "启动游戏失败:" << GetLastError();
            // }
        }

        reply2->deleteLater();
        loop.quit();
    });

    loop.exec();

    // manager->deleteLater();
    qDebug() << "API登录游戏执行都末尾";
    return jsonObject;
}

QJsonObject RequestSync::cjyTifen(QString account, QString password)
{
    // LOG("API查询积分");
    QString path = "http://upload.chaojiying.net/Upload/GetScore.php";

    QJsonObject jsonObject = {{"code",-1},{"tifen",0}};

    QUrl url(path);
    QNetworkAccessManager *manager = new QNetworkAccessManager();

    // 构造 multipart/form-data 对象
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // 添加 user 字段
    QHttpPart userPart;
    userPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"user\""));
    userPart.setBody(account.toUtf8());
    multiPart->append(userPart);

    // 添加 password 字段
    QHttpPart passwordPart;
    passwordPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"pass\""));
    passwordPart.setBody(password.toUtf8());
    multiPart->append(passwordPart);

    QNetworkRequest request(url);

    // 发送 POST 请求
    QNetworkReply *reply = manager->post(request, multiPart);

    // multiPart 会在 reply 被删除时自动删除
    multiPart->setParent(reply);


    QEventLoop loop;
    QString redirectUrl;

    QObject::connect(reply, &QNetworkReply::finished, [&]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QString responseText = QString::fromUtf8(responseData);

            qDebug() << "响应文本:" << responseText;
            // LOG("响应文本："+responseText);

            QJsonDocument jsonDocument = QJsonDocument::fromJson(responseData);
            QJsonObject responseObject = jsonDocument.object();

            if(responseObject.contains("err_no")){
                if(responseObject["err_no"].toInt() == 0){
                    jsonObject["code"] = 0;
                    jsonObject["tifen"] = responseObject["tifen"].toInt();
                }
            }
        } else {
            // LOG("POST错误：" + reply->errorString());
            qDebug() << "POST 请求错误: " << reply->errorString();
        }

        // 释放资源
        reply->deleteLater();
        manager->deleteLater();

        loop.quit();
    });

    loop.exec();

    // LOG("API查询积分结束");
    return jsonObject;
}

QJsonObject RequestSync::cjyVerifyCode(QString account, QString password, QString img_path)
{
    QString path = "http://upload.chaojiying.net/Upload/Processing.php";
    QString softid_str = "96001";
    QString codetype_str = "3005";

    QJsonObject jsonObject = {{"code",-1},{"pic_str","AAAAA"}};// 设置一个默认的验证码

    QUrl url(path);
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    manager->setProxy(QNetworkProxy::NoProxy);

    // 构造 multipart/form-data 对象
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // 添加 user 字段
    QHttpPart userPart;
    userPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"user\""));
    userPart.setBody(account.toUtf8());
    multiPart->append(userPart);

    // 添加 password 字段
    QHttpPart passwordPart;
    passwordPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"pass\""));
    passwordPart.setBody(password.toUtf8());
    multiPart->append(passwordPart);

    // 添加软件ID
    QHttpPart softidPart;
    softidPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"softid\""));
    softidPart.setBody(softid_str.toUtf8());
    multiPart->append(softidPart);

    // 添加验证码类型
    QHttpPart codetypePart;
    codetypePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"codetype\""));
    codetypePart.setBody(codetype_str.toUtf8());
    multiPart->append(codetypePart);

    // 添加图片
    QFile img_file(img_path);
    img_file.open(QIODevice::ReadOnly);
    QByteArray imgData = img_file.readAll();
    QHttpPart userfilePart;
    userfilePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"userfile\"; filename=\"img.jpg\""));
    userfilePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
    userfilePart.setBody(imgData);  // <-- 直接放二进制流
    multiPart->append(userfilePart);

    QNetworkRequest request(url);

    // 发送 POST 请求
    QNetworkReply *reply = manager->post(request, multiPart);

    // multiPart 会在 reply 被删除时自动删除
    multiPart->setParent(reply);

    QEventLoop loop;
    QString redirectUrl;

    QObject::connect(reply, &QNetworkReply::finished, [&]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QString responseText = QString::fromUtf8(responseData);

            qDebug() << "响应文本:" << responseText;
            // LOG(responseText);

            QJsonDocument jsonDocument = QJsonDocument::fromJson(responseData);
            QJsonObject responseObject = jsonDocument.object();

            if(responseObject.contains("err_no")){
                if(responseObject["err_no"].toInt() == 0){
                    jsonObject["code"] = 0;
                    jsonObject["pic_str"] = responseObject["pic_str"].toString();
                }
            }
        } else {
            qDebug() << "POST 请求错误: " << reply->errorString();
        }

        // 释放资源
        reply->deleteLater();
        manager->deleteLater();

        loop.quit();
    });

    loop.exec();

    return jsonObject;
}
