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
        ProcessHelper::programDelay(500);
        QApplication::processEvents();
    }
}

void MouseMoveHandle::checkMouseMove()
{
    if(Gplayer.mousecheck_exists){
        // return;
        MouseTrack tracktest;
        // tracktest.test();
        tracktest.realTrackTest();
        qDebug() << "滑鼠验证结束~";
        Helper::console("滑鼠验证结束~");
        return;
    }

    // 尋找透明圖形
    if(!Gplayer.p_assemblybase){
        qDebug() << "先运行一次，再获取数据";
        return;
    }

    quint64 text_list = readPointer(Gplayer.p_assemblybase,
                            BASE::TMP_UpdateManager_base,
                            BASE::TMP_UpdateManager_offset_lv1,
                            BASE::TMP_UpdateManager_offset_lv2,
                            BASE::TMP_UpdateManager_List);
    int text_count = readInt(text_list + BASE::List::size);

    quint64 text_head = readLong(text_list + BASE::List::InternalArray) + BASE::InternalArray::Head;

    bool find_mousemove = false;
    for(int i = 0; i < text_count; i++){
        quint64 text_mesh_pro = readLong(text_head + i * 8);
        QString text_string = readString(readLong(text_mesh_pro + BASE::TextMeshPro::m_text) + BASE::String::text, 100);
        // qDebug() << "内容:" << text_string;
        if(text_string == "尋找透明圖形"){
            find_mousemove = true;
            break;
        }
    }

    // 判断是否存在
    if(find_mousemove){
        MatchTemp tempmatch(":assets/templat/main_pannel/mouse_move");
        cv::Point title_coor;
        if(tempmatch.match(title_coor)){
            qDebug() << "发现滑鼠验证界面！！！";
            Helper::console("发现滑鼠验证界面！！！");
            Gplayer.mousecheck_exists = true;
        }
    }
}
