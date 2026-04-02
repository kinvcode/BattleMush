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
#include "threads/updategamedata.h"
#include "widgets/hotkeyedit.h"
#include "utils/helper.h"

// 测试头文件
#include "global/globalplayer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_threadManager(new ThreadManager(this))
{
    ui->setupUi(this);

    loadDriver();
    // MSDriverPRO::keyboardClick(0x45);

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

        // 初始化键盘状态，打开小键盘
        if ((GetKeyState(VK_NUMLOCK) & 1)) MSDriverPRO::keyboardClick(0x45);
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
    ui->lineEdit_auto_add_hp_key->m_config_path = "auto_add_health/hp_key";
    ui->lineEdit_auto_add_hp_key->m_currentKey = Gconfig.getValue("auto_add_health/hp_key").toInt();
    ui->lineEdit_auto_add_hp_key->updateDisplay();

    ui->checkBox_auto_add_mp->setChecked(Gconfig.getValue("auto_add_health/add_mp").toBool());
    ui->lineEdit_auto_add_mp_rate->setText(Gconfig.getValue("auto_add_health/mp_rate").toString());
    ui->lineEdit_auto_add_mp_key->m_config_path = "auto_add_health/mp_key";
    ui->lineEdit_auto_add_mp_key->m_currentKey = Gconfig.getValue("auto_add_health/mp_key").toInt();
    ui->lineEdit_auto_add_mp_key->updateDisplay();
}

void MainWindow::initSinalBind()
{
    // 连接配置信号槽
    connect(this, &MainWindow::saveConfig, &Gconfig, &GlobalConfig::updateConfig);

    // 自动加蓝快捷键
    // connect(ui->lineEdit_auto_add_mp_key, &HotkeyEdit::hotkeyChanged, this, &MainWindow::onHotkeyChanged);
}

void MainWindow::initThreads()
{
    // 连接信号
    connect(m_threadManager, &ThreadManager::threadStarted,
            this, &MainWindow::onThreadStarted);
    connect(m_threadManager, &ThreadManager::threadProgress,
            this, &MainWindow::onThreadProgress);
    connect(m_threadManager, &ThreadManager::threadFinished,
            this, &MainWindow::onThreadFinished);
    connect(m_threadManager, &ThreadManager::allThreadsFinished,
            this, &MainWindow::onAllThreadsFinished);

    // 添加默认线程
    m_threadManager->addThread(m_UpdateGameData_tid, "UpdateGameData");
    m_threadManager->addThread(m_HealthMonitor_tid, "HealthMonitor");

    m_threadManager->startAll();
    console("启动所有线程");
}

void MainWindow::onStartAll()
{
    m_threadManager->startAll();
    console("启动所有线程");
}

void MainWindow::onStopAll()
{
    m_threadManager->stopAll();
    console("停止所有线程");
}

void MainWindow::onAddThread()
{
    // static int nextId = 100;
    // QString workerType = m_workerTypeSelector->currentText();
    // if (m_threadManager->addThread(nextId, workerType)) {
    //     console(QString("添加线程 %1 (类型: %2)").arg(nextId).arg(workerType));
    //     refreshThreadList();
    //     nextId++;
    // }
}

void MainWindow::onRemoveThread()
{
    // int threadId = m_threadSelector->currentText().split(" ").first().toInt();
    // if (m_threadManager->removeThread(threadId)) {
    //     console(QString("移除线程 %1").arg(threadId));
    //     refreshThreadList();
    // }
}

void MainWindow::onStartThread()
{
    // int threadId = m_threadSelector->currentText().split(" ").first().toInt();
    // if (m_threadManager->startThread(threadId)) {
    //     console(QString("启动线程 %1").arg(threadId));
    // }
}

void MainWindow::onStopThread()
{
    // int threadId = m_threadSelector->currentText().split(" ").first().toInt();
    // if (m_threadManager->stopThread(threadId)) {
    //     console(QString("停止线程 %1").arg(threadId));
    // }
}

void MainWindow::onPauseThread()
{
    // int threadId = m_threadSelector->currentText().split(" ").first().toInt();
    // if (m_threadManager->pauseThread(threadId)) {
    //     console(QString("暂停线程 %1").arg(threadId));
    // }
}

void MainWindow::onResumeThread()
{
    // int threadId = m_threadSelector->currentText().split(" ").first().toInt();
    // if (m_threadManager->resumeThread(threadId)) {
    //     console(QString("恢复线程 %1").arg(threadId));
    // }
}

void MainWindow::onThreadStarted(int threadId, const QString& workerType)
{
    DebugOutput::out(QString("线程 %1 (%2) 已启动").arg(threadId).arg(workerType));
}

void MainWindow::onThreadProgress(int threadId, int value)
{
    DebugOutput::out(QString("线程 %1 进度: %2%").arg(threadId).arg(value));
}

void MainWindow::onThreadFinished(int threadId)
{
    DebugOutput::out(QString("线程 %1 已完成").arg(threadId));
}

void MainWindow::onAllThreadsFinished()
{
    DebugOutput::out("所有线程已完成");
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

    Gplayer.HP = 50;

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

void MainWindow::onHotkeyChanged(int key)
{
    qDebug() << "快捷键已更改:" << key;
}

