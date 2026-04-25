#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStringListModel>
#include <QDateTime>
#include <QScreen>
#include <QGuiApplication>
#include <QCursor>
#include <QPoint>
#include <QMenu>
#include <QJsonArray>

#include <vmp/VMProtectSDK.h>
#include "driver/msdriverpro.h"
#include "driver/driverload.h"
#include "global/globalconfig.h"
#include "global/gameoffset.h"
#include "global/controller.h"
#include "widgets/hotkeyedit.h"
#include "utils/helper.h"
#include "driver/mem.h"
#include "utils/gamestruct.h"
#include "widgets/alertwindow.h"
#include <QGraphicsScene>
#include "widgets/taskeditdialog.h"
#include "utils/data.h"

// 测试头文件
#include "global/globalplayer.h"

using namespace Mem;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_threadManager(new ThreadManager(this))
{
    ui->setupUi(this);

    loadDriver();

    // 初始化组件
    initCompomentUI();

    // 加载配置文件
    Gconfig.loadFromJson();

    // 设置配置组件
    initConfig();

    // 初始化目录
    initDir();

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

MainWindow *MainWindow::getInstance()
{
    static MainWindow instance;
    return &instance;
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

        #ifdef PROTECT
        // 保护自身
        MSDriverPRO::protect((ULONG64)GetCurrentProcessId());
        // 隐藏进程
        MSDriverPRO::hide((ULONG64)GetCurrentProcessId());
        m_hide_process = true;
        #endif

        // 初始化键盘状态，打开小键盘
        if ((GetKeyState(VK_NUMLOCK) & 1)) MSDriverPRO::keyboardClick(0x45);
    }
}

void MainWindow::initCompomentUI()
{
    // ui->pushButton_2->setProperty("class","danger");

    // 菜单
    ui->listWidget->setProperty("class","leftMenu");
    QListWidgetItem *item1 = new QListWidgetItem(QIcon(":/assets/images/icons/home.svg"), "启动");
    ui->listWidget->addItem(item1);
    QListWidgetItem *item2 = new QListWidgetItem(QIcon(":/assets/images/icons/setting.svg"), "计划");
    ui->listWidget->addItem(item2);
    QListWidgetItem *item3 = new QListWidgetItem(QIcon(":/assets/images/icons/vip.svg"), "配置");
    ui->listWidget->addItem(item3);
    QListWidgetItem *item4 = new QListWidgetItem(QIcon(":/assets/images/icons/sata.svg"), "数据");
    ui->listWidget->addItem(item4);
    ui->listWidget->setCurrentRow(0);

    // tab组件
    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->tabBar()->hide();

    // 日志
    ui->textEdit_log->setProperty("class","commonlog");

    // 主窗口按钮
    ui->pushButton_start_play->setDisabled(true);
    ui->pushButton_start_play->setProperty("class","start-play-btn");

    // 计划任务表格
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels({"序号","类型", "选项1", "选项2","运行时间"});
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableWidget->setColumnWidth(0, 50);
    ui->tableWidget->setColumnWidth(1, 100);
    ui->tableWidget->setColumnWidth(2, 145);
    ui->tableWidget->setColumnWidth(3, 145);
    ui->tableWidget->setColumnWidth(4, 205);

    // 计划任务监控
    ui->label_task_id_label->setProperty("class","task-key");
    ui->label_task_type_label->setProperty("class","task-key");
    ui->label_task_op1_label->setProperty("class","task-key");
    ui->label_task_op2_label->setProperty("class","task-key");
    ui->label_task_time_label->setProperty("class","task-key");
    ui->label_task_id_text->setProperty("class","task-value");
    ui->label_task_type_text->setProperty("class","task-value");
    ui->label_task_op1_text->setProperty("class","task-value");
    ui->label_task_op2_text->setProperty("class","task-value");
    ui->label_task_time_text->setProperty("class","task-value");


    // 重新设置样式表
    qApp->setStyleSheet(qApp->styleSheet());
}

void MainWindow::initConfig()
{
    // 任务队列初始化
    initTask();

    // 技能开关
    ui->checkBox_atk_switch_general->setChecked(Gconfig.getValue("atk_swtich/general").toBool());
    ui->checkBox_atk_switch_skill1->setChecked(Gconfig.getValue("atk_swtich/skill1").toBool());
    ui->checkBox_atk_switch_buff1->setChecked(Gconfig.getValue("atk_swtich/buff1").toBool());

    // 按键绑定
    ui->lineEdit_bindkey_general_atk->m_config_path = "bindkey/general_atk";
    ui->lineEdit_bindkey_general_atk->m_currentKey = Gconfig.getValue("bindkey/general_atk").toInt();
    ui->lineEdit_bindkey_general_atk->updateDisplay();

    ui->lineEdit_bindkey_skill_atk1->m_config_path = "bindkey/skill_atk1";
    ui->lineEdit_bindkey_skill_atk1->m_currentKey = Gconfig.getValue("bindkey/skill_atk1").toInt();
    ui->lineEdit_bindkey_skill_atk1->updateDisplay();

    ui->lineEdit_bindkey_buff1->m_config_path = "bindkey/buff1";
    ui->lineEdit_bindkey_buff1->m_currentKey = Gconfig.getValue("bindkey/buff1").toInt();
    ui->lineEdit_bindkey_buff1->updateDisplay();


    ui->lineEdit_bindkey_jump->m_config_path = "bindkey/jump";
    ui->lineEdit_bindkey_jump->m_currentKey = Gconfig.getValue("bindkey/jump").toInt();
    ui->lineEdit_bindkey_jump->updateDisplay();

    ui->lineEdit_bindkey_gohome->m_config_path = "bindkey/gohome";
    ui->lineEdit_bindkey_gohome->m_currentKey = Gconfig.getValue("bindkey/gohome").toInt();
    ui->lineEdit_bindkey_gohome->updateDisplay();

    // 攻击范围
    ui->lineEdit_atk_range_general_x->setText(Gconfig.getValue("atk_range/general_x").toString());
    ui->lineEdit_atk_range_general_y->setText(Gconfig.getValue("atk_range/general_y").toString());
    ui->lineEdit_atk_range_skill1_x->setText(Gconfig.getValue("atk_range/skill1_x").toString());
    ui->lineEdit_atk_range_skill1_y->setText(Gconfig.getValue("atk_range/skill1_y").toString());

    // 自动加血
    ui->checkBox_auto_add_hp->setChecked(Gconfig.getValue("auto_add_health/add_hp").toBool());
    ui->lineEdit_auto_add_hp_rate->setText(Gconfig.getValue("auto_add_health/hp_rate").toString());
    ui->lineEdit_auto_add_hp_key->m_config_path = "auto_add_health/hp_key";
    ui->lineEdit_auto_add_hp_key->m_currentKey = Gconfig.getValue("auto_add_health/hp_key").toInt();
    ui->lineEdit_auto_add_hp_key->updateDisplay();

    // 自动加蓝
    ui->checkBox_auto_add_mp->setChecked(Gconfig.getValue("auto_add_health/add_mp").toBool());
    ui->lineEdit_auto_add_mp_rate->setText(Gconfig.getValue("auto_add_health/mp_rate").toString());
    ui->lineEdit_auto_add_mp_key->m_config_path = "auto_add_health/mp_key";
    ui->lineEdit_auto_add_mp_key->m_currentKey = Gconfig.getValue("auto_add_health/mp_key").toInt();
    ui->lineEdit_auto_add_mp_key->updateDisplay();

    // 药水数量监控
    ui->checkBox_potion_num->setChecked(Gconfig.getValue("auto_add_health/potion_num_switch").toBool());
    ui->lineEdit_potion_num->setText(Gconfig.getValue("auto_add_health/potion_num").toString());

    // 技能释放频率
    ui->lineEdit_recast_buff1->setText(Gconfig.getValue("atk_recast/buff1").toString());
}

void MainWindow::initDir()
{
    // 检查图片文件夹是否存在
    QString img_dir = QCoreApplication::applicationDirPath() + "/maps/";

    QDir project_img_dir(img_dir);
    if(!project_img_dir.exists())
    {
        project_img_dir.mkdir(img_dir);
    }
}

void MainWindow::initSinalBind()
{
    // 连接配置信号槽
    connect(this, &MainWindow::saveConfig, &Gconfig, &GlobalConfig::updateConfig);
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

    // 数据更新线程
    m_threadManager->addThread(m_UpdateGameData_tid, "UpdateGameData");
    // 自动加血蓝线程
    m_threadManager->addThread(m_HealthMonitor_tid, "HealthMonitor");
    // 自动线程（寻路，攻击怪物）
    m_threadManager->addThread(m_KillMonster_tid, "KillMonster");

    // m_threadManager->addThread(m_MonitorToolTest_tid, "MonitorToolTest");
    m_threadManager->startAll();


    console("启动所有线程");
}

void MainWindow::initTask()
{
    Controller::loadTaskConfig();
    refreshTable();
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
    QMutexLocker locker(&consoleMutex);  // 自动加锁/解锁
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
    // ui->textEdit_log->moveCursor(QTextCursor::End);
    ui->textEdit_log->ensureCursorVisible();
}

void MainWindow::warning(QString msg)
{
    msg = "警报：" + msg;
    if (m_alert) {
        m_alert->close();
    }

    m_alert = new AlertWindow(msg);
    m_alert->setAttribute(Qt::WA_DeleteOnClose);
    m_alert->show();
}

void MainWindow::onApplicationQuit()
{
    DebugOutput::out("安全关闭程序中... ...");
    if(!GetKeyState(VK_NUMLOCK)) MSDriverPRO::keyboardClick(0x45); // 打开小键盘

    if(MSDriverPRO::isLoadDriver())
    {
        #ifdef PROTECT
        if(m_hide_process){
            MSDriverPRO::restore();
        }
        #endif
    }

    // 卸载驱动
    DriverLoad drvLoad;
    // 如果驱动已加载
    if (MSDriverPRO::isLoadDriver())
    {
        QString service_name = "MicrosoftGuard";
        // 停止驱动
        bool stoped = drvLoad.stopDriver(service_name);
        if(stoped)
        {
            qDebug() << "MS驱动已完成停止";
        }
        if(drvLoad.unInstallDriver(service_name)){
            qDebug() << "MS驱动已完成卸载";
        }
    }

    QString drvPath = "C:/Windows/System32/drivers/MicrosoftGuard.sys"; // 驱动文件路径

    QFile local_driver_file(drvPath);
    if(local_driver_file.exists()){

        // 判断是否可写
        if (!local_driver_file.isWritable()) {
            QFile::Permissions currentPerms = local_driver_file.permissions();

            // 添加写权限（移除非写权限）
            if (local_driver_file.setPermissions(currentPerms | QFile::WriteOwner)) {
            } else {
                qDebug() << "使用Qt修改权限失败：" << local_driver_file.errorString();
            }
        }

        bool removed = local_driver_file.remove();
        if(removed){
            qDebug() << "MS驱动文件已删除";
        }else{
            qDebug() << "MS驱动文件无法删除";
        }
    }

    // ProcessHelper::programDelay(3000);
    QApplication::processEvents();
}

void MainWindow::updateCompoment(bool status)
{
    if(status){
        ui->pushButton_start_play->setText("⏸️ 暂停 (F1)");
        ui->pushButton_start_play->setProperty("class","pause-play-btn");
    }else{
        ui->pushButton_start_play->setText("▶️ 开始 (F1)");
        ui->pushButton_start_play->setProperty("class","start-play-btn");
    }
    ui->pushButton_start_play->style()->polish(ui->pushButton_start_play);
}

void MainWindow::updateTaskMonitor(const TaskInfo &task)
{
    // qDebug() << "更新任务监控";
    // qDebug() << task.id;
    // qDebug() << task.type;
    // qDebug() << task.option1_label;
    // qDebug() << task.option1_text;

    ui->label_task_id_text->setText(QString::number(task.id));
    ui->label_task_type_text->setText(task.type);
    ui->label_task_op1_label->setText(task.option1_label);
    ui->label_task_op1_text->setText(task.option1_text);
    ui->label_task_op2_label->setText(task.option2_label);
    ui->label_task_op2_text->setText(task.option2_text);
    ui->label_task_time_text->setText(task.excute_time);
}

void MainWindow::on_pushButton_get_user_pos_clicked()
{
    VMProtectBegin("__FUNCTION__");

    quint64 m_transform = Mem::readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent);
    float user_x = Mem::readFloat(m_transform + BASE::TransformComponent::PositionX);
    float user_y = Mem::readFloat(m_transform + BASE::TransformComponent::PositionY);
    int x = RoomMap::toMapPosX(user_x,Gplayer.curmapinfo.map_bounding.x_offset);
    int y = RoomMap::toMapPosY(user_y,Gplayer.curmapinfo.map_bounding.y_offset, Gplayer.curmapinfo.map_bounding.height);
    qDebug() << x << "," << y;
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
        // 校验通过        console("自动加血比例已保存");
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

void MainWindow::onHotkeyChanged(int key)
{
    qDebug() << "快捷键已更改:" << key;
}


void MainWindow::on_pushButton_move_pos_list_clean_clicked()
{
    Gplayer.move_pos_list.clear();
    Gplayer.move_pos_list.reserve(1000);
    qDebug() << "运动轨迹已归零";
}


void MainWindow::on_pushButton_print_move_pos_list_clicked()
{
    if(Gplayer.move_pos_list.empty()){
        qDebug() << "暂时没有运动轨迹";
        return;
    }

    for(auto item : Gplayer.move_pos_list){
        // quint64 x = abs(item.first)
        uint x_offset = 777;
        uint y_offset = 599;
        uint map_height = 3137;
        // uint map_width = 6234;
        uint cur_x_left = Math::roundHalfAwayFromZero_bit(item.first) * 100 + x_offset;
        uint cur_y_left = map_height - (Math::roundHalfAwayFromZero_bit(item.second) * 100 + y_offset);

        qDebug() << item.first << "," << item.second << " " << cur_x_left << "," << cur_y_left;
    }
}


void MainWindow::on_lineEdit_atk_range_general_x_editingFinished()
{
    QString text = ui->lineEdit_atk_range_general_x->text();
    bool is_number;
    int value = text.toInt(&is_number);

    if (is_number && value >= 1 && value <= 10000) {
        // 校验通过
        console("普通攻击X范围已保存");
        Gconfig.setValue("atk_range/general_x", value);
    } else {
        showNotice("无效数值，请输入1-10000之间的数字");
        ui->lineEdit_atk_range_general_x->setText("");  // 清空内容
    }
}


void MainWindow::on_lineEdit_atk_range_general_y_editingFinished()
{
    QString text = ui->lineEdit_atk_range_general_y->text();
    bool is_number;
    int value = text.toInt(&is_number);

    if (is_number && value >= 1 && value <= 10000) {
        // 校验通过
        console("普通攻击Y范围已保存");
        Gconfig.setValue("atk_range/general_y", value);
    } else {
        showNotice("无效数值，请输入1-10000之间的数字");
        ui->lineEdit_atk_range_general_y->setText("");  // 清空内容
    }
}


void MainWindow::on_lineEdit_atk_range_skill1_x_editingFinished()
{
    QString text = ui->lineEdit_atk_range_skill1_x->text();
    bool is_number;
    int value = text.toInt(&is_number);

    if (is_number && value >= 1 && value <= 10000) {
        // 校验通过
        console("技能攻击X范围已保存");
        Gconfig.setValue("atk_range/skill1_x", value);
    } else {
        showNotice("无效数值，请输入1-10000之间的数字");
        ui->lineEdit_atk_range_skill1_x->setText("");  // 清空内容
    }
}


void MainWindow::on_lineEdit_atk_range_skill1_y_editingFinished()
{
    QString text = ui->lineEdit_atk_range_skill1_y->text();
    bool is_number;
    int value = text.toInt(&is_number);

    if (is_number && value >= 1 && value <= 10000) {
        // 校验通过
        console("技能攻击Y范围已保存");
        Gconfig.setValue("atk_range/skill1_y", value);
    } else {
        showNotice("无效数值，请输入1-10000之间的数字");
        ui->lineEdit_atk_range_skill1_y->setText("");  // 清空内容
    }
}


void MainWindow::on_pushButton_waring_alert_clicked()
{
    warning("蓝药数量不足！");
}


void MainWindow::on_pushButton_print_ui_child_clicked()
{
    // 检测游戏是否存在
    Gplayer.p_gamepid = MSDriverPRO::getPid("msw.exe");

    if(Gplayer.p_gamepid == 0){
        DebugOutput::out("游戏进程不存在");
        Helper::console("游戏进程不存在");
        return;
    }

    DebugOutput::out("游戏进程:" + QString::number(Gplayer.p_gamepid));
    Gplayer.p_assemblybase = MSDriverPRO::getModuleAddr(Gplayer.p_gamepid, "GameAssembly.dll");
    DebugOutput::out("模块基址:" + QString::number(Gplayer.p_assemblybase, 16).toUpper());

    Gplayer.p_gamecontext_entries = readPointer(Gplayer.p_assemblybase,
                                                BASE::ClientCoreThreadManager_base,
                                                BASE::ClientCoreThreadManager_offset_lv1,
                                                BASE::ClientCoreThreadManager_offset_lv2,
                                                BASE::ClientCoreThreadManager_GameDictionary,
                                                BASE::Dictionary::InternalArray_entries);
    Gplayer.p_gamecontext = readPointer(Gplayer.p_gamecontext_entries, BASE::InternalArray::Head + 0x10);
    DebugOutput::out("GameContext:" + QString::number(Gplayer.p_gamecontext, 16).toUpper());
    // 检测是否已经进入游戏
    if(Gplayer.p_gamecontext == 0){
        // 尚未进入游戏，
        DebugOutput::out("等待进入游戏中...");
        Helper::console("等待进入游戏中...");
        return;
    }

    // 游戏重要的两个模块：地图和UI
    Gplayer.p_mapsentity = readPointer(Gplayer.p_gamecontext, BASE::GameContext::MapsEntity);
    DebugOutput::out("mapsentity:" + QString::number(Gplayer.p_mapsentity, 16).toUpper());

    Gplayer.p_uientity = readPointer(Gplayer.p_gamecontext, BASE::GameContext::UIEntity);
    DebugOutput::out("uientity:" + QString::number(Gplayer.p_uientity, 16).toUpper());

    ///
    ///

    GameStruct::printEntityChild(Gplayer.p_uientity);



}


void MainWindow::on_pushButton_show_room_map_clicked()
{
    if(Gplayer.curmapinfo.map_mat.empty()){
        qDebug() << "当前地图尚未加载，请先启动";
        return;
    }
    cv::Mat map_mat = Gplayer.curmapinfo.map_mat;

    for(const auto &platform : Gplayer.curmapinfo.platform_list){
        cv::Point position(platform.start.x, platform.start.y);
        cv::putText(map_mat,std::to_string(platform.id),position,cv::FONT_HERSHEY_SIMPLEX,1.0,cv::Scalar(255, 0, 0),2);
    }

    // 保存地图
    QString img_path = QCoreApplication::applicationDirPath() + "/room_map_num.png";
    ImageTool::saveMatImage(map_mat, img_path);

    QImage qtImage(
        map_mat.data,
        map_mat.cols,
        map_mat.rows,
        map_mat.step,
        QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(qtImage);

    // 创建模态对话框
    QDialog dialog(this);
    dialog.setWindowTitle("房间地图");
    dialog.setModal(true);  // 模态

    // 创建布局和标签
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QLabel *label = new QLabel(&dialog);

    // 设置图片并缩放以适应标签
    // QPixmap pixmap = QPixmap::fromImage(qtImage);
    label->setPixmap(pixmap.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    label->setAlignment(Qt::AlignCenter);

    // 添加关闭按钮
    QPushButton *closeBtn = new QPushButton("关闭", &dialog);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    layout->addWidget(label);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);

    // 设置对话框大小
    dialog.resize(820, 650);

    // 显示模态对话框
    dialog.exec();
}


void MainWindow::on_pushButton_show_room_map_name_clicked()
{
    if(Gplayer.p_uientity == 0){
        DebugOutput::out("启动脚本后才可以获取地图名称");
        return;
    }

    quint64 ui_entity = Gplayer.p_uientity;
    quint64 main_group = Helper::findChild(ui_entity, "MainGroup");
    quint64 minimap = Helper::findChild(main_group,"Minimap");
    quint64 background = Helper::findChild(minimap,"Minimap_Background");
    quint64 scroll_top = Helper::findChild(background,"Scroll_Top");
    quint64 middle = Helper::findChild(scroll_top,"Middle");
    quint64 map_name_p = Helper::findChild(middle,"Minimap_MinText");

    QString map_name = GameStruct::readEntityText(map_name_p);
    DebugOutput::out("地图名称" + map_name);
}

void MainWindow::refreshTable()
{
    ui->tableWidget->setRowCount(Gcontroller.m_tasks.size());

    for (int i = 0; i < Gcontroller.m_tasks.size(); ++i)
    {
        const Task& t = Gcontroller.m_tasks[i];

        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(t.id)));

        QString typeStr;
        QTableWidgetItem *type_item = nullptr;
        switch (t.type)
        {
        case TaskType::CleanRoom:
            typeStr = "刷图";
            type_item = new QTableWidgetItem(typeStr);
            type_item->setIcon(QIcon(":/assets/images/icons/1.svg"));
            break;
        case TaskType::Rest:
            typeStr = "休息";
            type_item = new QTableWidgetItem(typeStr);
            type_item->setIcon(QIcon(":/assets/images/icons/2.svg"));
            break;
        case TaskType::Quest:
            typeStr = "任务";
            type_item = new QTableWidgetItem(typeStr);
            type_item->setIcon(QIcon(":/assets/images/icons/4.svg"));
            break;
        }

        ui->tableWidget->setItem(i, 1, type_item);
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(t.option1_value));
        ui->tableWidget->setItem(i, 3, new QTableWidgetItem(t.option2_value));

        QTime time = QTime::fromMSecsSinceStartOfDay(t.runTime * 1000);
        ui->tableWidget->setItem(i, 4, new QTableWidgetItem(time.toString("HH:mm:ss")));
    }
}

void MainWindow::deletePlanRow(int row)
{
    // 确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        "确定要删除第 " + QString::number(row + 1) + " 行数据吗？",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        qDebug() << "删除了第" << row << "行";
        Gcontroller.m_tasks.erase(Gcontroller.m_tasks.begin() + row);
        Controller::reSortTask();
        Controller::saveTaskConfig();
        ui->tableWidget->removeRow(row);
        refreshTable();
    }
}

void MainWindow::editPlanRow(int row)
{
    qDebug() << "编辑第" << row << "行";

    if (row < 0 || row >= Gcontroller.m_tasks.size()) return;

    TaskEditDialog dlg(this);
    dlg.setTask(Gcontroller.m_tasks[row]);

    if (dlg.exec() == QDialog::Accepted)
    {
        Gcontroller.m_tasks[row] = dlg.getTask();
        Controller::saveTaskConfig();
        refreshTable();
    }
}


void MainWindow::on_pushButton_add_plan_clicked()
{
    TaskEditDialog dlg(this);

    Task new_task;
    new_task.id = Gcontroller.m_tasks.size();
    dlg.setTask(new_task);
    if (dlg.exec() == QDialog::Accepted)
    {
        Task t = dlg.getTask();
        Gcontroller.m_tasks.push_back(t);
        Controller::saveTaskConfig();
        refreshTable();
    }
}

void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column)
{
    if (row < 0 || row >= Gcontroller.m_tasks.size()) return;

    TaskEditDialog dlg(this);
    dlg.setTask(Gcontroller.m_tasks[row]);

    if (dlg.exec() == QDialog::Accepted)
    {
        Gcontroller.m_tasks[row] = dlg.getTask();
        Controller::saveTaskConfig();
        refreshTable();
    }
}


void MainWindow::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    // 获取右键点击的行
    QTableWidgetItem *item = ui->tableWidget->itemAt(pos);
    if (item) {
        int row = item->row();

        // 创建菜单
        QMenu menu(this);

        QAction *deleteAction = menu.addAction("删除");
        QAction *editAction = menu.addAction("编辑");
        menu.addSeparator();
        QAction *cancelAction = menu.addAction("取消");

        // 显示菜单并获取选择的动作
        QAction *selectedAction = menu.exec(ui->tableWidget->viewport()->mapToGlobal(pos));

        if (selectedAction == deleteAction) {
            deletePlanRow(row);
        } else if (selectedAction == editAction) {
            editPlanRow(row);
        }
    }
}


void MainWindow::on_pushButton_print_map_child_clicked()
{
    // 检测游戏是否存在
    Gplayer.p_gamepid = MSDriverPRO::getPid("msw.exe");

    if(Gplayer.p_gamepid == 0){
        DebugOutput::out("游戏进程不存在");
        Helper::console("游戏进程不存在");
        return;
    }

    DebugOutput::out("游戏进程:" + QString::number(Gplayer.p_gamepid));
    Gplayer.p_assemblybase = MSDriverPRO::getModuleAddr(Gplayer.p_gamepid, "GameAssembly.dll");
    DebugOutput::out("模块基址:" + QString::number(Gplayer.p_assemblybase, 16).toUpper());

    Gplayer.p_gamecontext_entries = readPointer(Gplayer.p_assemblybase,
                                                BASE::ClientCoreThreadManager_base,
                                                BASE::ClientCoreThreadManager_offset_lv1,
                                                BASE::ClientCoreThreadManager_offset_lv2,
                                                BASE::ClientCoreThreadManager_GameDictionary,
                                                BASE::Dictionary::InternalArray_entries);
    Gplayer.p_gamecontext = readPointer(Gplayer.p_gamecontext_entries, BASE::InternalArray::Head + 0x10);
    DebugOutput::out("GameContext:" + QString::number(Gplayer.p_gamecontext, 16).toUpper());
    // 检测是否已经进入游戏
    if(Gplayer.p_gamecontext == 0){
        // 尚未进入游戏，
        DebugOutput::out("等待进入游戏中...");
        Helper::console("等待进入游戏中...");
        return;
    }

    // 游戏重要的两个模块：地图和UI
    Gplayer.p_mapsentity = readPointer(Gplayer.p_gamecontext, BASE::GameContext::MapsEntity);
    DebugOutput::out("mapsentity:" + QString::number(Gplayer.p_mapsentity, 16).toUpper());

    Gplayer.p_uientity = readPointer(Gplayer.p_gamecontext, BASE::GameContext::UIEntity);
    DebugOutput::out("uientity:" + QString::number(Gplayer.p_uientity, 16).toUpper());

    GameStruct::printEntityChild(Gplayer.p_mapsentity);
}


void MainWindow::on_checkBox_potion_num_stateChanged(int arg1)
{
    if(ui->checkBox_potion_num->isChecked()){
        showNotice("监控药水数量已开启");
        Gconfig.setValue("auto_add_health/potion_num_switch", true);
    }else{
        showNotice("监控药水数量已关闭");
        Gconfig.setValue("auto_add_health/potion_num_switch", false);
    }
}


void MainWindow::on_lineEdit_potion_num_editingFinished()
{
    QString text = ui->lineEdit_potion_num->text();
    bool is_number;
    int value = text.toInt(&is_number);

    if (is_number && value >= 1 && value <= 10000) {
        // 校验通过
        showNotice("监控药水数量已保存");
        Gconfig.setValue("auto_add_health/potion_num", value);
    } else {
        showNotice("无效数值，请输入1-10000之间的数字");
        ui->lineEdit_auto_add_mp_rate->setText("");  // 清空内容
    }
}


void MainWindow::on_checkBox_atk_switch_general_stateChanged(int arg1)
{
    if(ui->checkBox_atk_switch_general->isChecked()){
        showNotice("普通攻击已开启");
        Gconfig.setValue("atk_swtich/general", true);
    }else{
        showNotice("普通攻击已关闭");
        Gconfig.setValue("atk_swtich/general", false);
    }
}


void MainWindow::on_checkBox_atk_switch_skill1_stateChanged(int arg1)
{
    if(ui->checkBox_atk_switch_skill1->isChecked()){
        showNotice("技能攻击1已开启");
        Gconfig.setValue("atk_swtich/skill1", true);
    }else{
        showNotice("技能攻击1已关闭");
        Gconfig.setValue("atk_swtich/skill1", false);
    }
}


void MainWindow::on_checkBox_atk_switch_buff1_stateChanged(int arg1)
{
    if(ui->checkBox_atk_switch_buff1->isChecked()){
        showNotice("BUFF1已开启");
        Gconfig.setValue("atk_swtich/buff1", true);
    }else{
        showNotice("BUFF1已关闭");
        Gconfig.setValue("atk_swtich/buff1", false);
    }
}


void MainWindow::on_lineEdit_recast_buff1_editingFinished()
{
    QString text = ui->lineEdit_recast_buff1->text();
    bool is_number;
    int value = text.toInt(&is_number);

    if (is_number && value >= 1 && value <= 3600) {
        // 校验通过
        showNotice("BUFF1释放频率已保存");
        Gconfig.setValue("atk_recast/buff1", value);
    } else {
        showNotice("无效数值，请输入1-3600之间的数字");
        ui->lineEdit_recast_buff1->setText("");  // 清空内容
    }
}


void MainWindow::on_pushButton_show_room_map_items_clicked()
{
    Data::Game::roomMapItems();
}


void MainWindow::on_pushButton_print_map_collision_clicked()
{
    Data::Game::roomMapCollision();
}


void MainWindow::on_pushButton_runeui_sprite_clicked()
{
    Data::Game::runeUISprites();
}


void MainWindow::on_pushButton_screenshot_clicked()
{
    ImageTool::screenshot();
}


void MainWindow::on_pushButton_show_room_map_portal_clicked()
{
    Data::Game::roomMapPortal();
}


void MainWindow::on_pushButton_get_inrang_monster_clicked()
{
    Data::Game::rangeMonster();
}


void MainWindow::on_pushButton_show_room_map_npc_clicked()
{
    Data::Game::NPCScreenCoor();
}

