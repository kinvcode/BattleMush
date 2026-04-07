#include "updategamedata.h"
#include "driver/msdriverpro.h"
#include "global/globalplayer.h"
#include <QDebug>
#include "process/process.h"
#include "utils/helper.h"
#include "driver/mem.h"
#include "global/gameoffset.h"
#include "utils/gamestruct.h"

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

        Gplayer.p_gamecontext_entries = readPointer(Gplayer.p_assemblybase,
                                                    BASE::ClientCoreThreadManager_base,
                                                    BASE::ClientCoreThreadManager_offset_lv1,
                                                    BASE::ClientCoreThreadManager_offset_lv2,
                                                    BASE::ClientCoreThreadManager_GameDictionary,
                                                    BASE::Dictionary::InternalArray_entries);
        Gplayer.p_gamecontext = readPointer(Gplayer.p_gamecontext_entries, BASE::InternalArray::Head + 0x10);
        DebugOutput::out("GameContext:" + QString::number(Gplayer.p_gamecontext, 16).toUpper());
        // 检测是否已经进入游戏
        if(Gplayer.p_gamecontext == 0){
            // 尚未进入游戏，
            DebugOutput::out("等待进入游戏中...");
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
            continue;
        }

        // 服务列表
        Gplayer.p_allServices = Mem::readPointer(Gplayer.p_gamecontext, 0x298);
        DebugOutput::out("allServices:" + QString::number(Gplayer.p_allServices, 16).toUpper());

        // 碰撞服务
        Gplayer.p_collisionservice = Helper::findService(Gplayer.p_allServices,"CollisionService");
        DebugOutput::out("collisionservice:" + QString::number(Gplayer.p_collisionservice, 16).toUpper());

        // 游戏重要的两个模块：地图和UI
        Gplayer.p_mapsentity = readPointer(Gplayer.p_gamecontext, BASE::GameContext::MapsEntity);
        DebugOutput::out("mapsentity:" + QString::number(Gplayer.p_mapsentity, 16).toUpper());

        Gplayer.p_uientity = readPointer(Gplayer.p_gamecontext, BASE::GameContext::UIEntity);
        DebugOutput::out("uientity:" + QString::number(Gplayer.p_uientity, 16).toUpper());

        Gplayer.p_curmapentity = readPointer(Gplayer.p_mapsentity,
                                            BASE::MODEntity::children,
                                            BASE::List::InternalArray,
                                            BASE::InternalArray::Head + 0 * 8);


        // 判断当前是登录界面还是游戏中
        Gplayer.curmapinfo.id = GameStruct::readEntityID(Gplayer.p_mapsentity);
        Gplayer.curmapinfo.name = GameStruct::readEntityName(Gplayer.p_mapsentity);
        Gplayer.last_mapinfo = Gplayer.curmapinfo;
        if(Gplayer.curmapinfo.id == "e5a83e46-390a-4ecf-a069-4e786ccf22c4"){
            DebugOutput::out("处理登录...");
            HandleLogin();
            continue;
        }

        // 寻找LocalUserGroup
        quint64 localusergroup = Helper::findChild(Gplayer.p_uientity,"LocalUserGroup");
        quint64 localuseroffgroup = Helper::findChild(Gplayer.p_uientity,"LocalUserOffGroup");
        // 寻找StatusPanel
        quint64 statuspanel = Helper::findChild(localusergroup,"StatusPanel");
        if( statuspanel == 0){
            statuspanel = Helper::findChild(localuseroffgroup,"StatusPanel");
            if( statuspanel == 0){
                DebugOutput::out("找不到statuspanel模块");
                ProcessHelper::programDelay(3000);
                QApplication::processEvents();
                continue;
            }
        }

        //
        while(Gplayer.p_gamecontext){
            Gplayer.p_gamecontext = readPointer(Gplayer.p_gamecontext_entries, BASE::InternalArray::Head + 0x10);
            if(Gplayer.p_gamecontext == 0){
                DebugOutput::out("游戏已退出...");
                ProcessHelper::programDelay(1000);
                QApplication::processEvents();
                break;
            }

            Gplayer.curmapinfo.id = GameStruct::readEntityID(Gplayer.p_mapsentity);
            Gplayer.curmapinfo.name = GameStruct::readEntityName(Gplayer.p_mapsentity);
            if(Gplayer.curmapinfo.id == "e5a83e46-390a-4ecf-a069-4e786ccf22c4"){
                DebugOutput::out("切换到了登录界面");
                break;
            }

            if(Gplayer.curmapinfo.id != Gplayer.last_mapinfo.id){
                DebugOutput::out("切换了地图");

                // 更新地图信息
                UpdateRoomMap();
                continue;
            }


            // 更新用户信息
            UpdateUserStatus(statuspanel);


            ProcessHelper::programDelay(500); // 0.5秒更新一次数据
            QApplication::processEvents();
        }

        DebugOutput::out("当前没有事件处理... ...");
        ProcessHelper::programDelay(3000);
        QApplication::processEvents();
    }

    qDebug() << "UpdateGameData" << m_id << "finished";
}

void UpdateGameData::HandleLogin()
{
    DebugOutput::out("自动登录中... ...");
    ProcessHelper::programDelay(3000);
    QApplication::processEvents();
}


void UpdateGameData::UpdateUserStatus(quint64 statuspanel)
{
    // quint64 p_playername = Helper::findChild(statuspanel,"Playername");
    // quint64 p_job = Helper::findChild(statuspanel,"job");
    // quint64 p_Level = Helper::findChild(statuspanel,"Level");
    quint64 p_HP = Helper::findChild(statuspanel,"HP");
    quint64 p_MP = Helper::findChild(statuspanel,"MP");

    if(!p_HP || !p_MP){
        DebugOutput::out("找不到用户信息");
        ProcessHelper::programDelay(3000);
        QApplication::processEvents();
        return;
    }

    // QString player_name = Helper::getEntityTextComponentString(p_playername);
    // QString player_job = Helper::getEntityTextComponentString(p_job);
    // QString player_level = Helper::getEntityTextComponentString(p_Level);
    QString player_hp = Helper::getEntityTextComponentString(p_HP);
    QString player_mp = Helper::getEntityTextComponentString(p_MP);
    Helper::parseHPString(player_hp, Gplayer.HP, Gplayer.MAX_HP);
    Helper::parseHPString(player_mp, Gplayer.MP, Gplayer.MAX_MP);

    ProcessHelper::programDelay(500);
    QApplication::processEvents();
}

void UpdateGameData::UpdateRoomMap()
{
    // 遍历当前地图攀爬物体列表(有ClimbableComponent)
    std::vector<ClimbRect> climbs = Helper::dumpClimbList(Gplayer.p_curmapentity, Gplayer.p_collisionservice);

    // 遍历地板
    std::vector<FootholdRect> footholds = Helper::dumpFootholdList(Gplayer.p_curmapentity);
}



















