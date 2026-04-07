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
#include "global/gameoffset.h"
#include "threads/healthmonitor.h"
#include "threads/updategamedata.h"
#include "threads/monitortooltest.h"
#include "widgets/hotkeyedit.h"
#include "utils/helper.h"
#include "driver/mem.h"

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

    // 数据更新线程
    m_threadManager->addThread(m_UpdateGameData_tid, "UpdateGameData");
    // 自动加血蓝线程
    // m_threadManager->addThread(m_HealthMonitor_tid, "HealthMonitor");
    // 自动线程（寻路，攻击怪物）

    // m_threadManager->addThread(m_MonitorToolTest_tid, "MonitorToolTest");
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

void MainWindow::onApplicationQuit()
{
    DebugOutput::out("安全关闭程序中... ...");
    ProcessHelper::programDelay(3000);
    QApplication::processEvents();
}

void MainWindow::on_pushButton_clicked()
{
    VMProtectBegin("__FUNCTION__");
    // console(QString::number(Gplayer.HP));
    // showNotice("123456");
    // Gplayer.HP = 50;


    // 检测游戏是否存在
    Gplayer.p_gamepid = MSDriverPRO::getPid("msw.exe");

    if(Gplayer.p_gamepid == 0){
        // qDebug << "";
        DebugOutput::out("游戏进程不存在");
        return;
    }

    DebugOutput::out("游戏进程:" + QString::number(Gplayer.p_gamepid));
    Gplayer.p_assemblybase = MSDriverPRO::getModuleAddr(Gplayer.p_gamepid, "GameAssembly.dll");
    DebugOutput::out("模块基址:" + QString::number(Gplayer.p_assemblybase, 16).toUpper());

    // 如果已经进入游戏
    Gplayer.p_gamecontext_entries = Mem::readPointer(Gplayer.p_assemblybase,
                                                BASE::ClientCoreThreadManager_base,
                                                BASE::ClientCoreThreadManager_offset_lv1,
                                                BASE::ClientCoreThreadManager_offset_lv2,
                                                BASE::ClientCoreThreadManager_GameDictionary,
                                                BASE::Dictionary::InternalArray_entries);
    Gplayer.p_gamecontext = Mem::readPointer(Gplayer.p_gamecontext_entries, BASE::InternalArray::Head + 0x10);
    DebugOutput::out("GameContext:" + QString::number(Gplayer.p_gamecontext, 16).toUpper());

    Gplayer.p_mapsentity = Mem::readPointer(Gplayer.p_gamecontext, 0x148);
    DebugOutput::out("mapsentity:" + QString::number(Gplayer.p_mapsentity, 16).toUpper());

    Gplayer.p_allServices = Mem::readPointer(Gplayer.p_gamecontext, 0x298);
    DebugOutput::out("allServices:" + QString::number(Gplayer.p_allServices, 16).toUpper());

    // 获取当前地图
    quint64 p_cur_map = Mem::readPointer(Gplayer.p_mapsentity,0x3A8,0x10,0x20);
    DebugOutput::out("p_cur_map:" + QString::number(p_cur_map, 16).toUpper());

    QString cur_map_name = Mem::readString(Mem::readLong(p_cur_map + 0x270) + 0x14, 100);
    DebugOutput::out("地图名:" + cur_map_name);

    QString cur_map_uuid = Mem::readString(Mem::readLong(p_cur_map + 0x2A0) + 0x14, 100);
    DebugOutput::out("地图uuid:" + cur_map_uuid);

    // 搜索CollisionService
    quint64 p_collisionservice = Helper::findService(Gplayer.p_allServices,"CollisionService");
    DebugOutput::out("collisionservice:" + QString::number(p_collisionservice, 16).toUpper());

    // 遍历当前地图攀爬物体列表(有ClimbableComponent)
    std::vector<ClimbRect> climbs = Helper::dumpClimbList(p_cur_map, p_collisionservice);

    // 遍历地板
    std::vector<FootholdRect> footholds = Helper::dumpFootholdList(p_cur_map);

    // 获取地图边界
    quint64 p_MapComponent = Mem::readLong(p_cur_map + 0x70);
    float map_x_left = Mem::readFloat(p_MapComponent + 0x120);
    float map_y_bottom = Mem::readFloat(p_MapComponent + 0x124);
    float map_x_right = Mem::readFloat(p_MapComponent + 0x128);
    float map_y_top = Mem::readFloat(p_MapComponent + 0x12C);
    qDebug() << "地图边界:" << map_x_left << "," << map_y_bottom << " " << map_x_right << "," << map_y_top;


    for(auto item: climbs){
        qDebug () << "攀爬：" << item.x_left << "," << item.y_top << " " << item.x_right << "," << item.y_bottom;
    }

    for(auto item: footholds){
        qDebug () << "地板：" << item.id << " " << item.x_left << "," << item.y_left << " " << item.x_right << "," << item.y_right;
    }

    // 坐标转换
    int x_offset = 0,y_offset = 0;
    if(map_x_left < 0){
        x_offset = abs(map_x_left * 100);
    }
    if(map_y_bottom < 0){
        y_offset = abs(map_y_bottom * 100);
    }

    // 地图宽度
    uint map_width = map_x_right * 100 + x_offset;
    uint map_height = map_y_top * 100 + y_offset; // 同时也是Y坐标转换的计算因子
    qDebug() << "地图尺寸：" << map_width << "x" << map_height;

    // 转换攀爬物体
    struct climb_rect{
        uint x_left;
        uint y_top;
        uint x_right;
        uint y_bottom;
    };
    std::vector<climb_rect> climb_list;
    for(auto item: climbs){
        uint cur_x_left = item.x_left * 100 + x_offset;
        uint cur_y_bottom = map_height - (item.y_bottom * 100 + y_offset);
        uint cur_x_right = item.x_right * 100 + x_offset;
        uint cur_y_top = map_height - (item.y_top * 100 + y_offset);
        climb_list.push_back({cur_x_left,cur_y_top,cur_x_right,cur_y_bottom});
    }

    // 转换地板和空气墙
    struct foothold_rect{
        int ID;
        uint x_left;
        uint y_left;
        uint x_right;
        uint y_right;
    };
    std::vector<foothold_rect> foothold_list;
    for(auto item: footholds){

        item.x_left = clamp(item.x_left, map_x_left, map_x_right);
        item.x_right = clamp(item.x_right, map_x_left, map_x_right);
        item.y_left = clamp(item.y_left, map_y_bottom, map_y_top);
        item.y_right = clamp(item.y_right, map_y_bottom, map_y_top);

        uint cur_x_left = item.x_left * 100 + x_offset;
        uint cur_y_left = map_height - (item.y_left * 100 + y_offset);
        uint cur_x_right = item.x_right * 100 + x_offset;
        uint cur_y_right = map_height - (item.y_right * 100 + y_offset);

        if(cur_x_left > cur_x_right){
            uint tmp_x = cur_x_left;
            uint tmp_y = cur_y_left;
            cur_x_left = cur_x_right;
            cur_y_left = cur_y_right;
            cur_x_right = tmp_x;
            cur_y_right = tmp_y;
        }

        // 过滤掉不需要的墙体
        if(cur_x_left == 230 && cur_x_right == 230){
            continue;
        }

        if(cur_x_left == 1220 && cur_x_right == 1220){
            continue;
        }

        qDebug() << "黑色:" << cur_x_left << "," << cur_y_left << " " << cur_x_right << "," << cur_y_right;
        foothold_list.push_back({item.id,cur_x_left,cur_y_left,cur_x_right,cur_y_right});
    }


    // 1. 将所有地板ID放入到容器 id_set里面
    std::unordered_set<int> id_set;
    for (const auto& item : foothold_list) {
        id_set.insert(item.ID);
    }

    // 2. 创建容器：line_list
    std::vector<Line> line_list;

    // 3. 创建容器：wall_list
    std::vector<Wall> wall_list;

    // 对foothold_list进行遍历
    for (auto item : foothold_list) {
        // 如果元素x的id不存在于id_set中，则进入下一层循环
        if (id_set.find(item.ID) == id_set.end()) {
            continue;
        }

        // 处理墙体
        // 如果item是一个墙体(x相同，y不同)
        // item.y_left
        if (item.x_left == item.x_right &&
            item.y_left != item.y_right) {
            Wall wall;
            wall.foot_id = item.ID;
            wall.x = item.x_left;
            // 数值大的y是bottom,数值小的y是top
            wall.bottom = std::max(item.y_left, item.y_right);
            wall.top = std::min(item.y_left, item.y_right);
            wall_list.push_back(wall);

            // 将该item的id从id_set里面移除
            id_set.erase(item.ID);
            continue;  // 墙体处理完，继续下一个item
        }
    }

    for(auto wall : wall_list){
        qDebug() << "墙：" << wall.x << "," << wall.top << "," << wall.bottom;
        id_set.erase(wall.foot_id);
    }

    int line_id = 0;
    for (auto item : foothold_list) {
        // 如果元素x的id不存在于id_set中，则进入下一层循环
        if (id_set.find(item.ID) == id_set.end()) {
            continue;
        }

        // 将该item的id从id_set里面移除
        id_set.erase(item.ID);

        // 如果是直线（y相同）或者是斜线（x_left与x_right不同且y不同）
        if (item.y_left == item.y_right ||
            (item.x_left != item.x_right && item.y_left != item.y_right)) {


            if(item.x_left > item.x_right){
                qDebug() << "数据不正确，退出计算~";
                return;
            }

            Line tmp_line;

            // 创建第一个line_child
            LineChild first_child;
            first_child.foot_id = item.ID;
            first_child.start = cv::Point(item.x_left,item.y_left);
            first_child.end = cv::Point(item.x_right,item.y_right);
            first_child.length = qAbs(item.x_right - item.x_left) + qAbs(item.y_right - item.y_left);

            tmp_line.line_child.push_back(first_child);
            tmp_line.start = cv::Point(item.x_left,item.y_left);
            tmp_line.end = cv::Point(item.x_right,item.y_right);

            for (auto& other_item : foothold_list) {
                if (id_set.find(other_item.ID) == id_set.end()) {
                    continue;
                }

                if (cv::Point(other_item.x_right,other_item.y_right) == tmp_line.start) {
                    id_set.erase(other_item.ID);

                    LineChild child;
                    child.foot_id = other_item.ID;
                    child.start = cv::Point(other_item.x_left,other_item.y_left);
                    child.end = cv::Point(other_item.x_right,other_item.y_right);
                    child.length = qAbs(other_item.x_right - other_item.x_left) + qAbs(other_item.y_right - other_item.y_left);

                    // 将line_child插入到容器tmp_line.line_child前面
                    tmp_line.line_child.insert(tmp_line.line_child.begin(), child);
                    tmp_line.start = cv::Point(other_item.x_left,other_item.y_left);
                    continue;

                }

                if (cv::Point(other_item.x_left,other_item.y_left) == tmp_line.end) {
                    id_set.erase(other_item.ID);

                    LineChild child;
                    child.foot_id = other_item.ID;
                    child.start = cv::Point(other_item.x_left,other_item.y_left);
                    child.end = cv::Point(other_item.x_right,other_item.y_right);
                    child.length = qAbs(other_item.x_right - other_item.x_left) + qAbs(other_item.y_right - other_item.y_left);

                    // 将line_child插入到容器tmp_line.line_child后面
                    tmp_line.line_child.push_back(child);
                    tmp_line.end = cv::Point(other_item.x_right,other_item.y_right);
                    continue;
                }
            }

            // 注意：需要确保start的x最小
            if (tmp_line.start.x > tmp_line.end.x) {
                qDebug() << "数据不正确2，退出计算~";
                return;
            }

            tmp_line.lineid = line_id;
            line_id++;
            line_list.push_back(tmp_line);
        }
    }

    for(auto &item : line_list){
        // 遍历wall_list
        for (const auto& wall : wall_list) {
            // 检查左墙
            if (wall.x == item.start.x &&
                wall.bottom == item.start.y) {
                item.hasLeftWall = true;
            }else{
                item.hasLeftWall = false;
            }
            // 检查右墙
            if (wall.x == item.end.x &&
                wall.bottom == item.end.y) {
                item.hasRightWall = true;
            }else{
                item.hasRightWall = false;
            }
        }

        // qDebug() << "线:" << item.start.x << "," << item.end;
    }

    // 对地板进行分组
    for(auto item : line_list){
        // 找其与自己y轴相同，
        // Line tmp_line = *item;
        qDebug() << "线路:" << item.lineid << item.hasLeftWall << item.hasRightWall << item.start.x << "," << item.start.y << " " << item.end.x << "," << item.end.y;
    }

    // 开始寻路
    // 遍历线

    // 创建地图文件
    cv::Mat cur_room_map(map_height, map_width, CV_8UC3, cv::Scalar(255, 255, 255));

    // 填充攀爬
    for(auto item : climb_list){
        int x_width = item.x_right - item.x_left;
        int y_height = item.y_bottom - item.y_top;
        cv::Rect rect(item.x_left, item.y_top, x_width, y_height);
        cv::rectangle(cur_room_map, rect, cv::Scalar(0, 0xFF, 0), -1); // 用黄色填充矩形-攀爬
    }

    // 绘制地板
    for(auto item : foothold_list){
        if(item.x_left == item.x_right){
            // 用红色绘制墙壁
            cv::line(cur_room_map,cv::Point(item.x_left,item.y_left),cv::Point(item.x_right,item.y_right),cv::Scalar(0xFF,0,0));
        }else{
            // 用黑色绘制地板
            cv::line(cur_room_map,cv::Point(item.x_left,item.y_left),cv::Point(item.x_right,item.y_right),cv::Scalar(0,0,0));
        }

    }


    // 遍历线路
    qDebug() << "向左走寻路";
    for(auto item : line_list){
        qDebug() << "搜索线路：" << item.lineid;

        // 判断左边是否有墙
        if(item.hasLeftWall){
            qDebug() << "左边有墙，退出寻路";
            continue;
        }

        if(item.start.x == 0){
            qDebug() << "左边是边界，退出寻路";
            continue;
        }


        cv::Point to_left_pos = ImageTool::dropTrajectory(false,cur_room_map,item.start.x,item.start.y);
        if(to_left_pos.x != -1){
            qDebug() << "经过位置：" << to_left_pos.x << "," << to_left_pos.y;
        }

        for(auto wall_item : wall_list){
            if(ImageTool::isPointInWall(to_left_pos,wall_item)){
                qDebug() << "该点位于墙体中，不可用";
            }
        }

        bool searched_line_id = false;
        for(auto it : line_list){
            if(searched_line_id){
                break;
            }
            for (const auto& child : it.line_child)
            {
                if (ImageTool::isPointOnSegment(to_left_pos, child.start, child.end))
                {
                    // return;
                    qDebug() << "该点位于线路ID：" << it.lineid;
                    searched_line_id = true;
                    break;

                }
            }
        }
    }

    qDebug() << "向右走寻路";
    for(auto item : line_list){
        qDebug() << "搜索线路：" << item.lineid;

        // 判断右边是否有墙
        if(item.hasRightWall){
            qDebug() << "右边有墙，退出寻路";
            continue;
        }

        if(item.end.x == map_width){
            qDebug() << "右边是边界，退出寻路";
            continue;
        }

        cv::Point to_right_pos = ImageTool::dropTrajectory(true,cur_room_map,item.end.x,item.end.y);
        if(to_right_pos.x != -1){
            qDebug() << "经过位置：" << to_right_pos.x << "," << to_right_pos.y;
        }

        for(auto item : wall_list){
            if(ImageTool::isPointInWall(to_right_pos,item)){
                qDebug() << "该点位于墙体中，不可用";
            }
        }

        bool searched_line_id = false;
        for(auto it : line_list){
            if(searched_line_id){
                break;
            }
            for (const auto& child : it.line_child)
            {
                if (ImageTool::isPointOnSegment(to_right_pos, child.start, child.end))
                {
                    // return;
                    qDebug() << "该点位于线路ID：" << it.lineid;
                    searched_line_id = true;
                    break;
                }
            }
        }
    }



    QString currentDir = QCoreApplication::applicationDirPath() + "/room_map.png";
    qDebug() << "图片保存路径：" << currentDir;
    ImageTool::saveMatImage(cur_room_map, currentDir);

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


void MainWindow::on_pushButton_2_clicked()
{
    Gplayer.move_pos_list.clear();
    Gplayer.move_pos_list.reserve(1000);
    qDebug() << "运动轨迹已归零";
}


void MainWindow::on_pushButton_3_clicked()
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

