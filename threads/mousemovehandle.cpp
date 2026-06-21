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
    // 提前读取数据
    quint64 UIPopupManager = readPointer(
        Gplayer.p_assemblybase,
        BASE::UIPopupManager_base,
        BASE::UIPopupManager_offset_lv1,
        BASE::UIPopupManager_offset_lv2);

    quint64 on_change_screen_size = readLong(UIPopupManager + BASE::UIPopupManager::OnChangeScreenSize);

    if(on_change_screen_size == 0){
        qDebug() << "滑鼠验证不存在";
        Gplayer.mousecheck_exists = false;
        ProcessHelper::programDelay(500);
        QApplication::processEvents();
        return;
    }

    Gplayer.mousecheck_exists = true;

    handleMouseCheck();

    // AI识别
    // MouseTrack tracktest;
    // tracktest.realTrackTest();

    qDebug() << "滑鼠验证结束~";
    Helper::console("滑鼠验证结束~");
    ProcessHelper::programDelay(1500);
}

bool MouseMoveHandle::UIAntiMacroExists(quint64 list, quint64 head)
{
    // quint64 text_list = readPointer(Gplayer.p_assemblybase,
    //                                 BASE::TMP_UpdateManager_base,
    //                                 BASE::TMP_UpdateManager_offset_lv1,
    //                                 BASE::TMP_UpdateManager_offset_lv2,
    //                                 BASE::TMP_UpdateManager_List);
    // quint64 text_head = readLong(text_list + BASE::List::InternalArray) + BASE::InternalArray::Head;


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

cv::Point2f MouseMoveHandle::to_pixel_coords(cv::Point2f pos)
{
    cv::Point2f res = {0,0};
    res.x = (pos.x - range_.x) / (range_.y - range_.x) * box_width_;
    res.y = (pos.y - range_.x) / (range_.y - range_.x) * box_height_;
    return res;
}

void MouseMoveHandle::handleMouseCheck()
{
    if(Gplayer.p_assemblybase == 0)
    {
        qDebug() << "游戏尚未初始化";
        return;
    }

    // 时间戳
    QElapsedTimer run_start_time;
    run_start_time.start();

    QDateTime logDateTime = QDateTime::currentDateTime();
    QString logTimeString = logDateTime.toString("yyyy-MM-dd-hh-mm-ss");
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

    // 提前读取数据
    quint64 UIPopupManager = readPointer(
        Gplayer.p_assemblybase,
        BASE::UIPopupManager_base,
        BASE::UIPopupManager_offset_lv1,
        BASE::UIPopupManager_offset_lv2);

    qDebug() << QString("UIPopupManager:0x%1").arg(UIPopupManager, 0, 16);

    quint64 on_change_screen_size = readLong(UIPopupManager + BASE::UIPopupManager::OnChangeScreenSize);

    if(on_change_screen_size == 0){
        qDebug() << "滑鼠验证不存在";
        return;
    }

    quint64 uiantimacro = readLong(on_change_screen_size + 0x20);

    // 读取坐标
    quint64 pos_list = readLong(uiantimacro + BASE::UIAntiMacro::posList);
    int pos_count = readInt(pos_list + BASE::List::size);
    quint64 pos_head = readLong(pos_list + BASE::List::InternalArray) + BASE::InternalArray::Head;

    std::vector<cv::Point2f> raw_pos_list;
    for(int i = 0; i < pos_count; i++){
        quint64 cur_pos = pos_head + i * 8;
        float pos_x = readFloat(cur_pos);
        pos_x = std::round(pos_x * 100) / 100.0f;
        float pos_y = readFloat(cur_pos + 4);
        pos_y = std::round(pos_y * 100) / 100.0f;
        raw_pos_list.push_back({pos_x,pos_y});
    }

    // 转换坐标
    mouse_pos_.clear();
    mouse_pos_.resize(330);
    for(int i = 0; i < 330; i++){
        mouse_pos_[i] = to_pixel_coords(raw_pos_list[i]);
    }

    // 主循环
    while(true){
        quint64 mousecheck_pannel = readLong(UIPopupManager + BASE::UIPopupManager::OnChangeScreenSize);
        if(mousecheck_pannel == 0 && false){
            qDebug() << "滑鼠验证不存在";
            return;
        }

        int currentTick = readLong(uiantimacro + BASE::UIAntiMacro::currentTick);
        if(currentTick < 0 || currentTick > 330){
            currentTick = 0;
        }

        // 创建当前帧的日志文件
        QString logFilePath = QString("%1/frame_%2.log").arg(save_dir).arg(currentTick, 6, 10, QChar('0'));
        QFile logFile(logFilePath);
        logFile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream stream(&logFile);

        int now_time = run_start_time.elapsed();

        if(now_time > 20000){
            stream << "滑鼠界面超时，退出\r\n";
            break;
        }

        // 转换成游戏屏幕坐标
        cv::Point2f raw_pos = mouse_pos_[currentTick];
        cv::Point2i cur_target_pos = cv::Point2i(
            static_cast<int>(raw_pos.x + box_pos_x_),
            static_cast<int>(raw_pos.y + box_pos_y_));

        stream << "---------------------------------------------------------------------------------------\r\n";
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString currentTime = currentDateTime.toString("yyyy-MM-dd hh:mm:ss");
        stream << "第" << currentTick << "帧" << " " << currentTime << "\r\n";
        stream << "坐标:" << cur_target_pos.x << "," << cur_target_pos.y;
        stream << "---------------------------------------------------------------------------------------\r\n";

        QPixmap cur_capture = m_pScreen->grabWindow((WId)GetDesktopWindow(),
                                                    game_x + 8,
                                                    game_y + 31,
                                                    1280,
                                                    720);
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
        cv::Point2i cursor_pos = {cur_target_pos.x - 8, cur_target_pos.y - 31};
        cv::circle(bgr_frame, cursor_pos, 10, cv::Scalar(0, 0, 0), -1);
        QString save_path = QString("%1/frame_%2.jpg").arg(save_dir).arg(currentTick, 6, 10, QChar('0'));
        cv::imwrite(save_path.toStdString(), bgr_frame);

        int target_x = game_x + cur_target_pos.x;
        int target_y = game_y + cur_target_pos.y;

        MSDriverPRO::mouseMoveTo(target_x, target_y);

        printMouseInfo(&stream);
        ProcessHelper::programDelay(10);
        ProcessHelper::handleEvents();
    }
}

void MouseMoveHandle::printMouseInfo(QTextStream* stream)
{
    if(Gplayer.p_gamepid == 0 || Gplayer.p_assemblybase == 0){
        *stream << "游戏进程不存在\r\n";
        return;
    }

    quint64 ui_anti_macro = readPointer(
        Gplayer.p_assemblybase,
        0xA8BF178,
        0xB8,
        0x0,
        0x58,
        0x20);

    *stream << QString("ui_anti_macro:0x%1").arg(ui_anti_macro, 0, 16) << "\r\n";


    quint64 plan_a = ui_anti_macro;
    quint64 plan_b = ui_anti_macro + 0x160;

    // 打印数据
    *stream << QString("PlanA:0x%1").arg(plan_a, 0, 16) << "\r\n";
    printPlanInfo(plan_a,stream);
    *stream << QString("PlanB:0x%1").arg(plan_b, 0, 16) << "\r\n";
    printPlanInfo(plan_b,stream);
}

void MouseMoveHandle::printPlanInfo(quint64 plan_p,QTextStream* stream)
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

    *stream << "Id:" << Id << "\r\n";
    *stream << "IsLoadComplete:" << IsLoadComplete << "\r\n";
    *stream << "IsDestroy:" << IsDestroy << "\r\n";
    *stream << "IsForceClose:" << IsForceClose << "\r\n";
    *stream << "currentStatus:" << currentStatus << "\r\n";
    *stream << "currentTick:" << currentTick << "\r\n";
    *stream << "readyTick:" << readyTick << "\r\n";
    *stream << "randomCheckColorTick:" << randomCheckColorTick << "\r\n";
    *stream << "startTick:" << startTick << "\r\n";
    *stream << "setResultTick:" << setResultTick << "\r\n";
    *stream << "level:" << level << "\r\n";
    *stream << "detectId:" << detectId << "\r\n";
    *stream << "expireTime:" << expireTime << "\r\n";
    *stream << "cursor:" << cursor.x << "," << cursor.y << "\r\n";

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
        *stream << i << "{" << pos_x << "," << pos_y << "}\r\n";
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
        *stream << i << "{" << pos_x << "," << pos_y << "}\r\n";
    }
}
