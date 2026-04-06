#include "monitortooltest.h"
#include "process/process.h"
#include "global/globalplayer.h"
#include "driver/msdriverpro.h"
#include "driver/mem.h"
#include "utils/helper.h"

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
    m_paused = false;
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

        // 记录当前位置
        // 检测游戏是否存在
        Gplayer.p_gamepid = MSDriverPRO::getPid("msw.exe");

        if(Gplayer.p_gamepid == 0){
            // qDebug << "";
            DebugOutput::out("游戏进程不存在");

            ProcessHelper::programDelay(3000);
            QApplication::processEvents();
            continue;
        }

        DebugOutput::out("游戏进程:" + QString::number(Gplayer.p_gamepid));
        Gplayer.p_assemblybase = MSDriverPRO::getModuleAddr(Gplayer.p_gamepid, "GameAssembly.dll");
        DebugOutput::out("模块基址:" + QString::number(Gplayer.p_assemblybase, 16).toUpper());

        Gplayer.p_entry = readPointer(Gplayer.p_assemblybase, 0x0A1E0AE0, 0xB8, 0x0, 0x10, 0x40);
        DebugOutput::out("Entry:" + QString::number(Gplayer.p_entry, 16).toUpper());
        // 检测是否已经进入游戏
        if(Gplayer.p_entry == 0){
            // 尚未进入游戏，
            ProcessHelper::programDelay(3000);
            QApplication::processEvents();
            continue;
        }

        // 如果已经进入游戏
        Gplayer.p_gamecontext = readPointer(Gplayer.p_entry, 0x10, 0x18, 0x30);
        DebugOutput::out("GameContext:" + QString::number(Gplayer.p_gamecontext, 16).toUpper());

        Gplayer.p_allServices = Mem::readPointer(Gplayer.p_gamecontext, 0x298);
        DebugOutput::out("allServices:" + QString::number(Gplayer.p_allServices, 16).toUpper());

        // 搜索p_userservice
        quint64 p_userservice = Helper::findService(Gplayer.p_allServices,"UserService");
        DebugOutput::out("UserService:" + QString::number(p_userservice, 16).toUpper());

        // 找到人物指针
        quint64 user_transform = readPointer(p_userservice,0x90,0x158);
        // qDebug() << "指针：" << QString::number(user_transform,16).toUpper();
        // user_transform = readLong(user_transform + 0x158);
        // qDebug() << "指针：" << QString::number(user_transform,16).toUpper();

        // 几率
        // std::set<std::pair<float, float>> user_pos_list;

        Gplayer.move_pos_list.reserve(1000);

        readUserData:
        // uint user_x = Math::roundHalfAwayFromZero_bit(readFloat(user_transform + 0x190)) * 100;
        // uint user_y = Math::roundHalfAwayFromZero_bit(readFloat(user_transform + 0x194)) * 100;

        float user_x = Math::roundHalfAwayFromZero_bit(readFloat(user_transform + 0x190));
        float user_y = Math::roundHalfAwayFromZero_bit(readFloat(user_transform + 0x194));

        Helper::posExists(Gplayer.move_pos_list,{user_x,user_y});

        // ProcessHelper::programDelay(20);
        // QApplication::processEvents();
        goto readUserData;

        // 控制频率
        ProcessHelper::programDelay(3000);
        QApplication::processEvents();
    }
}
