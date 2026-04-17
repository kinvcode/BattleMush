#include "killmonster.h"
#include <QDebug>
#include "utils/helper.h"
#include "process/process.h"
#include "global/globalplayer.h"
#include "global/gameoffset.h"
#include "driver/mem.h"
#include "utils/gamestruct.h"
#include "pathfinding/walkbase.h"
#include "driver/mskeycode.h"
#include "global/globalconfig.h"
#include "pathfinding/moveplatform.h"
#include "global/controller.h"
#include "mainwindow.h"

using namespace Mem;

KillMonster::KillMonster(int id, QObject *parent)
    : IWorker(id, parent)
    , m_running(false)
    , m_paused(false)
    , m_counter(0)
{
    qDebug() << "KillMonster" << m_id;
}

KillMonster::~KillMonster()
{
    qDebug() << "KillMonster" << m_id;
}

void KillMonster::start()
{
    qDebug() << "KillMonster" << m_id << "started in thread:" << QThread::currentThread();
    m_running = true;
    m_paused = true;
    m_counter = 0;
    process();
}

void KillMonster::stop()
{
    qDebug() << "KillMonster" << m_id << "stop requested";
    m_running = false;
    m_paused = false;
}

void KillMonster::pause()
{
    qDebug() << "KillMonster" << m_id << "paused";
    m_paused = true;
}

void KillMonster::resume()
{
    qDebug() << "KillMonster" << m_id << "resumed";

    // m_timer.restart();

    Gplayer.m_bindkey.general_atk = Gconfig.getValue("bindkey/general_atk").toInt();
    Gplayer.m_bindkey.skill_atk1 = Gconfig.getValue("bindkey/skill_atk1").toInt();
    Gplayer.m_bindkey.jump = Gconfig.getValue("bindkey/jump").toInt();
    Gplayer.m_bindkey.gohome = Gconfig.getValue("bindkey/gohome").toInt();

    int atk_general_range_x = Gconfig.getValue("atk_range/general_x").toInt();
    int atk_general_range_y = Gconfig.getValue("atk_range/general_y").toInt();
    int atk_skill1_range_x = Gconfig.getValue("atk_range/skill1_x").toInt();
    int atk_skill1_range_y = Gconfig.getValue("atk_range/skill2_y").toInt();
    Gcontroller.m_attack_range_general = {atk_general_range_x, atk_general_range_y};
    Gcontroller.m_attack_range_skill1 = {atk_skill1_range_x, atk_skill1_range_y};

    m_recast_buff1_keycode = Gconfig.getValue("bindkey/buff1").toInt();
    m_recast_buff1_enable = Gconfig.getValue("atk_swtich/buff1").toBool();
    m_recast_buff1_time = Gconfig.getValue("atk_recast/buff1").toInt();


    m_paused = false;
}

void KillMonster::process()
{
    m_timer.start();

    Gplayer.m_bindkey.general_atk = Gconfig.getValue("bindkey/general_atk").toInt();
    Gplayer.m_bindkey.skill_atk1 = Gconfig.getValue("bindkey/skill_atk1").toInt();
    Gplayer.m_bindkey.jump = Gconfig.getValue("bindkey/jump").toInt();
    Gplayer.m_bindkey.gohome = Gconfig.getValue("bindkey/gohome").toInt();

    int atk_general_range_x = Gconfig.getValue("atk_range/general_x").toInt();
    int atk_general_range_y = Gconfig.getValue("atk_range/general_y").toInt();
    int atk_skill1_range_x = Gconfig.getValue("atk_range/skill1_x").toInt();
    int atk_skill1_range_y = Gconfig.getValue("atk_range/skill2_y").toInt();
    Gcontroller.m_attack_range_general = {atk_general_range_x, atk_general_range_y};
    Gcontroller.m_attack_range_skill1 = {atk_skill1_range_x, atk_skill1_range_y};

    m_recast_buff1_keycode = Gconfig.getValue("bindkey/buff1").toInt();
    m_recast_buff1_enable = Gconfig.getValue("atk_swtich/buff1").toBool();

    // 初始化任务
    m_task_id = 0;
    m_task_excute_time = 0;
    m_task_running = false;

    // 模拟路线
    m_platform_sequence.index = 0;
    m_platform_sequence.platform_set = {};

    while (m_running) {
        // 处理暂停状态
        while (m_paused && m_running) {
            m_map_start = 0;
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

        if(m_currentmap == 0){
            m_currentmap = Gplayer.last_mapentity;
        }

        // 切换了地图
        if(m_currentmap != Gplayer.p_curmapentity){
            m_map_start = 0;
            m_currentmap = 0;
            m_allmonster.clear();
            DebugOutput::out("切换了地图");
            DebugOutput::out("首次访问的地图需要计算地图路径，请耐心等待~");
            Helper::console("切换了地图\r\n首次访问的地图需要计算地图路径，请耐心等待~");
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
            continue;
        }

        // 如果地图没有加载完毕，
        if(!Gplayer.m_map_loaded){
            m_map_start = 0;
            m_currentmap = 0;
            m_allmonster.clear();
            DebugOutput::out("地图加载中... ...");
            Helper::console("地图加载中... ...");
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
            continue;
        }

        // RetryCase:
        if(m_paused) continue;

        // 检查键盘
        if ((GetKeyState(VK_NUMLOCK) & 1)) MSDriverPRO::keyboardClick(0x45);

        // 获取当前的任务
        if(Gcontroller.m_tasks.empty())
        {
            DebugOutput::out("当前没有计划任务，即将执行智能路线模式");
            autoCleanLine();
            ProcessHelper::programDelay(100);
            QApplication::processEvents();
            continue;
        }

        if(m_task_id >= Gcontroller.m_tasks.size()){
            DebugOutput::out("计划任务ID错误，即将重置任务");
            m_task_id = 0;
            continue;
        }

        if(!m_task_running){
            m_task_type = Gcontroller.m_tasks.at(m_task_id).type;
            m_task_running = true;
            m_task_timer.start();
        }else{
            int run_time = Gcontroller.m_tasks.at(m_task_id).runTime;
            int excute_time = m_task_timer.elapsed() / 1000;
            if(excute_time > run_time){
                DebugOutput::out("计划任务已执行:" + QString::number(excute_time) + "秒");
                DebugOutput::out("本次计划任务已执行完毕，即将进行下一个计划");

                m_task_id = (m_task_id == Gcontroller.m_tasks.size() - 1) ? 0 : m_task_id + 1;
                m_task_timer.restart();
                m_task_type = Gcontroller.m_tasks.at(m_task_id).type;
                continue;
            }
        }

        TaskInfo task_info;
        task_info.id = m_task_id;

        m_task_excute_time = m_task_timer.elapsed() / 1000;
        task_info.excute_time = Helper::timestampToHI(m_task_excute_time);

        if(m_task_type == 0){
            task_info.type = "刷图";
            task_info.option1_label = "地图";

            int map_index = Gcontroller.m_tasks.at(m_task_id).option1_key;
            int path_index = Gcontroller.m_tasks.at(m_task_id).option2_key;

            // 读取地图
            MapRoomPath task_map = Gcontroller.m_clean_room_list.at(map_index);
            task_info.option1_text = task_map.name;

            // 当前地图UUID
            if(Gplayer.curmapinfo.id != task_map.uuid){
                // 前往该地图
                // moveToRoomByUUID(task_map.uuid);

                DebugOutput::out("暂不支持自动前往的地图:" + task_map.name);
                DebugOutput::out("请手动前往该地图！");
                ProcessHelper::programDelay(5000);
                QApplication::processEvents();
                continue;
            }

            // 读取路线
            m_task_room_path = task_map.paths.at(path_index).platforms;
            task_info.option2_label = "路线";
            task_info.option2_text = task_map.paths.at(path_index).name;

            // 更新任务信息
            sendTaskInfo(task_info);
            // 默认第一条路线都是自动路线
            if(path_index == 0){
                autoCleanLine();
            }else{
                m_platform_sequence.platform_set = m_task_room_path;
                m_platform_sequence.index = 0;
                // 执行路线刷图
                excutePlatformSequence();
            }
        }else if(m_task_type == 1){
            m_task_id = (m_task_id == Gcontroller.m_tasks.size() - 1) ? 0 : m_task_id + 1;
            m_task_timer.restart();
            m_task_type = Gcontroller.m_tasks.at(m_task_id).type;

            DebugOutput::out("暂不支持【休息】类型计划任务~,即将跳过此任务");
        }else if(m_task_type == 2){
            m_task_id = (m_task_id == Gcontroller.m_tasks.size() - 1) ? 0 : m_task_id + 1;
            m_task_timer.restart();
            m_task_type = Gcontroller.m_tasks.at(m_task_id).type;

            DebugOutput::out("暂不支持【NPC任务】类型计划任务~,即将跳过此任务");
        }

        DebugOutput::out("即将进行下一轮找怪");
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
    }

    qDebug() << "KillMonster" << m_id << "finished";
}

int KillMonster::userX()
{
    quint64 transform = readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent);
    float user_x = readFloat(transform + BASE::TransformComponent::PositionX);
    return RoomMap::toMapPosX(user_x,Gplayer.curmapinfo.map_bounding.x_offset);
}

int KillMonster::userY()
{
    quint64 transform = readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent);
    float user_y = readFloat(transform + BASE::TransformComponent::PositionY);
    return RoomMap::toMapPosY(user_y,Gplayer.curmapinfo.map_bounding.y_offset, Gplayer.curmapinfo.map_bounding.height);
}

cv::Point KillMonster::userPos()
{
    cv::Point pos;
    quint64 transform = readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent);
    float user_x = readFloat(transform + BASE::TransformComponent::PositionX);
    float user_y = readFloat(transform + BASE::TransformComponent::PositionY);
    pos.x = RoomMap::toMapPosX(user_x,Gplayer.curmapinfo.map_bounding.x_offset);
    pos.y = RoomMap::toMapPosY(user_y,Gplayer.curmapinfo.map_bounding.y_offset, Gplayer.curmapinfo.map_bounding.height);
    return pos;
}

quint64 KillMonster::userFoothold()
{
    return readLong(readLong(Gplayer.p_localplayer + BASE::MODEntity::RigidbodyComponent) + BASE::RigidbodyComponent::currentFoothold);
}

quint64 KillMonster::userClimb()
{
    return readLong(readLong(Gplayer.p_localplayer + BASE::MODEntity::RigidbodyComponent) + BASE::RigidbodyComponent::attachEntity);
}

quint64 KillMonster::userPlatform()
{
    quint64 user_foothold_p = userFoothold();
    if(user_foothold_p){
        int foot_id = readInt(user_foothold_p + BASE::Foothold::ID);
        return RoomMap::getFootPlatform(foot_id, Gplayer.curmapinfo.platform_list);
    }else{
        quint64 user_climb_p = userClimb();
        if(user_climb_p == 0){
            return -1;
        }

        cv::Point pos = userPos();
        return RoomMap::getClimbPlatform(pos, Gplayer.curmapinfo.platform_list, Gplayer.curmapinfo.map_mat);
    }
}

Item KillMonster::findDropItem()
{
    Item target_item;
    target_item.entity = 0;
    quint64 map_entity_count = readInt(readLong(Gplayer.p_curmapentity + BASE::MODEntity::children) + BASE::List::size);
    quint64 entity_head = readLong(readLong(Gplayer.p_curmapentity + BASE::MODEntity::children) + BASE::List::InternalArray) + BASE::InternalArray::Head;

    m_allItem.clear();
    qDebug() << "====== 刷新物品 ======";
    for(int i = m_map_start; i < map_entity_count; i++){

        quint64 cur_entity_address = entity_head + i * 8;
        quint64 entity = readLong(cur_entity_address);

        // 过滤黑名单
        if (m_black_item.find(entity) != m_black_item.end()) {
            continue;
        }

        QString name = GameStruct::readEntityName(entity);

        if(name == "EndObj"){
            m_map_start = i + 1;
            continue;
        }

        if(name != "PoolObject"){
            continue;
        }

        float item_x = readFloat(readLong(entity + BASE::MODEntity::TransformComponent)+BASE::TransformComponent::PositionX);
        float item_y = readFloat(readLong(entity + BASE::MODEntity::TransformComponent)+BASE::TransformComponent::PositionY);

        int x = RoomMap::toMapPosX(item_x, Gplayer.curmapinfo.map_bounding.x_offset);
        int y = RoomMap::toMapPosY(item_y, Gplayer.curmapinfo.map_bounding.y_offset,Gplayer.curmapinfo.map_bounding.height);

        cv::Point item_pos = RoomMap::getItemLinePos(Gplayer.curmapinfo.map_mat,{x, y});
        if(item_pos.x == -1){
            qDebug() << "物品坐标" << x << "," << y << "无法转换平台坐标，已忽略";
            continue;
        }

        int line_id = RoomMap::getPosPlatform(item_pos, Gplayer.curmapinfo.platform_list, Gplayer.curmapinfo.map_mat);
        if(line_id == -1){
            qDebug() << "物品坐标" << x << "," << y << "找不到平台，已忽略";
            continue;
        }

        target_item.pos = item_pos;
        target_item.entity = entity;
        target_item.line_id = line_id;
        m_allItem.push_back(target_item);
    }
    qDebug() << "====== 刷新物品 ======";

    if(m_allItem.empty()){
        qDebug() << "全图未发现物品！！！";
        return target_item;
    }

    // 移除失效的黑名单物品
    int user_platform = userPlatform();
    int user_x = userX();
    int distance = 999999;
    for(const auto &item : m_allItem){
        if (m_black_item.find(item.entity) != m_black_item.end()) {
            m_black_item.erase(item.entity);
        }
        // 找到离自己最近的物品
        if(item.line_id == user_platform){
            int diff = abs(user_x - item.pos.x);
            if( diff < distance){
                distance = diff;
                target_item = item;
                qDebug() << "发现本平台物品,距离自己:" << distance << "坐标:" << item.pos.x << "," << item.pos.y;
            }
        }
    }

    // 如果没找本平台物品
    if(target_item.entity == 0){
        qDebug() << "本平台未发现物品！";
        target_item = m_allItem.front();
    }

    return target_item;
}

void KillMonster::updateAllMonsters()
{
    // 注意，当前地图的childList不断在变化，需要实时读取
    quint64 map_entity_count = readInt(readLong(Gplayer.p_curmapentity + BASE::MODEntity::children) + BASE::List::size);
    quint64 entity_head = readLong(readLong(Gplayer.p_curmapentity
                                            + BASE::MODEntity::children)
                                            + BASE::List::InternalArray)
                                            + BASE::InternalArray::Head;

    m_allmonster.clear();
    for(int i = m_map_start; i < map_entity_count; i++){
        // 节省遍历时间
        quint64 cur_entity_address = entity_head + i * 8;
        quint64 entity = readLong(cur_entity_address);

        // 找到End标记位
        QString name = GameStruct::readEntityName(entity);
        if(name == "EndObj"){
            m_map_start = i + 1;
            continue;
        }
        // 也可以找玩家
        // 找怪物
        if(name != "Monster"){
            continue;
        }

        // 判断怪物是否存在
        bool exists = readByte(readLong(entity + BASE::MODEntity::RigidbodyComponent) + BASE::RigidbodyComponent::exists);
        if(!exists){
            continue;
        }

        // 存放怪物信息:怪物指针，怪物所在地板，怪物所在平台
        int foothold_id = readInt(readLong(readLong(entity + BASE::MODEntity::RigidbodyComponent)
                                           + BASE::RigidbodyComponent::currentFoothold)
                                  + BASE::Foothold::ID);
        int line_id = RoomMap::getFootPlatform(foothold_id, Gplayer.curmapinfo.platform_list);
        if(line_id == -1){
            continue;
        }

        quint64 pos_p = readLong(entity + BASE::MODEntity::RigidbodyComponent);
        float float_x = readFloat(pos_p + BASE::RigidbodyComponent::airPos);
        float float_y = readFloat(pos_p + BASE::RigidbodyComponent::airPos + 4);
        int monster_x = RoomMap::toMapPosX(float_x, Gplayer.curmapinfo.map_bounding.x_offset);
        int monster_y = RoomMap::toMapPosY(float_y, Gplayer.curmapinfo.map_bounding.y_offset,Gplayer.curmapinfo.map_bounding.height);

        if(monster_x == 0 && monster_y == 0){
            continue;
        }

        Monster cur_monster = {entity, foothold_id, line_id, {monster_x, monster_y}};
        m_allmonster.push_back(cur_monster);
    }
}

std::vector<Monster> KillMonster::getLineMonsters(int user_line)
{
    std::vector<Monster> monsters;

    for(const auto &item : m_allmonster){
        if(item.pos.x == 0 && item.pos.y == 0) continue;
        // 读取怪物坐标
        if(item.line_id == user_line){
            monsters.push_back(item);
        }
    }

    return monsters;
}

void KillMonster::killNearMonster(const std::vector<Monster> &handle_monsters)
{
    // 找到离自己最近的怪物
    // 获取人物坐标
    qDebug() << "发现本平台存在怪物，计算最近的怪物~";
    cv::Point user_pos = userPos();
    qDebug() << "人物坐标：" << user_pos.x << "," << user_pos.y;

    Monster target_monster = Math::getNearMonster(handle_monsters, user_pos.x);
    if(target_monster.entity == 0){
        qDebug() << "错误的怪物信息，放弃击杀怪物~";
        return;
    }

    qDebug() << "目标怪物：" << target_monster.line_id << "->" << QString::number(target_monster.entity,16).toUpper() << ":" << target_monster.pos.x << "," << target_monster.pos.y;
    qDebug() << "开始寻路清怪~";

    WalkBase user_move(target_monster.pos.x);
    user_move.clearMonster(target_monster);
}

void KillMonster::reciveItem(const Item &target_item)
{
    cv::Point user_pos = userPos();
    qDebug() << "人物坐标：" << user_pos.x << "," << user_pos.y;


    qDebug() << "目标物品：" << target_item.line_id << "->" << QString::number(target_item.entity,16).toUpper() << ":" << target_item.pos.x << "," << target_item.pos.y;
    qDebug() << "开始寻路拾取~";

    WalkBase user_move(target_item.pos.x);
    user_move.m_deviation = 10; // 修改一下误差范围
    if(user_move.moveToItem()){
        // 加入到黑名单
        m_black_item.insert(target_item.entity);
    }
}

void KillMonster::skillRecast()
{
    int now_time = m_timer.elapsed() / 1000;
    qDebug() << "现在时间:" << now_time;
    qDebug() << "上次释放时间" << m_recast_buff1_lasttime;
    qDebug() << "释放频率" << m_recast_buff1_time;
    qDebug() << "释放技能已过去" << (now_time - m_recast_buff1_lasttime);
    if((now_time - m_recast_buff1_lasttime) > m_recast_buff1_time || m_recast_buff1_lasttime == 0){

        DebugOutput::out("BUFF1已到释放时间，自动释放BUFF1");
        Helper::console("BUFF1已到释放时间，自动释放BUFF1");

        while(isAttaking()){
            ProcessHelper::programDelay(50);
            QApplication::processEvents();
        }

        MSDriverPRO::keyboardClick(m_recast_buff1_keycode, 1);
        Helper::console("BUFF1已按");

        m_recast_buff1_lasttime = m_timer.elapsed() / 1000;
        // ProcessHelper::programDelay(1000);
    }
}

bool KillMonster::isAttaking()
{
    return readByte(readLong(Gplayer.p_localplayer + BASE::MODEntity::StateComponent) + BASE::StateComponent::attacking) == 0;
}

void KillMonster::autoCleanLine()
{

    // 先检查BUFF
    skillRecast();

    // 智能线路
    // 获取人物所在地板
    int user_in_lineid = userPlatform();

    // 优先找物品，找物品过程中可以找怪物，真正没有怪物的时候才会去找怪物
    Item cur_item = findDropItem();
    if(cur_item.entity != 0 && cur_item.line_id != -1){
        DebugOutput::out("发现需要拾取的物品~");
        if(cur_item.line_id == user_in_lineid){
            // 拾取物品
            DebugOutput::out("人物所在平台存在物品，开始拾取~");
            reciveItem(cur_item);
        }else{
            DebugOutput::out("人物平台没有物品，开始寻路~");
            std::vector<int> paths = PathAlgo::findAStarPath(user_in_lineid, cur_item.line_id, Gplayer.curmapinfo.path_nodes);

            qDebug() << "======A星路径======";
            for(auto const &path : paths){
                qDebug() << path;
            }
            qDebug() << "======A星路径======";

            if(paths.empty()){
                qDebug() << "无法找到前往该物品平台的路径";
            }else{
                for(auto plat : paths){
                    user_in_lineid = userPlatform();
                    PathNode node = Gplayer.curmapinfo.path_nodes[user_in_lineid];
                    qDebug() << "准备前往平台:" << plat;

                    bool move_success = MovePlatform::toPlatform(node, plat);
                    if(!move_success){
                        qDebug() << "A星移动平台失败 平台:" << plat;

                        ProcessHelper::programDelay(100);
                        QApplication::processEvents();
                        return;
                    }
                }
            }
        }

        DebugOutput::out("拾取物品中...");
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        return;
    }

    // 更新全图怪物
    updateAllMonsters();

    if(user_in_lineid == -1){
        DebugOutput::out("暂不支持当前位置寻怪~");
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        return;
    }

    // 先刷本平台的怪物

    // 待处理的怪物列表
    std::vector<Monster> handle_monsters = getLineMonsters(user_in_lineid);

    if(handle_monsters.empty()){
        DebugOutput::out("当前平台不存在怪物");

        // 寻找其他平台的怪物，可以先随便找个怪物。然后生成A星路径，在路径图中如果存在怪物就击杀
        if(m_allmonster.empty()){
            DebugOutput::out("全图未存在怪物~");
            ProcessHelper::programDelay(100);
            QApplication::processEvents();
            return;
        }

        Monster monster = m_allmonster.front();

        int monster_platform = RoomMap::getFootPlatform(monster.foothold_id, Gplayer.curmapinfo.platform_list);
        if(monster_platform == -1){
            qDebug() << "怪物所在平台找不到";
            return;
        }

        std::vector<int> paths = PathAlgo::findAStarPath(user_in_lineid, monster_platform, Gplayer.curmapinfo.path_nodes);

        qDebug() << "======A星路径======";
        for(auto const &path : paths){
            qDebug() << path;
        }
        qDebug() << "======A星路径======";

        if(paths.empty()){
            qDebug() << "无法找到前往该怪物平台的路径";
            return;
        }

        for(auto plat : paths){
            user_in_lineid = userPlatform();
            PathNode node = Gplayer.curmapinfo.path_nodes[user_in_lineid];
            qDebug() << "准备前往平台:" << plat;

            bool move_success = MovePlatform::toPlatform(node, plat);
            // bool move_success = true;
            if(!move_success){
                qDebug() << "A星移动平台失败 平台:" << plat;
                ProcessHelper::programDelay(100);
                QApplication::processEvents();
                return;
            }

            handle_monsters = getLineMonsters(user_in_lineid);

            if(handle_monsters.empty()){
                continue;;
            }
            qDebug() << "当前角色所在平台:" << user_in_lineid << "存在怪物，准备清理怪物";
            return;
        }
    }

    // 击杀怪物
    if(!handle_monsters.empty()){
        killNearMonster(handle_monsters);
    }
}

void KillMonster::excutePlatformSequence()
{
    retryEnter:
    if(m_paused){
        return;
    }

    // 先检查BUFF
    skillRecast();

    qDebug() << "当前路线节点索引：" << m_platform_sequence.index;

    // 获取人物所在地板
    int user_in_lineid = userPlatform();

    int plat_count = m_platform_sequence.platform_set.size();
    if(m_platform_sequence.index >= plat_count){
        m_platform_sequence.index = 0;
        DebugOutput::out("错误的路线索引号，已强制重置索引");
        return;
    }

    int plat_id = m_platform_sequence.platform_set[m_platform_sequence.index];

    user_in_lineid = userPlatform();

    if(user_in_lineid == plat_id && plat_count > 1){
        m_platform_sequence.index = m_platform_sequence.index + 1;
        DebugOutput::out("人物平台与当前路线目标节点相同，即将前往下一个路线节点");
        return;
    }

    std::vector<int> paths = PathAlgo::findAStarPath(user_in_lineid, plat_id, Gplayer.curmapinfo.path_nodes);

    qDebug() << "======A星路径======";
    for(auto const &path : paths){
        qDebug() << path;
    }
    qDebug() << "======A星路径======";

    if(paths.empty()){
        qDebug() << "无法找到前往该平台的路径";
        return;
    }else{
        for(auto plat : paths){
            user_in_lineid = userPlatform();

            // 先找物品
            Item cur_item = findDropItem();
            if(cur_item.line_id == user_in_lineid && cur_item.entity != 0){
                DebugOutput::out("人物所在平台存在物品，开始拾取~");
                reciveItem(cur_item);
                goto retryEnter;
            }

            // 清理该平台怪物
            updateAllMonsters();
            std::vector<Monster> handle_monsters = getLineMonsters(user_in_lineid);
            if(!handle_monsters.empty()){
                DebugOutput::out("人物所在平台存在怪物，开始清怪~");
                killNearMonster(handle_monsters);
                goto retryEnter;
            }

            PathNode node = Gplayer.curmapinfo.path_nodes[user_in_lineid];
            qDebug() << "准备前往平台:" << plat;

            bool move_success = MovePlatform::toPlatform(node, plat);
            if(!move_success){
                qDebug() << "A星移动平台失败 平台:" << plat;

                ProcessHelper::programDelay(100);
                QApplication::processEvents();
                return;
            }

            user_in_lineid = userPlatform();
            if(user_in_lineid == plat_id){
                m_platform_sequence.index++;
                DebugOutput::out("已到达目标路线节点，即将前往下一个节点");
                return;
            }
        }
    }
}

void KillMonster::updateTaskID()
{
    if(Gcontroller.m_tasks.size() == 1){
        m_task_id = 0;
        return;
    }
    m_task_id = (m_task_id == Gcontroller.m_tasks.size() - 1) ? 0 : m_task_id + 1;
    return;
}

void KillMonster::sendTaskInfo(const TaskInfo &task)
{
    MainWindow::getInstance()->updateTaskMonitor(task);
}






















