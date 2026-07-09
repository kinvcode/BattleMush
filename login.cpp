#include "login.h"
#include "ui_login.h"
#include "http/request.h"
#include "http/requestsync.h"
#include "http/apirequest.h"
#include "process/process.h"
#include "global/controller.h"
#include "utils/config.h"

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);
    loadLoginParam();
}

Login::~Login()
{
    delete ui;
}

void Login::loadLoginParam()
{
    QString currentDir = QCoreApplication::applicationDirPath();
    QString config_path = currentDir + "/viplogin.json";
    Config* config = Config::instance();
    config->setFilePath(config_path);
    QString name = config->getValue("acc").toString();
    QString pass = config->getValue("pass").toString();

    if(!name.isEmpty()){
        qDebug() << "acc:" << name;
        ui->lineEdit_login_acc->setText(name);
    }
    if(!pass.isEmpty()){
        qDebug() << "pass:" << pass;
        ui->lineEdit_login_pass->setText(pass);
    }
}

void Login::on_pushButton_querycard_btn_clicked()
{
    QString card = ui->lineEdit_querycard_card->text();
    card = card.trimmed();
    int code = APIRequest::querycard(card);

    switch (code) {
    case 0:
        QMessageBox::information(NULL,"MicrosoftFastSearch","卡密可以正常使用");
        return;
        break;
    case 1:
        QMessageBox::information(NULL,"MicrosoftFastSearch","非法数据！");
        return;
        break;
    case 2:
        QMessageBox::information(NULL,"MicrosoftFastSearch","非法参数！");
        return;
        break;
    case 3:
        QMessageBox::information(NULL,"MicrosoftFastSearch","卡密不存在！");
        return;
        break;
    case 4:
        QMessageBox::information(NULL,"MicrosoftFastSearch","该卡密已被使用");
        return;
        break;
    default:
        QMessageBox::information(NULL,"MicrosoftFastSearch","未知错误！");
        return;
        break;
    }
}


void Login::on_pushButton_login_btn_clicked()
{
    VMProtectBegin("__FUNCTION__");

    ui->pushButton_login_btn->setEnabled(false);

    QString name = ui->lineEdit_login_acc->text();
    QString password = ui->lineEdit_login_pass->text();

    int code = APIRequest::login(name,password);
    if(code != 0)
    {
        switch (code) {
        case 1:
            QMessageBox::information(NULL,"MicrosoftFastSearch","会员已到期！");
            ui->pushButton_login_btn->setEnabled(true);
            return;
            break;
        case 2:
            QMessageBox::information(NULL,"MicrosoftFastSearch","机器码已变更");
            ui->pushButton_login_btn->setEnabled(true);
            return;
            break;
        default:
            break;
        }
    }

    bool verify_ok = false;

    QString machine_code = Hardware::generateMachineCode();
    QUrlQuery postData;
    postData.addQueryItem("machine_code", machine_code);
    QJsonObject data = RequestSync::postAPI("userinfo",postData);

    if(data.contains("expired"))
    {
        int expired = data.value("expired").toInt();
        if(expired == 0)
        {
            verify_ok = true;
        }
    }

    if(verify_ok){
        QString currentDir = QCoreApplication::applicationDirPath();
        QString config_path = currentDir + "/viplogin.json";
        Config* config = Config::instance();
        config->setFilePath(config_path);
        QJsonObject logindata;
        logindata["acc"] = name;
        logindata["pass"] = password;
        config->m_jsonObject = logindata;
        config->save();

        QMessageBox::information(NULL,"MicrosoftFastSearch",Gcontroller.m_vip_expired_date + "到期");
        emit processingFinished();
    }else{
        QMessageBox::information(NULL,"MicrosoftFastSearch","认证失败,请检查用户名和密码");
    }

    ui->pushButton_login_btn->setEnabled(true);

    VMProtectEnd();
}


void Login::on_pushButton_register_btn_clicked()
{
    QString machine_code = Hardware::generateMachineCode();
    QString name = ui->lineEdit_register_acc->text();
    QString password = ui->lineEdit_register_pass->text();
    QString card = ui->lineEdit_register_card->text();
    name = name.trimmed();
    password = password.trimmed();
    card = card.trimmed();

    int code = APIRequest::registeruser(name, password, card, machine_code);

    switch (code) {
    case 0:
        QMessageBox::information(NULL,"MicrosoftFastSearch","注册成功！");
        return;
        break;
    case 1:
        QMessageBox::information(NULL,"MicrosoftFastSearch","非法数据！");
        return;
        break;
    case 2:
        QMessageBox::information(NULL,"MicrosoftFastSearch","非法参数！");
        return;
        break;
    case 3:
        QMessageBox::information(NULL,"MicrosoftFastSearch","账号或密码格式不正确，请使用3~15位英文和数字");
        return;
        break;
    case 4:
        QMessageBox::information(NULL,"MicrosoftFastSearch","该卡密无效");
        return;
        break;
    case 5:
        QMessageBox::information(NULL,"MicrosoftFastSearch","该用户已存在");
        return;
        break;
    default:
        QMessageBox::information(NULL,"MicrosoftFastSearch","未知错误！");
        return;
        break;
    }
}


void Login::on_pushButton_unbind_btn_clicked()
{
    QString name = ui->lineEdit_unbind_acc->text();
    QString password = ui->lineEdit_unbind_pass->text();

    int code = APIRequest::unbind(name,password);

    switch (code) {
    case 0:
        QMessageBox::information(NULL,"MicrosoftFastSearch","已成功解绑！");
        return;
        break;
    case 1:
        QMessageBox::information(NULL,"MicrosoftFastSearch","非法数据！");
        return;
        break;
    case 2:
        QMessageBox::information(NULL,"MicrosoftFastSearch","非法账号密码！");
        return;
        break;
    case 3:
        QMessageBox::information(NULL,"MicrosoftFastSearch","账号不存在");
        return;
        break;
    case 4:
        QMessageBox::information(NULL,"MicrosoftFastSearch","账号密码不匹配");
        return;
        break;
    default:
        QMessageBox::information(NULL,"MicrosoftFastSearch","未知错误！");
        return;
        break;
    }
}


void Login::on_pushButton_renewal_btn_clicked()
{
    QString machine_code = Hardware::generateMachineCode();
    QString name = ui->lineEdit_renewal_acc->text();
    QString card = ui->lineEdit_renewal_card->text();
    name = name.trimmed();
    card = card.trimmed();

    int code = APIRequest::renewal(name, card, machine_code);

    switch (code) {
    case 0:
        QMessageBox::information(NULL,"MicrosoftFastSearch","续费成功！");
        return;
        break;
    case 1:
        QMessageBox::information(NULL,"MicrosoftFastSearch","非法数据！");
        return;
        break;
    case 2:
        QMessageBox::information(NULL,"MicrosoftFastSearch","非法参数！");
        return;
        break;
    case 3:
        QMessageBox::information(NULL,"MicrosoftFastSearch","该卡密无效！");
        return;
        break;
    case 4:
        QMessageBox::information(NULL,"MicrosoftFastSearch","该用户不存在！");
        return;
        break;
    default:
        QMessageBox::information(NULL,"MicrosoftFastSearch","未知错误！");
        return;
        break;
    }
}

