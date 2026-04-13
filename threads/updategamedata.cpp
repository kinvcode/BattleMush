#include "updategamedata.h"
#include "driver/msdriverpro.h"
#include "global/globalplayer.h"
#include <QDebug>
#include "process/process.h"
#include "utils/helper.h"
#include "driver/mem.h"
#include "global/gameoffset.h"
#include "utils/gamestruct.h"
#include "utils/mapserializer.h"
#include "driver/msdriverpro.h"

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
    m_paused = true;
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
            Helper::console("游戏进程不存在");

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
            Helper::console("等待进入游戏中...");
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

        // 角色服务
        Gplayer.p_userservice = Helper::findService(Gplayer.p_allServices, "UserService");
        DebugOutput::out("UserService:" + QString::number(Gplayer.p_collisionservice, 16).toUpper());

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
        Gplayer.curmapinfo.id = GameStruct::readEntityID(Gplayer.p_curmapentity);
        Gplayer.curmapinfo.name = GameStruct::readEntityName(Gplayer.p_curmapentity);
        // Gplayer.last_mapentity = Gplayer.p_curmapentity;
        if(Gplayer.curmapinfo.id == "e5a83e46-390a-4ecf-a069-4e786ccf22c4"){
            DebugOutput::out("处理登录...");
            Helper::console("登录功能开发中...");
            HandleLogin();
            continue;
        }

        // 寻找LocalUserGroup
        quint64 localusergroup = Helper::findChild(Gplayer.p_uientity,"LocalUserGroup");
        quint64 localuseroffgroup = Helper::findChild(Gplayer.p_uientity,"LocalUserOffGroup");
        // 寻找StatusPanel
        quint64 statuspanel = Helper::findChild(localuseroffgroup,"StatusPanel");
        if( statuspanel == 0){
            statuspanel = Helper::findChild(localusergroup,"StatusPanel");
            if( statuspanel == 0){
                DebugOutput::out("找不到statuspanel模块，确保打开过能力值面板。即将尝试打开能力值面板");
                MSDriverPRO::keyboardClick(MSKeyboard_s);
                ProcessHelper::programDelay(3000);
                QApplication::processEvents();
                continue;
            }
        }

        // 找到localPlayer
        Gplayer.p_localplayer = readLong(Gplayer.p_userservice + BASE::UserService::localPlayer);

        //
        while(Gplayer.p_gamecontext){
            if (!m_running) break;

            Gplayer.p_gamecontext = readPointer(Gplayer.p_gamecontext_entries, BASE::InternalArray::Head + 0x10);
            if(Gplayer.p_gamecontext == 0){
                DebugOutput::out("游戏已退出...");
                ProcessHelper::programDelay(1000);
                QApplication::processEvents();
                break;
            }

            Gplayer.p_curmapentity = readPointer(Gplayer.p_mapsentity,
                                                 BASE::MODEntity::children,
                                                 BASE::List::InternalArray,
                                                 BASE::InternalArray::Head + 0 * 8);

            Gplayer.curmapinfo.id = GameStruct::readEntityID(Gplayer.p_curmapentity);
            Gplayer.curmapinfo.name = GameStruct::readEntityName(Gplayer.p_curmapentity);
            if(Gplayer.curmapinfo.id == "e5a83e46-390a-4ecf-a069-4e786ccf22c4"){
                DebugOutput::out("切换到了登录界面");
                break;
            }

            if(Gplayer.p_curmapentity != Gplayer.last_mapentity){
                DebugOutput::out("切换了地图");
                Gplayer.m_map_loaded = false;
                Gplayer.last_mapentity = Gplayer.p_curmapentity;
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
    // 判断登录界面
    // 是否存在Login entity
    // 登录界面：WorldUIObjects->Main->Login
    // 选择角色界面：WorldUIObjects->CharSelect->GameStart

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
    QString map_id = GameStruct::readEntityID(Gplayer.p_curmapentity);

    RoomMap::loadLocalMap(map_id);

    if(!Gplayer.curmapinfo.path_nodes.empty() && !Gplayer.curmapinfo.map_mat.empty())
    {
        DebugOutput::out("地图数据加载成功~");
        Gplayer.m_map_loaded = true;
        return;
    }

    if(Gplayer.curmapinfo.map_bounding.width == 0){
        Gplayer.curmapinfo.map_bounding = RoomMap::getMapBounding(Gplayer.p_curmapentity);
    }

    if(Gplayer.curmapinfo.climbs.empty()){
        // 遍历当前地图攀爬物体列表(有ClimbableComponent)
       Gplayer.curmapinfo.climbs = RoomMap::makeClimbList(Gplayer.p_curmapentity, Gplayer.p_collisionservice, Gplayer.curmapinfo.map_bounding);
    }

    // 遍历地板
    if(Gplayer.curmapinfo.footholds.empty()){
        Gplayer.curmapinfo.footholds = RoomMap::makeFootholdList(Gplayer.p_curmapentity, Gplayer.curmapinfo.map_bounding);
    }

    // 1. 将所有地板ID放入到容器 footid_set里面
    std::unordered_set<int> footid_set;
    for (const auto& item : Gplayer.curmapinfo.footholds) {
        footid_set.insert(item.id);
    }

    // 第一步：拿到地形数据并绘制地图
    if(Gplayer.curmapinfo.wall_list.empty()){
        // 创建墙体列表
        Gplayer.curmapinfo.wall_list = RoomMap::makeWallList(Gplayer.curmapinfo.footholds, footid_set);
    }


    // 创建平台列表
    if(Gplayer.curmapinfo.platform_list.empty()){
        Gplayer.curmapinfo.platform_list = RoomMap::makePlatformList(
            Gplayer.curmapinfo.footholds,
            Gplayer.curmapinfo.wall_list,
            Gplayer.curmapinfo.climbs,
            Gplayer.curmapinfo.map_bounding, footid_set
        );
    }

    // 绘制地图框架
    if(Gplayer.curmapinfo.map_mat.empty()){
        Gplayer.curmapinfo.map_mat = RoomMap::makeRoomMap(
            Gplayer.curmapinfo.climbs,
            Gplayer.curmapinfo.footholds,
            Gplayer.curmapinfo.wall_list,
            Gplayer.curmapinfo.map_bounding
        );
    }

    if(Gplayer.curmapinfo.path_nodes.empty()){
        Gplayer.curmapinfo.path_nodes = RoomMap::makeNodes(
            Gplayer.curmapinfo.platform_list,
            Gplayer.curmapinfo.wall_list,
            Gplayer.curmapinfo.map_mat,
            Gplayer.curmapinfo.map_bounding
        );
    }

    // 序列化地图数据
    QString savePath = QCoreApplication::applicationDirPath() + "/maps/map_" + map_id;
    if (MapSerializer::saveToFile(Gplayer.curmapinfo, savePath)) {
        qDebug() << "地图缓存成功!";
        Gplayer.m_map_loaded = true;
    } else {
        qDebug() << "地图缓存失败!";
        Gplayer.m_map_loaded = false;
        Gplayer.last_mapentity = 0;
        return;
    }

    // QElapsedTimer astart_time;
    // astart_time.start();

    // std::vector<int> paths = PathAlgo::findAStarPath(0, 7, Gplayer.curmapinfo.path_nodes);
    // for(const auto plat : paths){
    //     qDebug() << "前往平台：" << plat;
    // }

    // quint64 astar_times = astart_time.elapsed();
    // qDebug() << "AStar算法用时:" << astar_times;
}

void UpdateGameData::UpdateMonsters()
{
    // 判断怪物
    //     +F0 StateComponent
    //         [+138]+10 状态 IDLE STAND MOVE
    //     +11C StateUpdateSpace 0同步 1客户端 2服务端

    //     MapleMonsterActionState Invalid = 0; // 0x0
    // MapleMonsterActionState Stand = 1; // 0x0
    // MapleMonsterActionState Move = 2; // 0x0
    // MapleMonsterActionState Jump = 3; // 0x0
    // MapleMonsterActionState Hit = 4; // 0x0
    // MapleMonsterActionState Die = 5; // 0x0


    // + 1D8 RigidbodyComponent
    //     +A5 0死亡 1存活 可以判断怪物是否存在
    //     +F8 当前所在地板 可以判断怪物是否存在
    //     +118 行走{当前X坐标，X行走步长} 怪物没有
    //     +120 airPos 怪物坐标
}



















