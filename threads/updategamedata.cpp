#include "updategamedata.h"
#include "driver/msdriverpro.h"
#include "global/globalplayer.h"
#include <QDebug>
#include "process/process.h"
#include "utils/helper.h"
#include "driver/mem.h"

using namespace Mem;

UpdateGameData::UpdateGameData(int id, QObject *parent)
    : IWorker(id, parent)
    , m_running(false)
    , m_paused(false)
    , m_counter(0)
{
    qDebug() << "UpdateGameData" << m_id;
}

UpdateGameData::~UpdateGameData()
{
    qDebug() << "UpdateGameData" << m_id;
}

void UpdateGameData::start()
{
    qDebug() << "UpdateGameData" << m_id << "started in thread:" << QThread::currentThread();
    m_running = true;
    m_paused = false;
    m_counter = 0;
    process();
}

void UpdateGameData::stop()
{
    qDebug() << "UpdateGameData" << m_id << "stop requested";
    m_running = false;
    m_paused = false;
}

void UpdateGameData::pause()
{
    qDebug() << "UpdateGameData" << m_id << "paused";
    m_paused = true;
}

void UpdateGameData::resume()
{
    qDebug() << "UpdateGameData" << m_id << "resumed";
    m_paused = false;
}

void UpdateGameData::process()
{
    while (m_running) {
        // 处理暂停状态
        while (m_paused && m_running) {
            QThread::msleep(10);  // 等待恢复
        }

        if (!m_running) break;


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

        Gplayer.p_mapsentity = readPointer(Gplayer.p_gamecontext, 0x148);
        DebugOutput::out("mapsentity:" + QString::number(Gplayer.p_mapsentity, 16).toUpper());

        Gplayer.p_uientity = readPointer(Gplayer.p_gamecontext, 0x150);
        DebugOutput::out("uientity:" + QString::number(Gplayer.p_uientity, 16).toUpper());

        // 寻找LocalUserGroup
        quint64 localusergroup = Helper::findChild(Gplayer.p_uientity,"LocalUserGroup");
        // 寻找StatusPanel
        if( localusergroup == 0){
            DebugOutput::out("找不到localusergroup模块");
            ProcessHelper::programDelay(3000);
            QApplication::processEvents();
            continue;
        }
        DebugOutput::out("localusergroup:" + QString::number(localusergroup, 16).toUpper());

        // 寻找StatusPanel
        quint64 statuspanel = Helper::findChild(localusergroup,"StatusPanel");
        if( statuspanel == 0){
            // 说明没有进入到游戏？
            DebugOutput::out("找不到statuspanel模块");

            // 找LocalUserOffGroup
            localusergroup = Helper::findChild(Gplayer.p_uientity,"LocalUserOffGroup");
            if(localusergroup == 0){
                DebugOutput::out("找不到LocalUserOffGroup模块");
                ProcessHelper::programDelay(3000);
                QApplication::processEvents();
                continue;
            }
            DebugOutput::out("LocalUserOffGroup:" + QString::number(localusergroup, 16).toUpper());
            statuspanel = Helper::findChild(localusergroup,"StatusPanel");
            if( statuspanel == 0){
                // 说明没有进入到游戏？
                DebugOutput::out("找不到statuspanel模块");
                ProcessHelper::programDelay(3000);
                QApplication::processEvents();
                continue;
            }
        }

        // 用户面板
        status_panel:
        // DebugOutput::out("statuspanel:" + QString::number(statuspanel, 16).toUpper());

        // 找Playername、job、Level、HP、MP
        quint64 p_playername = Helper::findChild(statuspanel,"Playername");
        quint64 p_job = Helper::findChild(statuspanel,"job");
        quint64 p_Level = Helper::findChild(statuspanel,"Level");
        quint64 p_HP = Helper::findChild(statuspanel,"HP");
        quint64 p_MP = Helper::findChild(statuspanel,"MP");
        if(!p_playername || !p_job || !p_Level || !p_HP || !p_MP){
            DebugOutput::out("找不到用户信息");
            ProcessHelper::programDelay(3000);
            QApplication::processEvents();
            continue;
        }
        // DebugOutput::out("p_playername:" + QString::number(p_playername, 16).toUpper());
        // DebugOutput::out("p_job:" + QString::number(p_job, 16).toUpper());
        // DebugOutput::out("p_Level:" + QString::number(p_Level, 16).toUpper());
        // DebugOutput::out("p_HP:" + QString::number(p_HP, 16).toUpper());
        // DebugOutput::out("p_MP:" + QString::number(p_MP, 16).toUpper());

        QString player_name = Helper::getEntityTextComponentString(p_playername);
        QString player_job = Helper::getEntityTextComponentString(p_job);
        QString player_level = Helper::getEntityTextComponentString(p_Level);
        QString player_hp = Helper::getEntityTextComponentString(p_HP);
        QString player_mp = Helper::getEntityTextComponentString(p_MP);
        // qDebug() << player_name;
        // qDebug() << player_job;
        // qDebug() << player_level;
        // qDebug() << player_hp;
        // qDebug() << player_mp;
        Helper::parseHPString(player_hp, Gplayer.HP, Gplayer.MAX_HP);
        Helper::parseHPString(player_mp, Gplayer.MP, Gplayer.MAX_MP);

        ProcessHelper::programDelay(500);
        QApplication::processEvents();
        goto status_panel;

        ProcessHelper::programDelay(3000);
        QApplication::processEvents();
    }

    qDebug() << "UpdateGameData" << m_id << "finished";
}
