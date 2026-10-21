#include "monitortooltest.h"
#include "process/process.h"
#include "global/globalplayer.h"
#include "driver/mem.h"
#include "utils/helper.h"
#include "global/gameoffset.h"

using namespace Mem;

MonitorToolTest::MonitorToolTest(int id, QObject *parent)
    : IWorker(id, parent)
    , m_running(false)
    , m_paused(false)
    , m_counter(0)
{}

MonitorToolTest::~MonitorToolTest()
{
    qDebug() << "MonitorToolTest" << m_id;
}

void MonitorToolTest::start()
{
    qDebug() << "MonitorToolTest" << m_id << "started in thread:" << QThread::currentThread();
    m_running = true;
    m_paused = true;
    m_counter = 0;
    process();
}

void MonitorToolTest::stop()
{
    qDebug() << "MonitorToolTest" << m_id << "stop requested";
    m_running = false;
    m_paused = false;
}

void MonitorToolTest::pause()
{
    qDebug() << "MonitorToolTest" << m_id << "paused";
    m_paused = true;
}

void MonitorToolTest::resume()
{
    qDebug() << "MonitorToolTest" << m_id << "resumed";
    m_paused = false;
}


void MonitorToolTest::process()
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

        Gplayer.p_allServices = Mem::readPointer(Gplayer.p_gamecontext, BASE::GameContext::allServices);
        DebugOutput::out("allServices:" + QString::number(Gplayer.p_allServices, 16).toUpper());

        // 搜索p_userservice
        quint64 p_userservice = Helper::findService(Gplayer.p_allServices,"UserService");
        DebugOutput::out("UserService:" + QString::number(p_userservice, 16).toUpper());

        // 找到人物指针
        quint64 user_transform = readPointer(p_userservice,BASE::UserService::localPlayer, BASE::MODEntity::TransformComponent);

        Gplayer.move_pos_list.reserve(1000);

        readUserData:
        if (!m_running) break;

        float user_x = Math::roundHalfAwayFromZero_bit(readFloat(user_transform + BASE::TransformComponent::PositionX));
        float user_y = Math::roundHalfAwayFromZero_bit(readFloat(user_transform + BASE::TransformComponent::PositionY));

        Helper::posExists(Gplayer.move_pos_list,{user_x,user_y});

        // ProcessHelper::programDelay(20);
        // QApplication::processEvents();
        goto readUserData;

        // 控制频率
        ProcessHelper::programDelay(3000);
        QApplication::processEvents();
    }
}
