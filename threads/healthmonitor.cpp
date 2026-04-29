#include "healthmonitor.h"
#include "process/process.h"
#include "global/globalconfig.h"
#include "global/globalplayer.h"
#include "driver/msdriverpro.h"
#include "utils/helper.h"
#include "global/gameoffset.h"
#include "driver/mem.h"
#include "global/controller.h"

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
    m_paused = true;
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
    Gplayer.need_hp_potion = false;
    Gplayer.need_mp_potion = false;
}

void HealthMonitor::resume()
{
    qDebug() << "HealthMonitor" << m_id << "resumed";

    m_hp_key_code = Gconfig.getValue("auto_add_health/hp_key").toInt();
    m_mp_key_code = Gconfig.getValue("auto_add_health/mp_key").toInt();
    m_paused = false;
}

void HealthMonitor::process()
{
    m_hp_key_code = Gconfig.getValue("auto_add_health/hp_key").toInt();
    m_mp_key_code = Gconfig.getValue("auto_add_health/mp_key").toInt();

    while (m_running) {
        // 处理暂停状态
        while (m_paused && m_running) {
            QThread::msleep(10);  // 等待恢复
        }

        if (!m_running) break;

        // gamecontext统一由数据更新线程维护
        quint64 gamecontext = readPointer(Gplayer.p_gamecontext_entries, BASE::InternalArray::Head + 0x10);
        if(gamecontext == 0){
            // 尚未进入游戏，
            DebugOutput::out("等待进入游戏中...");
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
            continue;
        }

        // 如果地图没有加载完毕，
        if(!Gplayer.m_map_loaded){
            DebugOutput::out("地图加载中，停止血蓝监控");
            Helper::console("地图加载中，停止血蓝监控");
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
            continue;
        }

        // 检测血量
        bool add_hp = Gconfig.getValue("auto_add_health/add_hp").toBool();
        bool add_mp = Gconfig.getValue("auto_add_health/add_mp").toBool();
        bool potion_monitor = Gconfig.getValue("auto_add_health/potion_num_switch").toBool();

        if(add_hp){
            int hp_rate = Gconfig.getValue("auto_add_health/hp_rate").toInt();
            if(Gplayer.MAX_HP != 0 && Gplayer.HP < Gplayer.MAX_HP){
                int cur_hp_rate = Gplayer.HP * 100 / Gplayer.MAX_HP;
                if(cur_hp_rate <= hp_rate){
                    qDebug() << "当前血量：" << Gplayer.HP;
                    qDebug() << "当前最大血量：" << Gplayer.MAX_HP;
                    qDebug() << "触发阈值：" << hp_rate;
                    qDebug() << "当前比例" << cur_hp_rate;

                    DebugOutput::out("检测到血量低于安全值，自动加血");
                    Helper::console("检测到血量低于安全值，自动加血");
                    MSDriverPRO::keyboardClick(m_hp_key_code, 1);
                    ProcessHelper::programDelay(1000); // 防止加两次
                }
            }
        }

        if(add_mp){
            int mp_rate = Gconfig.getValue("auto_add_health/mp_rate").toInt();
            if(Gplayer.MAX_MP != 0 && Gplayer.MP < Gplayer.MAX_MP){
                int cur_mp_rate = Gplayer.MP * 100 / Gplayer.MAX_MP;
                if(cur_mp_rate <= mp_rate){
                    qDebug() << "当前蓝量：" << Gplayer.MP;
                    qDebug() << "当前最大蓝量：" << Gplayer.MAX_MP;
                    qDebug() << "触发阈值：" << mp_rate;
                    qDebug() << "当前比例" << cur_mp_rate;
                    DebugOutput::out("检测到蓝量低于安全值，自动加蓝");
                    Helper::console("检测到蓝量低于安全值，自动加蓝");
                    MSDriverPRO::keyboardClick(m_mp_key_code, 1);
                    ProcessHelper::programDelay(1000);
                }
            }
        }


        // 检测药水数量
        if(potion_monitor){
            int potion_num = Gconfig.getValue("auto_add_health/potion_num").toInt();
            if(Gplayer.MAX_HP != 0 && Gplayer.MAX_MP != 0){
                if(Gplayer.quick_slot_pageup_count < potion_num){

                    // if(!Gplayer.show_warning){
                    //     Helper heler_tool;
                    //     heler_tool.warning("血药数量不足"+QString::number(potion_num)+"个");
                    // }

                    Gplayer.need_hp_potion = true;
                    DebugOutput::out("血药数量不足！");
                    ProcessHelper::programDelay(1000);
                    QApplication::processEvents();
                    continue;
                }else{
                    Gplayer.need_hp_potion = false;
                }

                if(Gplayer.quick_slot_pagedown_count < potion_num){
                    // if(!Gplayer.show_warning){
                    //     Helper heler_tool;
                    //     heler_tool.warning("蓝药数量不足"+QString::number(potion_num)+"个");
                    // }

                    Gplayer.need_mp_potion = true;
                    DebugOutput::out("蓝药数量不足！");
                    ProcessHelper::programDelay(1000);
                    QApplication::processEvents();
                    continue;
                }else{
                    Gplayer.need_mp_potion = false;
                }
            }
        }

        ProcessHelper::programDelay(500);
        QApplication::processEvents();
    }

    qDebug() << "HealthMonitor" << m_id << "finished";
}

bool HealthMonitor::isAttaking()
{
    return readByte(readLong(Gplayer.p_localplayer + BASE::MODEntity::StateComponent) + BASE::StateComponent::attacking) == 0;
}
