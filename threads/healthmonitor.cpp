#include "healthmonitor.h"
#include "process/process.h"
#include "global/globalconfig.h"
#include "global/globalplayer.h"
#include "driver/msdriverpro.h"
#include "utils/helper.h"
#include "global/gameoffset.h"
#include "driver/mem.h"

using namespace Mem;

HealthMonitor::HealthMonitor(int id, QObject *parent)
    : IWorker(id, parent)
    , m_running(false)
    , m_paused(false)
    , m_counter(0)
{
    qDebug() << "HealthMonitor" << m_id;
}

HealthMonitor::~HealthMonitor()
{
    qDebug() << "HealthMonitor" << m_id;
}

void HealthMonitor::start()
{
    qDebug() << "HealthMonitor" << m_id << "started in thread:" << QThread::currentThread();
    m_running = true;
    m_paused = false;
    m_counter = 0;
    process();
}

void HealthMonitor::stop()
{
    qDebug() << "HealthMonitor" << m_id << "stop requested";
    m_running = false;
    m_paused = false;
}

void HealthMonitor::pause()
{
    qDebug() << "HealthMonitor" << m_id << "paused";
    m_paused = true;
}

void HealthMonitor::resume()
{
    qDebug() << "HealthMonitor" << m_id << "resumed";
    m_paused = false;
}

void HealthMonitor::process()
{
    while (m_running) {
        // 处理暂停状态
        while (m_paused && m_running) {
            QThread::msleep(10);  // 等待恢复
        }

        if (!m_running) break;

        Gplayer.p_gamecontext = readPointer(Gplayer.p_gamecontext_entries, BASE::InternalArray::Head + 0x10);
        if(Gplayer.p_gamecontext == 0){
            // 尚未进入游戏，
            DebugOutput::out("等待进入游戏中...");
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
            continue;
        }

        // 检测血量
        int hp_rate = Gconfig.getValue("auto_add_health/hp_rate").toInt();
        if(Gplayer.MAX_HP != 0 && Gplayer.HP < Gplayer.MAX_HP){
            int cur_hp_rate =Gplayer.HP * 100 / Gplayer.MAX_HP;
            if(cur_hp_rate <= hp_rate){
                qDebug() << "当前血量：" << Gplayer.HP;
                qDebug() << "当前最大血量：" << Gplayer.MAX_HP;
                qDebug() << "触发阈值：" << hp_rate;
                qDebug() << "当前比例" << cur_hp_rate;

                qDebug() << "检测到血量低于安全值，自动加血";
                int hp_key_code = Gconfig.getValue("auto_add_health/hp_key").toInt();
                qDebug() << "驱动按键码:" << hp_key_code;
                MSDriverPRO::keyboardClick(hp_key_code, 1);
            }
        }

        int mp_rate = Gconfig.getValue("auto_add_health/mp_rate").toInt();
        if(Gplayer.MAX_MP != 0 && Gplayer.MP < Gplayer.MAX_MP){
            int cur_mp_rate = ( Gplayer.MP / Gplayer.MAX_MP) * 100;
            if(cur_mp_rate <= mp_rate){
                qDebug() << "检测到蓝量低于安全值，自动加蓝";
                int mp_key_code = Gconfig.getValue("auto_add_health/mp_key").toInt();
                qDebug() << "驱动按键码:" << mp_key_code;

                // MSDriverPRO::keyboardClick(0x49);
                MSDriverPRO::keyboardClick(mp_key_code, 1);
            }
        }

        ProcessHelper::programDelay(500);
        QApplication::processEvents();
    }

    qDebug() << "HealthMonitor" << m_id << "finished";
}
