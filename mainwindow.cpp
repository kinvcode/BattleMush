#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStringListModel>
#include <QDateTime>
#include <QScreen>
#include <QGuiApplication>
#include <QCursor>
#include <QPoint>

#include <vmp/VMProtectSDK.h>
#include "driver/msdriverpro.h"
#include "driver/driverload.h"
#include "global/globalconfig.h"
#include "threads/healthmonitor.h"


// 测试头文件
#include "global/globalplayer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    loadDriver();

    // 初始化组件
    initCompomentUI();

    // 加载配置文件
    Gconfig.loadFromJson();

    // 设置配置组件
    initConfig();

    // 绑定信号
    initSinalBind();

    // 初始化线程
    initThreads();

    console("系统初始化完毕！");

    // 线程操作
    // 主线程:FPS限制，检测是否登录，自动登录，是否创建队伍、
    // 键盘按键线程：buff\
    // 开启一个血蓝检测线程
    // 窗口截图线程
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_listWidget_clicked(const QModelIndex &index)
{
    int cur = index.row();
    if(cur > -1 && cur < 4)
    {
        ui->tabWidget->setCurrentIndex(cur);
    }
}

void MainWindow::showNotice(const QString &message)
{
    auto* notice = new NoticeWidget(message,this);
    notice->setAutoClose(1500);  // 自动关闭

    QPoint topLeft = mapToGlobal(rect().topLeft());  // 改为 topLeft
    QPoint pos = QPoint(topLeft.x() + 200, topLeft.y() - 20);

    notice->move(pos);
    notice->show();
}

void MainWindow::loadDriver()
{
    if (!MSDriverPRO::isLoadDriver())
    {
        DriverLoad driver_loader;
        bool driver_success = driver_loader.loadCommonDriver();
        if(!driver_success)
        {
            qApp->quit();
            return;
        }
    }
    if (MSDriverPRO::isLoadDriver()){
        console("变色龙安全驱动加载完毕~");
    }
}

void MainWindow::initCompomentUI()
{
    // ui->pushButton_2->setProperty("class","danger");

    // 表格
    ui->tableWidget->setColumnCount(11);
    ui->tableWidget->setHorizontalHeaderLabels({"怪物指针","怪物阵营", "怪物类型", "怪物代码","怪物名称","怪物血量","怪物坐标","宽度","高度","碰撞检测","建筑属性"});

    // 菜单
    ui->listWidget->setProperty("class","leftMenu");
    QListWidgetItem *item1 = new QListWidgetItem(QIcon(":/assets/images/icons/home.svg"), "启动");
    ui->listWidget->addItem(item1);
    QListWidgetItem *item2 = new QListWidgetItem(QIcon(":/assets/images/icons/setting.svg"), "配置");
    ui->listWidget->addItem(item2);
    QListWidgetItem *item3 = new QListWidgetItem(QIcon(":/assets/images/icons/vip.svg"), "VIP");
    ui->listWidget->addItem(item3);
    QListWidgetItem *item4 = new QListWidgetItem(QIcon(":/assets/images/icons/sata.svg"), "功能");
    ui->listWidget->addItem(item4);
    ui->listWidget->setCurrentRow(0);

    // tab组件
    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->tabBar()->hide();

    // 日志
    ui->textEdit_log->setProperty("class","commonlog");

    ui->comboBox->addItem("组包秒杀");
    ui->comboBox->addItem("搬砖");
    ui->comboBox->addItem("升级");

    // 重新设置样式表
    qApp->setStyleSheet(qApp->styleSheet());
}

void MainWindow::initConfig()
{
    ui->checkBox_auto_add_hp->setChecked(Gconfig.getValue("auto_add_health/add_hp").toBool());
    ui->lineEdit_auto_add_hp_rate->setText(Gconfig.getValue("auto_add_health/hp_rate").toString());
    ui->lineEdit_auto_add_hp_key->setText(Gconfig.getValue("auto_add_health/hp_key").toString());

    ui->checkBox_auto_add_mp->setChecked(Gconfig.getValue("auto_add_health/add_mp").toBool());
    ui->lineEdit_auto_add_mp_rate->setText(Gconfig.getValue("auto_add_health/mp_rate").toString());
    ui->lineEdit_auto_add_mp_key->setText(Gconfig.getValue("auto_add_health/mp_key").toString());
}

void MainWindow::initSinalBind()
{
    // 连接配置信号槽
    connect(this, &MainWindow::saveConfig, &Gconfig, &GlobalConfig::updateConfig);
}

void MainWindow::initThreads()
{
    HealthMonitor* t_heal_monitor = new HealthMonitor(this);
    t_heal_monitor->start();
    console("健康状态监控线程已启动");
}

void MainWindow::console(QString msg)
{
    int cursor = ui->textEdit_log->textCursor().blockNumber();
    if(cursor > 50000){
        ui->textEdit_log->clear();
    }
    // 获取当前日期时间
    QDateTime currentDateTime = QDateTime::currentDateTime();
    // 格式化为字符串
    QString formattedDateTime = currentDateTime.toString("HH:mm:ss ");
    msg = formattedDateTime + msg;
    ui->textEdit_log->append(msg);
    ui->textEdit_log->moveCursor(QTextCursor::End);
}


void MainWindow::on_pushButton_clicked()
{
    VMProtectBegin("__FUNCTION__");
    console(QString::number(Gplayer.HP));

    showNotice("123456");

    VMProtectEnd();
}


void MainWindow::on_lineEdit_auto_add_mp_rate_editingFinished()
{
    QString text = ui->lineEdit_auto_add_mp_rate->text();
    bool is_number;
    int value = text.toInt(&is_number);

    if (is_number && value >= 1 && value <= 100) {
        // 校验通过
        console("自动加蓝比例已保存");
        Gconfig.setValue("auto_add_health/mp_rate", value);
    } else {
        showNotice("无效数值，请输入1-100之间的数字");
        ui->lineEdit_auto_add_mp_rate->setText("");  // 清空内容
    }
}


void MainWindow::on_lineEdit_auto_add_hp_rate_editingFinished()
{
    QString text = ui->lineEdit_auto_add_hp_rate->text();
    bool is_number;
    int value = text.toInt(&is_number);

    if (is_number && value >= 1 && value <= 100) {
        // 校验通过
        console("自动加血比例已保存");
        Gconfig.setValue("auto_add_health/hp_rate", value);
    } else {
        showNotice("无效数值，请输入1-100之间的数字");
        ui->lineEdit_auto_add_hp_rate->setText("");  // 清空内容
    }
}


void MainWindow::on_checkBox_auto_add_mp_stateChanged(int arg1)
{
    if(ui->checkBox_auto_add_mp->isChecked()){
        console("自动加蓝已开启");
        Gconfig.setValue("auto_add_health/add_mp", true);
    }else{
        console("自动加蓝已关闭");
        Gconfig.setValue("auto_add_health/add_mp", false);
    }
}


void MainWindow::on_checkBox_auto_add_hp_stateChanged(int arg1)
{
    if(ui->checkBox_auto_add_hp->isChecked()){
        console("自动加血已开启");
        Gconfig.setValue("auto_add_health/add_hp", true);
    }else{
        console("自动加血已关闭");
        Gconfig.setValue("auto_add_health/add_hp", false);
    }
}


void MainWindow::on_lineEdit_auto_add_hp_rate_textEdited(const QString &arg1)
{
    QString text = ui->lineEdit_auto_add_hp_rate->text();
    bool is_number;
    int value = text.toInt(&is_number);

    if (!is_number || value < 1 || value > 100) {
        console("请输入1-100之间的数字");
    }
}


void MainWindow::on_lineEdit_auto_add_mp_rate_textEdited(const QString &arg1)
{
    QString text = ui->lineEdit_auto_add_mp_rate->text();
    bool is_number;
    int value = text.toInt(&is_number);

    if (!is_number || value < 1 || value > 100) {
        console("请输入1-100之间的数字");
    }
}

