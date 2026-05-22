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

        // 控制频率
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
    }
}

void MouseMoveHandle::checkMouseMove()
{
    if(Gplayer.mousecheck_exists){
        MouseTrack tracktest;
        tracktest.test();
        qDebug() << "滑鼠验证结束~";
        return;
    }

    // 判断是否存在
    MatchTemp tempmatch(":assets/templat/main_pannel/mouse_move");
    cv::Point title_coor;
    if(tempmatch.match(title_coor)){
        Gplayer.mousecheck_exists = true;
    }
}
