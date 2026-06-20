#include "mousemovehandle.h"
#include "process/process.h"
#include "global/globalplayer.h"
#include "driver/msdriverpro.h"
#include "driver/mem.h"
#include "utils/helper.h"
#include "global/gameoffset.h"
#include "driver/msdriverpro.h"
#include "utils/matchtemp.h"
#include "utils/mousetrack.h"

using namespace Mem;

MouseMoveHandle::MouseMoveHandle(int id, QObject *parent)
    : IWorker(id, parent)
    , m_running(false)
    , m_paused(false)
    , m_counter(0)
{}

MouseMoveHandle::~MouseMoveHandle()
{
    qDebug() << "MouseMoveHandle" << m_id;
}

void MouseMoveHandle::start()
{
    qDebug() << "MouseMoveHandle" << m_id << "started in thread:" << QThread::currentThread();
    m_running = true;
    m_paused = true;
    m_counter = 0;
    process();
}

void MouseMoveHandle::stop()
{
    qDebug() << "MouseMoveHandle" << m_id << "stop requested";
    m_running = false;
    m_paused = false;
}

void MouseMoveHandle::pause()
{
    qDebug() << "MouseMoveHandle" << m_id << "paused";
    m_paused = true;
}

void MouseMoveHandle::resume()
{
    qDebug() << "MouseMoveHandle" << m_id << "resumed";
    m_paused = false;
}


void MouseMoveHandle::process()
{
    while (m_running) {
        // 处理暂停状态
        while (m_paused && m_running) {
            QThread::msleep(10);  // 等待恢复
        }

        if (!m_running) break;

        // 检测是否已经进入游戏
        if(Gplayer.p_gamecontext == 0){
            // 尚未进入游戏，
            ProcessHelper::programDelay(3000);
            QApplication::processEvents();
            continue;
        }

        // 检测鼠标验证
        checkMouseMove();
    }
}

void MouseMoveHandle::checkMouseMove()
{
    bool find_mousemove = false;

    quint64 text_list = readPointer(Gplayer.p_assemblybase,
                                    BASE::TMP_UpdateManager_base,
                                    BASE::TMP_UpdateManager_offset_lv1,
                                    BASE::TMP_UpdateManager_offset_lv2,
                                    BASE::TMP_UpdateManager_List);
    quint64 text_head = readLong(text_list + BASE::List::InternalArray) + BASE::InternalArray::Head;

    if(Gplayer.mousecheck_exists){

        logMouseCheck();
        // MouseTrack tracktest;
        // tracktest.realTrackTest();
        qDebug() << "滑鼠验证结束~";
        Helper::console("滑鼠验证结束~");
        ProcessHelper::programDelay(10000);
        return;
    }

    // 尋找透明圖形
    if(!Gplayer.p_assemblybase){
        qDebug() << "先运行一次，再获取数据";
        ProcessHelper::programDelay(1000);
        return;
    }

    find_mousemove = UIAntiMacroExists(text_list, text_head);

    // 判断是否存在
    if(find_mousemove){
        Gplayer.mousecheck_exists = true;

        // MatchTemp tempmatch(":assets/templat/main_pannel/mouse_move");
        // cv::Point title_coor;
        // if(tempmatch.match(title_coor)){
        //     qDebug() << "发现滑鼠验证界面！！！";
        //     Helper::console("发现滑鼠验证界面！！！");
        //     Gplayer.mousecheck_exists = true;
        //     return;
        // }
    }
    ProcessHelper::programDelay(500);
    // 控制频率
    QApplication::processEvents();
}

bool MouseMoveHandle::UIAntiMacroExists(quint64 list, quint64 head)
{
    int text_count = readInt(list + BASE::List::size);
    for(int i = 0; i < text_count; i++){
        quint64 text_mesh_pro = readLong(head + i * 8);
        QString text_string = readString(readLong(text_mesh_pro + BASE::TextMeshPro::m_text) + BASE::String::text, 100);
        if(text_string == "尋找透明圖形"){
            return true;
        }
    }
    return false;
}

void MouseMoveHandle::logMouseCheck()
{
    quint64 text_list = readPointer(Gplayer.p_assemblybase,
                                    BASE::TMP_UpdateManager_base,
                                    BASE::TMP_UpdateManager_offset_lv1,
                                    BASE::TMP_UpdateManager_offset_lv2,
                                    BASE::TMP_UpdateManager_List);
    quint64 text_head = readLong(text_list + BASE::List::InternalArray) + BASE::InternalArray::Head;

    // 时间戳
    QElapsedTimer run_start_time;
    run_start_time.start();
    int frame_index = 1;

    QDateTime logDateTime = QDateTime::currentDateTime();
    QString logTimeString = logDateTime.toString("hh-mm-ss");
    QString save_dir = QCoreApplication::applicationDirPath() + "/output/mousecheck/" + logTimeString;

    // 获取目录名和父目录
    QDir dir(save_dir);
    QString dirName = dir.dirName();
    QDir parentDir = dir;
    parentDir.cdUp();

    // 如果目录存在，先递归删除所有内容，然后删除目录本身
    if (dir.exists()) {
        // 方法1：使用 removeRecursively() 删除整个目录树（Qt 5+）
        if (!dir.removeRecursively()) {
            qWarning() << "删除目录失败:" << save_dir;
            return;
        }

        // 重新创建空目录
        if (!parentDir.mkdir(dirName)) {
            qWarning() << "创建目录失败:" << save_dir;
            return;
        }
    } else {
        // 目录不存在，直接创建
        if (!parentDir.mkdir(dirName)) {
            qWarning() << "创建目录失败:" << save_dir;
            return;
        }
    }

    HWND game_hwnd = NULL;

    game_hwnd = FindWindow(NULL, TEXT("MapleStory Worlds-Artale (繁體中文版)"));
    if (game_hwnd == NULL) {
        qCritical() << "游戏窗口不存在！";
        return;
    }

    RECT game_rect{};
    GetWindowRect(game_hwnd, &game_rect);
    int game_x = game_rect.left;
    int game_y = game_rect.top;

    QScreen* m_pScreen = QGuiApplication::primaryScreen();

    bool find_mousemove = false;

    while(true){

        find_mousemove = UIAntiMacroExists(text_list, text_head);
        if(!find_mousemove){
            qDebug() << "滑鼠界面不存在，退出";
            break;
        }

        int now_time = run_start_time.elapsed();

        if(now_time > 15000){
            qDebug() << "滑鼠界面超时，退出";
            break;
        }

        qDebug() << "---------------------------------------------------------------------------------------";
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString currentTime = currentDateTime.toString("yyyy-MM-dd hh:mm:ss");
        qDebug() << "第" << frame_index << "帧" << " " << currentTime;
        qDebug() << "---------------------------------------------------------------------------------------";

        QPixmap cur_capture = m_pScreen->grabWindow((WId)GetDesktopWindow(), game_x, game_y, 1280, 720);
        cv::Mat bgr_frame;
        QImage image = cur_capture.toImage();
        cv::Mat frame(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), image.bytesPerLine());
        cv::cvtColor(frame, bgr_frame, cv::COLOR_BGRA2BGR);
        cv::putText(bgr_frame,
                    currentTime.toStdString(),
                    cv::Point(50, 50),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.6,
                    cv::Scalar(0, 0, 255),
                    2);
        QString save_path = QString("%1/frame_%2.jpg").arg(save_dir).arg(frame_index, 6, 10, QChar('0'));
        cv::imwrite(save_path.toStdString(), bgr_frame);

        printMouseInfo();
        frame_index++;
        ProcessHelper::programDelay(500);
        ProcessHelper::handleEvents();
    }
}

void MouseMoveHandle::printMouseInfo()
{
    if(Gplayer.p_gamepid == 0 || Gplayer.p_assemblybase == 0){
        DebugOutput::out("游戏进程不存在");
        Helper::console("游戏进程不存在");
        return;
    }

    quint64 ui_anti_macro = readPointer(
        Gplayer.p_assemblybase,
        0xA8BF178,
        0xB8,
        0x0,
        0x58,
        0x20);

    qDebug() << QString("ui_anti_macro:0x%1").arg(ui_anti_macro, 0, 16);


    quint64 plan_a = ui_anti_macro;
    quint64 plan_b = ui_anti_macro + 0x160;

    // 打印数据
    qDebug() << QString("PlanA:0x%1").arg(plan_a, 0, 16);
    printPlanInfo(plan_a);
    qDebug() << QString("PlanB:0x%1").arg(plan_b, 0, 16);
    printPlanInfo(plan_b);
}

void MouseMoveHandle::printPlanInfo(quint64 plan_p)
{
    int Id = readInt(plan_p + 0x38);
    bool IsLoadComplete = readByte(plan_p + 0x3C);
    bool IsDestroy = readByte(plan_p + 0x3D);
    bool IsForceClose = readByte(plan_p + 0x3E);

    // posList
    // mousePosList
    int currentStatus = readInt(plan_p + 0xD0);
    qint64 currentTick = readLong(plan_p + 0xD8);
    qint64 readyTick = readLong(plan_p + 0xE0);
    qint64 randomCheckColorTick = readLong(plan_p + 0xE8);
    qint64 startTick = readLong(plan_p + 0xF0);
    qint64 setResultTick = readLong(plan_p + 0xF8);
    int level = readInt(plan_p + 0x100);
    QString detectId = readString(readLong(plan_p + 0x108) + 0x14,100);
    qint64 expireTime = readLong(plan_p + 0x110);
    cv::Point cursor;
    cursor.x = readInt(plan_p + 0x14C);
    cursor.y = readInt(plan_p + 0x150);

    qDebug() << "Id:" << Id;
    qDebug() << "IsLoadComplete:" << IsLoadComplete;
    qDebug() << "IsDestroy:" << IsDestroy;
    qDebug() << "IsForceClose:" << IsForceClose;
    qDebug() << "currentStatus:" << currentStatus;
    qDebug() << "currentTick:" << currentTick;
    qDebug() << "readyTick:" << readyTick;
    qDebug() << "randomCheckColorTick:" << randomCheckColorTick;
    qDebug() << "startTick:" << startTick;
    qDebug() << "setResultTick:" << setResultTick;
    qDebug() << "level:" << level;
    qDebug() << "detectId:" << detectId;
    qDebug() << "expireTime:" << expireTime;
    qDebug() << "cursor:" << cursor.x << "," << cursor.y;

    quint64 posList = readLong(plan_p + 0xC0);
    int pos_count = readInt(posList + BASE::List::size);
    quint64 pos_head = readLong(posList + BASE::List::InternalArray) + BASE::InternalArray::Head;
    std::vector<cv::Point2f> pos_list;
    for(int i = 0; i < pos_count; i++){
        quint64 cur_pos = pos_head + i * 8;
        float pos_x = readFloat(cur_pos);
        pos_x = std::round(pos_x * 100) / 100.0f;
        float pos_y = readFloat(cur_pos + 4);
        pos_y = std::round(pos_y * 100) / 100.0f;
        qDebug() << i << "{" << pos_x << "," << pos_y << "}";
        pos_list.push_back({pos_x,pos_y});
    }

    quint64 mousePosList = readLong(plan_p + 0xC8);
    int mousepos_count = readInt(mousePosList + BASE::List::size);
    quint64 mousepos_head = readLong(mousePosList + BASE::List::InternalArray) + BASE::InternalArray::Head;
    for(int i = 0; i < mousepos_count; i++){
        quint64 cur_pos = mousepos_head + i * 8;
        float pos_x = readFloat(cur_pos);
        pos_x = std::round(pos_x * 100) / 100.0f;
        float pos_y = readFloat(cur_pos + 4);
        pos_y = std::round(pos_y * 100) / 100.0f;
        qDebug() << i << "{" << pos_x << "," << pos_y << "}";
    }
}
