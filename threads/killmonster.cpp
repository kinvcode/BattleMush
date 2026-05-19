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
#include "utils/matchtemp.h"

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
    Gplayer.m_bindkey.skillteleport = Gconfig.getValue("bindkey/skillteleport").toInt();

    int atk_general_range_x = Gconfig.getValue("atk_range/general_x").toInt();
    int atk_general_range_y = Gconfig.getValue("atk_range/general_y").toInt();
    int atk_skill1_range_x = Gconfig.getValue("atk_range/skill1_x").toInt();
    int atk_skill1_range_y = Gconfig.getValue("atk_range/skill1_y").toInt();
    Gcontroller.m_attack_range_general = {atk_general_range_x, atk_general_range_y};
    Gcontroller.m_attack_range_skill1 = {atk_skill1_range_x, atk_skill1_range_y};

    m_recast_buff1_keycode = Gconfig.getValue("bindkey/buff1").toInt();
    m_recast_buff2_keycode = Gconfig.getValue("bindkey/buff2").toInt();
    m_recast_buff3_keycode = Gconfig.getValue("bindkey/buff3").toInt();
    m_recast_buff4_keycode = Gconfig.getValue("bindkey/buff4").toInt();
    m_recast_buff5_keycode = Gconfig.getValue("bindkey/buff5").toInt();
    m_recast_buff1_enable = Gconfig.getValue("atk_swtich/buff1").toBool();
    m_recast_buff2_enable = Gconfig.getValue("atk_swtich/buff2").toBool();
    m_recast_buff3_enable = Gconfig.getValue("atk_swtich/buff3").toBool();
    m_recast_buff4_enable = Gconfig.getValue("atk_swtich/buff4").toBool();
    m_recast_buff5_enable = Gconfig.getValue("atk_swtich/buff5").toBool();
    m_recast_buff1_time = Gconfig.getValue("atk_recast/buff1").toInt();
    m_recast_buff2_time = Gconfig.getValue("atk_recast/buff2").toInt();
    m_recast_buff3_time = Gconfig.getValue("atk_recast/buff3").toInt();
    m_recast_buff4_time = Gconfig.getValue("atk_recast/buff4").toInt();
    m_recast_buff5_time = Gconfig.getValue("atk_recast/buff5").toInt();
    m_recast_buff1_max_time = Gconfig.getValue("atk_recast/buff1_max").toInt();
    m_recast_buff2_max_time = Gconfig.getValue("atk_recast/buff2_max").toInt();
    m_recast_buff3_max_time = Gconfig.getValue("atk_recast/buff3_max").toInt();
    m_recast_buff4_max_time = Gconfig.getValue("atk_recast/buff4_max").toInt();
    m_recast_buff5_max_time = Gconfig.getValue("atk_recast/buff5_max").toInt();

    Gplayer.use_skillteleport = Gconfig.getValue("atk_swtich/skillteleport").toBool();

    m_paused = false;
}

void KillMonster::process()
{
    m_timer.start();

    Gplayer.m_bindkey.general_atk = Gconfig.getValue("bindkey/general_atk").toInt();
    Gplayer.m_bindkey.skill_atk1 = Gconfig.getValue("bindkey/skill_atk1").toInt();
    Gplayer.m_bindkey.jump = Gconfig.getValue("bindkey/jump").toInt();
    Gplayer.m_bindkey.gohome = Gconfig.getValue("bindkey/gohome").toInt();
    Gplayer.m_bindkey.skillteleport = Gconfig.getValue("bindkey/skillteleport").toInt();

    int atk_general_range_x = Gconfig.getValue("atk_range/general_x").toInt();
    int atk_general_range_y = Gconfig.getValue("atk_range/general_y").toInt();
    int atk_skill1_range_x = Gconfig.getValue("atk_range/skill1_x").toInt();
    int atk_skill1_range_y = Gconfig.getValue("atk_range/skill1_y").toInt();
    Gcontroller.m_attack_range_general = {atk_general_range_x, atk_general_range_y};
    Gcontroller.m_attack_range_skill1 = {atk_skill1_range_x, atk_skill1_range_y};

    m_recast_buff1_keycode = Gconfig.getValue("bindkey/buff1").toInt();
    m_recast_buff2_keycode = Gconfig.getValue("bindkey/buff2").toInt();
    m_recast_buff3_keycode = Gconfig.getValue("bindkey/buff3").toInt();
    m_recast_buff4_keycode = Gconfig.getValue("bindkey/buff4").toInt();
    m_recast_buff5_keycode = Gconfig.getValue("bindkey/buff5").toInt();
    m_recast_buff1_enable = Gconfig.getValue("atk_swtich/buff1").toBool();
    m_recast_buff2_enable = Gconfig.getValue("atk_swtich/buff2").toBool();
    m_recast_buff3_enable = Gconfig.getValue("atk_swtich/buff3").toBool();
    m_recast_buff4_enable = Gconfig.getValue("atk_swtich/buff4").toBool();
    m_recast_buff5_enable = Gconfig.getValue("atk_swtich/buff5").toBool();
    m_recast_buff1_time = Gconfig.getValue("atk_recast/buff1").toInt();
    m_recast_buff2_time = Gconfig.getValue("atk_recast/buff2").toInt();
    m_recast_buff3_time = Gconfig.getValue("atk_recast/buff3").toInt();
    m_recast_buff4_time = Gconfig.getValue("atk_recast/buff4").toInt();
    m_recast_buff5_time = Gconfig.getValue("atk_recast/buff5").toInt();
    m_recast_buff1_max_time = Gconfig.getValue("atk_recast/buff1_max").toInt();
    m_recast_buff2_max_time = Gconfig.getValue("atk_recast/buff2_max").toInt();
    m_recast_buff3_max_time = Gconfig.getValue("atk_recast/buff3_max").toInt();
    m_recast_buff4_max_time = Gconfig.getValue("atk_recast/buff4_max").toInt();
    m_recast_buff5_max_time = Gconfig.getValue("atk_recast/buff5_max").toInt();

    Gplayer.use_skillteleport = Gconfig.getValue("atk_swtich/skillteleport").toBool();

    // 初始化任务
    m_task_id = 0;
    m_task_excute_time = 0;

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
            DebugOutput::out("地图加载中,停止自动");
            Helper::console("地图加载中,停止自动");
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
            continue;
        }

        // 检查键盘
        if ((GetKeyState(VK_NUMLOCK) & 1)) MSDriverPRO::keyboardClick(0x45);

        if(Gplayer.DIED){
            // 处理死亡
            handleDied();
        }

        if(Gplayer.mousecheck_exists){
            DebugOutput::out("出现滑鼠验证");
            handleMousemove();
            ProcessHelper::programDelay(100);
            QApplication::processEvents();
            continue;
        }

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

        m_task_type = Gcontroller.m_tasks.at(m_task_id).type;
        m_task_timer.restart();
        m_task_info.id = m_task_id;

        if(m_task_type == 0){
            taskCleanRoom();
        }else if(m_task_type == 1){
            taskRest();
        }else if(m_task_type == 2){
            taskQuest();
        }

        DebugOutput::out("主线程循环体进入下一次循环");
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
    // qDebug() << "====== 刷新物品 ======";
    for(int i = m_map_start; i < map_entity_count; i++){

        quint64 cur_entity_address = entity_head + i * 8;
        quint64 entity = readLong(cur_entity_address);

        QString name = GameStruct::readEntityName(entity);

        if(name == "EndObj"){
            m_map_start = i + 1;
            continue;
        }

        if(name != "PoolObject"){
            continue;
        }

        // qDebug() << "物品:" << QString::number(entity,16).toUpper();
        // qDebug() << "UUID:" << GameStruct::readEntityID(entity);


        // 如果物品已经消失
        QString _SpriteRUID = readString(readLong(readLong(entity + BASE::MODEntity::SpriteRendererComponent)
            + BASE::SpriteRendererComponent::_SpriteRUID) + BASE::String::text,100);
        if(_SpriteRUID == "7eba29245d2f4e7fb157a9dbe160a241"){
            // qDebug() << "物品已消失~";
            continue;
        }

        // // 过滤黑名单
        if(m_black_item_pro.find(entity) != m_black_item_pro.end()){
            if(m_timer.elapsed() - m_black_item_pro[entity] > 5000){
                // qDebug() << "存在黑名单中超过5秒，移除黑名单";
                m_black_item_pro.erase(entity);
                Gplayer.m_black_item_pro = m_black_item_pro;
            }else{
                // qDebug() << "该物品在黑名单";
                continue;
            }
        }

        float item_x = readFloat(readLong(entity + BASE::MODEntity::TransformComponent)+BASE::TransformComponent::PositionX);
        float item_y = readFloat(readLong(entity + BASE::MODEntity::TransformComponent)+BASE::TransformComponent::PositionY);
        // qDebug() << "浮点坐标:" << item_x << "," << item_y;

        int x = RoomMap::toMapPosX(item_x, Gplayer.curmapinfo.map_bounding.x_offset);
        int y = RoomMap::toMapPosY(item_y, Gplayer.curmapinfo.map_bounding.y_offset,Gplayer.curmapinfo.map_bounding.height);

        cv::Point item_pos = RoomMap::getItemLinePos(Gplayer.curmapinfo.map_mat,{x, y});
        if(item_pos.x == -1){
            qDebug() << "物品无法转换平台坐标，已忽略";
            continue;
        }

        // qDebug() << "物品坐标" << x << "," << y;

        int line_id = RoomMap::getPosPlatform(item_pos, Gplayer.curmapinfo.platform_list, Gplayer.curmapinfo.map_mat);
        if(line_id == -1){
            qDebug() << "物品坐标" << x << "," << y << "找不到平台，已忽略";
            continue;
        }

        // qDebug() << "物品平台" << line_id;

        target_item.pos = item_pos;
        target_item.entity = entity;
        target_item.line_id = line_id;
        m_allItem.push_back(target_item);
    }
    // qDebug() << "====== 刷新物品 ======";

    if(m_allItem.empty()){
        qDebug() << "全图未发现物品！！！";
        return target_item;
    }

    // 移除失效的黑名单物品
    int user_platform = userPlatform();
    int user_x = userX();
    int distance = 999999;
    for(const auto &item : m_allItem){
        if (m_black_item_pro.find(item.entity) != m_black_item_pro.end()) {
            m_black_item_pro.erase(item.entity);
            Gplayer.m_black_item_pro = m_black_item_pro;
        }
        // 找到离自己最近的物品
        if(item.line_id == user_platform){
            int diff = abs(user_x - item.pos.x);
            if( diff < distance){
                distance = diff;
                target_item = item;
                // qDebug() << "发现本平台物品,距离自己:" << distance << "坐标:" << item.pos.x << "," << item.pos.y;
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
    // qDebug() << "发现本平台存在怪物，计算最近的怪物~";
    cv::Point user_pos = userPos();
    // qDebug() << "人物坐标：" << user_pos.x << "," << user_pos.y;

    Monster target_monster = Math::getNearMonster(handle_monsters, user_pos.x);
    if(target_monster.entity == 0){
        qDebug() << "错误的怪物信息，放弃击杀怪物~";
        return;
    }

    // qDebug() << "目标怪物：" << target_monster.line_id << "->" << QString::number(target_monster.entity,16).toUpper() << ":" << target_monster.pos.x << "," << target_monster.pos.y;
    // qDebug() << "开始寻路清怪~";

    WalkBase user_move(target_monster.pos.x);
    user_move.clearMonster(target_monster);
}

void KillMonster::reciveItem(const Item &target_item)
{
    // cv::Point user_pos = userPos();
    // qDebug() << "人物坐标：" << user_pos.x << "," << user_pos.y;


    qDebug() << "目标物品：" << target_item.line_id << "->" << QString::number(target_item.entity,16).toUpper() << ":" << target_item.pos.x << "," << target_item.pos.y;
    // qDebug() << "UUID:" << GameStruct::readEntityID(target_item.entity);
    // qDebug() << "开始寻路拾取~";

    WalkBase user_move(target_item.pos.x);
    user_move.m_deviation = 10; // 修改一下误差范围
    if(user_move.moveToItem()){
        // 加入到黑名单
        m_black_item_pro[target_item.entity] = m_timer.elapsed();
        Gplayer.m_black_item_pro = m_black_item_pro;
    }
}

void KillMonster::skillRecast()
{
    int now_time = m_timer.elapsed() / 1000;

    // qDebug() << "现在时间:" << now_time;
    // qDebug() << "上次释放时间" << m_recast_buff1_lasttime;
    // qDebug() << "释放频率" << m_recast_buff1_time;
    // qDebug() << "释放技能已过去" << (now_time - m_recast_buff1_lasttime);

    int compare_time = 999999;
    if(m_recast_buff1_enable){
        now_time = m_timer.elapsed() / 1000;
        if (m_recast_buff1_time > 0 && m_recast_buff1_max_time > m_recast_buff1_time) {
            compare_time = QRandomGenerator::global()->bounded(m_recast_buff1_time, m_recast_buff1_max_time + 1);
        }
        if((now_time - m_recast_buff1_lasttime) > compare_time || m_recast_buff1_lasttime == 0){

            DebugOutput::out("BUFF1已到释放时间，自动释放BUFF1");
            Helper::console("BUFF1已到释放时间，自动释放BUFF1");

            while(isAttaking()){
                ProcessHelper::programDelay(50);
                QApplication::processEvents();
            }

            MSDriverPRO::keyboardClick(m_recast_buff1_keycode, 1);
            Helper::console("BUFF1已按");

            m_recast_buff1_lasttime = m_timer.elapsed() / 1000;
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
        }
    }

    if(m_recast_buff2_enable){
        now_time = m_timer.elapsed() / 1000;
        if (m_recast_buff2_time > 0 && m_recast_buff2_max_time > m_recast_buff2_time) {
            compare_time = QRandomGenerator::global()->bounded(m_recast_buff2_time, m_recast_buff2_max_time + 1);
        }
        if((now_time - m_recast_buff2_lasttime) > compare_time || m_recast_buff2_lasttime == 0){

            DebugOutput::out("BUFF2已到释放时间，自动释放BUFF2");
            Helper::console("BUFF2已到释放时间，自动释放BUFF2");

            while(isAttaking()){
                ProcessHelper::programDelay(50);
                QApplication::processEvents();
            }

            MSDriverPRO::keyboardClick(m_recast_buff2_keycode, 1);
            Helper::console("BUFF1已按");

            m_recast_buff2_lasttime = m_timer.elapsed() / 1000;
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
        }
    }

    if(m_recast_buff3_enable){
        now_time = m_timer.elapsed() / 1000;
        if (m_recast_buff3_time > 0 && m_recast_buff3_max_time > m_recast_buff3_time) {
            compare_time = QRandomGenerator::global()->bounded(m_recast_buff3_time, m_recast_buff3_max_time + 1);
        }
        if((now_time - m_recast_buff3_lasttime) > compare_time || m_recast_buff3_lasttime == 0){

            DebugOutput::out("BUFF3已到释放时间，自动释放BUFF3");
            Helper::console("BUFF3已到释放时间，自动释放BUFF3");

            while(isAttaking()){
                ProcessHelper::programDelay(50);
                QApplication::processEvents();
            }

            MSDriverPRO::keyboardClick(m_recast_buff3_keycode, 1);
            Helper::console("BUFF1已按");

            m_recast_buff3_lasttime = m_timer.elapsed() / 1000;
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
        }
    }

    if(m_recast_buff4_enable){
        now_time = m_timer.elapsed() / 1000;
        if (m_recast_buff4_time > 0 && m_recast_buff4_max_time > m_recast_buff4_time) {
            compare_time = QRandomGenerator::global()->bounded(m_recast_buff4_time, m_recast_buff4_max_time + 1);
        }
        if((now_time - m_recast_buff4_lasttime) > compare_time || m_recast_buff4_lasttime == 0){

            DebugOutput::out("BUFF4已到释放时间，自动释放BUFF4");
            Helper::console("BUFF4已到释放时间，自动释放BUFF4");

            while(isAttaking()){
                ProcessHelper::programDelay(50);
                QApplication::processEvents();
            }

            MSDriverPRO::keyboardClick(m_recast_buff4_keycode, 1);
            Helper::console("BUFF4已按");

            m_recast_buff4_lasttime = m_timer.elapsed() / 1000;
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
        }
    }

    if(m_recast_buff5_enable){
        now_time = m_timer.elapsed() / 1000;
        if (m_recast_buff5_time > 0 && m_recast_buff5_max_time > m_recast_buff5_time) {
            compare_time = QRandomGenerator::global()->bounded(m_recast_buff5_time, m_recast_buff5_max_time + 1);
        }
        if((now_time - m_recast_buff5_lasttime) > compare_time || m_recast_buff5_lasttime == 0){

            DebugOutput::out("BUFF5已到释放时间，自动释放BUFF5");
            Helper::console("BUFF5已到释放时间，自动释放BUFF5");

            while(isAttaking()){
                ProcessHelper::programDelay(50);
                QApplication::processEvents();
            }

            MSDriverPRO::keyboardClick(m_recast_buff5_keycode, 1);
            Helper::console("BUFF1已按");

            m_recast_buff5_lasttime = m_timer.elapsed() / 1000;
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
        }
    }
}

bool KillMonster::isAttaking()
{
    return readByte(readLong(Gplayer.p_localplayer + BASE::MODEntity::StateComponent) + BASE::StateComponent::attacking) == 0;
}

bool KillMonster::isExistsRune()
{
    quint64 map_entity_count = readInt(readLong(Gplayer.p_curmapentity + BASE::MODEntity::children) + BASE::List::size);
    quint64 entity_head = readLong(readLong(Gplayer.p_curmapentity + BASE::MODEntity::children) + BASE::List::InternalArray) + BASE::InternalArray::Head;

    for(int i = m_map_start; i < map_entity_count; i++){

        quint64 cur_entity_address = entity_head + i * 8;
        quint64 entity = readLong(cur_entity_address);

        QString name = GameStruct::readEntityName(entity);

        if(name == "EndObj"){
            m_map_start = i + 1;
            continue;
        }

        if(name == "Rune"){
            quint64 p_realease = Helper::findChild(entity, "ReleaseEffect");
            if(p_realease != 0){

                // 找到地板
                int foothold_id = readInt(readLong(readLong(entity
                    + BASE::MODEntity::RigidbodyComponent)
                    + BASE::RigidbodyComponent::currentFoothold)
                    + BASE::Foothold::ID);
                int line_id = RoomMap::getFootPlatform(foothold_id, Gplayer.curmapinfo.platform_list);
                if(line_id == -1){
                    qDebug() << "无法计算符文所在平台";
                    return false;
                }

                quint64 pos_p = readLong(entity + BASE::MODEntity::RigidbodyComponent);
                float float_x = readFloat(pos_p + BASE::RigidbodyComponent::airPos);
                float float_y = readFloat(pos_p + BASE::RigidbodyComponent::airPos + 4);
                int rune_x = RoomMap::toMapPosX(float_x, Gplayer.curmapinfo.map_bounding.x_offset);
                int rune_y = RoomMap::toMapPosY(float_y, Gplayer.curmapinfo.map_bounding.y_offset,Gplayer.curmapinfo.map_bounding.height);

                if(rune_x == 0 && rune_y == 0){
                    qDebug() << "无法转换符文坐标";
                    return false;
                }

                // 记录当前符文位置：平台坐标
                m_rune_info.platform = line_id;
                m_rune_info.area = {rune_x - 10, rune_x + 10};
                return true;
            }
        }
    }

    return false;
}

void KillMonster::solveRune()
{
retryEnter:
    if(m_paused || !m_running){
        return;
    }
    // DebugOutput::out("处理解除符文~，暂时没有相关解决方式");

    // 前往符文位置：平台坐标
    int platform = m_rune_info.platform;
    cv::Point area = m_rune_info.area;

    int user_in_lineid;
    user_in_lineid = userPlatform();

    if(user_in_lineid == -1){
        qDebug() << "人物可能在空中~";
        ProcessHelper::programDelay(500);
        goto retryEnter;
    }

    if(user_in_lineid == platform){
        WalkBase walk_to_platform(0);
        qDebug() << "寻路到符文位置:" << area.x << "," << area.y;
        Gcontroller.m_disable_attack = true;
        bool success = walk_to_platform.moveToPlatform(area); // 走到范围
        Gcontroller.m_disable_attack = false;
        if(success){
            qDebug() << "已到达符文解除位置";
            runneKeyboard(); // 符文按键
            if(!isExistsRune()){
                qDebug() << "符文封印已解除！";
                return;
            }
            ProcessHelper::programDelay(500);
            QApplication::processEvents();
            goto retryEnter;
        }
        qDebug() << "移动符文位置失败";
        ProcessHelper::programDelay(500);
        goto retryEnter;
    }

    std::vector<int> paths = PathAlgo::findAStarPath(user_in_lineid, platform, Gplayer.curmapinfo.path_nodes);
    qDebug() << "======A星路径======";
    for(auto const &path : paths){
        qDebug() << path;
        if(path == -1){
            ProcessHelper::programDelay(500);
            goto retryEnter;
        }
    }
    qDebug() << "======A星路径======";

    if(paths.empty()){
        qDebug() << "无法找到前往该平台的路径";
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        goto retryEnter;
    }

    for(auto plat : paths){
        user_in_lineid = userPlatform();

        PathNode node = Gplayer.curmapinfo.path_nodes[user_in_lineid];
        qDebug() << "准备前往平台:" << plat;
        Gcontroller.m_disable_attack = true;
        bool move_success = MovePlatform::toPlatform(node, plat);
        Gcontroller.m_disable_attack = false;

        user_in_lineid = userPlatform();
        if(user_in_lineid == platform){
            DebugOutput::out("A星已到达目标路线节点");
            goto retryEnter;
        }

        if(!move_success){
            qDebug() << "A星移动平台失败 平台:" << plat;
            ProcessHelper::programDelay(100);
            QApplication::processEvents();
            goto retryEnter;
        }
    }

    DebugOutput::out("A星到达路线节点失败~，重新执行路线");
    ProcessHelper::programDelay(100);
    QApplication::processEvents();
    goto retryEnter;
}

void KillMonster::runneKeyboard()
{
    // 按上键触发指令
    MSDriverPRO::keyboardClick(MSKeyboard_UpArrow,1);

    ProcessHelper::programDelay(300);
    QApplication::processEvents();
    ImageTool::screenshot();

    quint64 alarm_group = Helper::findChild(Gplayer.p_uientity, "LocalUserAlarmGroup");
    quint64 rune_ui = Helper::findChild(alarm_group,"RuneUI");

    // 如果没有，则重新触发
    if(rune_ui == 0){
        DebugOutput::out("未显示解除符文封印指令~");
        return;
    }

    DebugOutput::out("RuneUI:" + QString::number(rune_ui,16).toUpper());

    quint64 sprite1 = Helper::findChild(rune_ui,"Sprite1");
    quint64 sprite2 = Helper::findChild(rune_ui,"Sprite2");
    quint64 sprite3 = Helper::findChild(rune_ui,"Sprite3");
    quint64 sprite4 = Helper::findChild(rune_ui,"Sprite4");

    DebugOutput::out("Sprite1:" + QString::number(sprite1,16).toUpper());
    DebugOutput::out("Sprite2:" + QString::number(sprite2,16).toUpper());
    DebugOutput::out("Sprite3:" + QString::number(sprite3,16).toUpper());
    DebugOutput::out("Sprite4:" + QString::number(sprite4,16).toUpper());

    // 找到图片ID
    QString sprite1_dataid = readString(readLong(readLong(readLong(
        sprite1 + BASE::MODEntity::SpriteGUIRendererComponent)
        +BASE::SpriteGUIRendererComponent::MODDataRef)
        +BASE::MODDataRef::dataId) + BASE::String::text, 100);

    QString sprite2_dataid = readString(readLong(readLong(readLong(
        sprite2 + BASE::MODEntity::SpriteGUIRendererComponent)
        +BASE::SpriteGUIRendererComponent::MODDataRef)
        +BASE::MODDataRef::dataId) + BASE::String::text, 100);

    QString sprite3_dataid = readString(readLong(readLong(readLong(
        sprite3 + BASE::MODEntity::SpriteGUIRendererComponent)
        +BASE::SpriteGUIRendererComponent::MODDataRef)
        +BASE::MODDataRef::dataId) + BASE::String::text, 100);

    QString sprite4_dataid = readString(readLong(readLong(readLong(
        sprite4 + BASE::MODEntity::SpriteGUIRendererComponent)
        +BASE::SpriteGUIRendererComponent::MODDataRef)
        +BASE::MODDataRef::dataId) + BASE::String::text, 100);

    DebugOutput::out("sprite1_dataid:" + sprite1_dataid);
    DebugOutput::out("sprite2_dataid:" + sprite2_dataid);
    DebugOutput::out("sprite3_dataid:" + sprite3_dataid);
    DebugOutput::out("sprite4_dataid:" + sprite4_dataid);

    std::vector<QString> arrows;
    arrows.push_back(sprite1_dataid);
    arrows.push_back(sprite2_dataid);
    arrows.push_back(sprite3_dataid);
    arrows.push_back(sprite4_dataid);

    // 依次按键
    for(int i = 0; i < 4; i++){
        if(arrows[i] == "467d3efd79eb46368dcb17a6d891ca1a"){
            qDebug() << "指令：左键";
            MSDriverPRO::keyboardClick(MSKeyboard_LeftArrow,1);
        }else if(arrows[i] == "bdcd80c306884d58be078cd31b33ac02"){
            qDebug() << "指令：右键";
            MSDriverPRO::keyboardClick(MSKeyboard_RightArrow,1);
        }else if(arrows[i] == "d74418eb49f44475953e75c7df39f01f"){
            qDebug() << "指令：上键";
            MSDriverPRO::keyboardClick(MSKeyboard_UpArrow,1);
        }else if(arrows[i] == "997c024d824e4c60995d461f718f1e86"){
            qDebug() << "指令：下键";
            MSDriverPRO::keyboardClick(MSKeyboard_DownArrow,1);
        }

        ProcessHelper::randomDelay(300,500);
        QApplication::processEvents();
    }

    ProcessHelper::programDelay(500);
    QApplication::processEvents();
}

void KillMonster::autoCleanLine()
{
    continue_recive:
    if(m_paused || !m_running) return;

    // bool test_func = false;
    // if(test_func){

    //     int user_in_lineid = userPlatform();
    //     int plat = 0;
    //     if(user_in_lineid == 2){
    //         plat = 3;
    //     }else if(user_in_lineid == 3){
    //         plat = 2;
    //     }else{
    //         qDebug() << "无法传送~";
    //         ProcessHelper::programDelay(1500);
    //         QApplication::processEvents();
    //         return;
    //     }

    //     PathNode node = Gplayer.curmapinfo.path_nodes[user_in_lineid];
    //     qDebug() << "准备前往平台:" << plat;
    //     Gcontroller.m_disable_attack = true;
    //     bool move_success = MovePlatform::toPlatform(node, plat);
    //     if(move_success){
    //         qDebug() << "移动成功！！！";
    //     }

    //     qDebug() << "测试完成~";
    //     ProcessHelper::programDelay(1500);
    //     QApplication::processEvents();
    //     return;
    // }

    if(!Gcontroller.m_tasks.empty())
    {
        // 更新任务信息
        m_task_info.excute_time = Helper::timestampToHI(m_task_excute_time);
        sendTaskInfo(m_task_info);

        m_task_excute_time = m_task_timer.elapsed() / 1000;
        int run_time = Gcontroller.m_tasks.at(m_task_id).runTime;
        if(m_task_excute_time > run_time){
            DebugOutput::out("计划任务已执行:" + QString::number(m_task_excute_time) + "秒");
            DebugOutput::out("本次计划任务已执行完毕，即将进行下一个计划");
            m_task_id = (m_task_id == Gcontroller.m_tasks.size() - 1) ? 0 : m_task_id + 1;
            return;
        }
    }

    // 检查药水
    if(Gplayer.need_hp_potion || Gplayer.need_mp_potion)
    {
        qDebug() << "缺少药水，准备前往商店";
        buyPotion();
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        return;
    }

    if(Gplayer.DIED){
        // 处理死亡
        handleDied();
    }

    if(Gplayer.mousecheck_exists){
        DebugOutput::out("出现滑鼠验证");
        handleMousemove();
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        goto continue_recive;
    }

    // 先检查BUFF
    skillRecast();

    // 符文检测
    if(isExistsRune()){
        qDebug() << "检测到解除符文封印，尝试解除中...";
        solveRune();
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        goto continue_recive;
    }

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

        cur_item = findDropItem();
        if(cur_item.entity != 0 && cur_item.line_id != -1){
            DebugOutput::out("拾取物品中...");
            ProcessHelper::programDelay(100);
            QApplication::processEvents();
            goto continue_recive;
        }
        // return;
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
    if(m_paused || !m_running){
        return;
    }

    // m_platform_sequence.index = 0;

    // 更新任务信息
    m_task_info.excute_time = Helper::timestampToHI(m_task_excute_time);
    sendTaskInfo(m_task_info);

    m_task_excute_time = m_task_timer.elapsed() / 1000;
    int run_time = Gcontroller.m_tasks.at(m_task_id).runTime;
    if(m_task_excute_time > run_time){
        DebugOutput::out("计划任务已执行:" + QString::number(m_task_excute_time) + "秒");
        DebugOutput::out("本次计划任务已执行完毕，即将进行下一个计划");
        m_task_id = (m_task_id == Gcontroller.m_tasks.size() - 1) ? 0 : m_task_id + 1;
        return;
    }

    if(Gplayer.DIED){
        // 处理死亡
        handleDied();
    }

    if(Gplayer.mousecheck_exists){
        DebugOutput::out("出现滑鼠验证");
        handleMousemove();
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        goto retryEnter;
    }

    // 符文检测
    if(isExistsRune()){
        qDebug() << "检测到解除符文封印，尝试解除中...";
        solveRune();
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        goto retryEnter;
    }

    // 检查药水
    if(Gplayer.need_hp_potion || Gplayer.need_mp_potion)
    {
        qDebug() << "缺少药水，准备前往商店";
        buyPotion();
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        return;
    }

    // 先检查BUFF
    skillRecast();

    // 获取人物所在地板
    int user_in_lineid = userPlatform();
    if(user_in_lineid == -1){
        // qDebug() << "人物平台未知，可能在空中";
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        goto retryEnter;
    }

    int plat_count = m_platform_sequence.platform_set.size();
    qDebug() << "路线节点数量：" << plat_count;

    bool user_in_platline = false;
    if(m_platform_sequence.platform_set[m_platform_sequence.index] == user_in_lineid)
    {
        user_in_platline = true;
    }

    if(user_in_platline){
        qDebug() << "人物在当前路径节点位置：" << m_platform_sequence.index << "," << user_in_lineid;
        // 先找物品
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

                qDebug() << "======寻找其他平台物品，A星路径======";
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
                        qDebug() << "寻找物品，准备前往平台:" << plat;
                        Gcontroller.m_disable_attack = true;
                        bool move_success = MovePlatform::toPlatform(node, plat);
                        Gcontroller.m_disable_attack = false;
                        if(!move_success){
                            qDebug() << "A星移动平台失败 平台:" << plat;

                            ProcessHelper::programDelay(100);
                            QApplication::processEvents();
                            goto retryEnter;
                        }
                    }
                }
            }

            cur_item = findDropItem();
            if(cur_item.entity != 0 && cur_item.line_id != -1){
                DebugOutput::out("拾取物品中...");
                ProcessHelper::programDelay(100);
                QApplication::processEvents();
                goto retryEnter;
            }
            // return;
        }


        // 判断当前平台怪物是否够多
        updateAllMonsters();
        std::vector<Monster> handle_monsters = getLineMonsters(user_in_lineid);
        int monster_num = handle_monsters.size();
        qDebug() << "当前平台怪物数量:" << monster_num;
        Platform cur_plat = Gplayer.curmapinfo.platform_list[user_in_lineid];
        int plat_length = cur_plat.end.x - cur_plat.start.x;
        qDebug() << "当前平台长度:" << plat_length;
        bool is_allow_clean = false;
        if(monster_num > 0 && plat_length / monster_num <= 500){
            qDebug() << "平台怪物数量够多，先清理怪物~";
            is_allow_clean = true;
        }

        // 如果只有一个节点
        if(plat_count == 1 || is_allow_clean){
            // 清理该平台怪物
            // updateAllMonsters();
            // std::vector<Monster> handle_monsters = getLineMonsters(user_in_lineid);
            if(!handle_monsters.empty()){
                DebugOutput::out("人物所在平台存在怪物，开始清怪~");
                killNearMonster(handle_monsters);
                goto retryEnter;
            }

            DebugOutput::out("循环当前单路线");
            ProcessHelper::programDelay(100);
            QApplication::processEvents();
            goto retryEnter;
        }

        int plat;
        if(m_platform_sequence.index + 1 >= plat_count){
            m_platform_sequence.index = 0;
            DebugOutput::out("路线完成一次循环，即将开始下一轮循环");
            ProcessHelper::programDelay(100);
            QApplication::processEvents();
            goto retryEnter;
        }else{
            plat = m_platform_sequence.platform_set[m_platform_sequence.index+1];
        }
        // int plat = m_platform_sequence.index + 1 >= plat_count ? m_platform_sequence.platform_set[0] : m_platform_sequence.platform_set[m_platform_sequence.index+1];
        qDebug() << "前往路线下个节点：" << plat;

        PathNode node = Gplayer.curmapinfo.path_nodes[user_in_lineid];
        bool move_success = MovePlatform::toPlatform(node, plat);
        if(move_success){
            user_in_lineid = userPlatform();
            if(user_in_lineid == plat){
                m_platform_sequence.index++;
                if(m_platform_sequence.index >= plat_count){
                    m_platform_sequence.index = 0;
                }
                DebugOutput::out("已到达目标路线节点，即将前往下一个节点");
                ProcessHelper::programDelay(100);
                QApplication::processEvents();
                goto retryEnter;
            }
            DebugOutput::out("人物不在目标路线节点~");
        }else{
            bool allow_arrive = false;
            for(const auto &target : node.targets){
                if(target.second.id == plat){
                    allow_arrive = true;
                }
            }
            if(!allow_arrive){
                qDebug() << "无法直接到达该节点，即将A星寻路";
                m_platform_sequence.index++;
                if(m_platform_sequence.index >= plat_count){
                    m_platform_sequence.index = 0;
                    ProcessHelper::programDelay(100);
                    QApplication::processEvents();
                    goto retryEnter;
                }
            }
        }
        DebugOutput::out("到达目标路线节点失败~");
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        goto retryEnter;
    }

    int plat_id = m_platform_sequence.platform_set[m_platform_sequence.index];
    // 如果人物与当前节点相同，则直接进入下个节点，否则使用A星寻路
    user_in_lineid = userPlatform();
    if(user_in_lineid == -1){
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        goto retryEnter;
    }
    if(plat_id == user_in_lineid) goto retryEnter;

    std::vector<int> paths = PathAlgo::findAStarPath(user_in_lineid, plat_id, Gplayer.curmapinfo.path_nodes);
    qDebug() << "======寻找下个节点，A星路径======";
    for(auto const &path : paths){
        qDebug() << path;
    }
    qDebug() << "======A星路径======";

    if(paths.empty()){
        qDebug() << "无法找到前往该平台的路径";
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        goto retryEnter;
    }

    for(auto platform : paths){
        user_in_lineid = userPlatform();
        if(user_in_lineid == -1){
            ProcessHelper::programDelay(100);
            QApplication::processEvents();
            goto retryEnter;
        }

        qDebug() << "当前要前往的路径节点:" << platform;

        // 先找物品
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

                qDebug() << "======寻找其他平台物品，A星路径======";
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
                        qDebug() << "寻找物品，准备前往平台:" << plat;
                        Gcontroller.m_disable_attack = true;
                        bool move_success = MovePlatform::toPlatform(node, plat);
                        Gcontroller.m_disable_attack = false;
                        if(!move_success){
                            qDebug() << "A星移动平台失败 平台:" << plat;

                            ProcessHelper::programDelay(100);
                            QApplication::processEvents();
                            goto retryEnter;
                        }
                    }
                }
            }

            cur_item = findDropItem();
            if(cur_item.entity != 0 && cur_item.line_id != -1){
                DebugOutput::out("拾取物品中...");
                ProcessHelper::programDelay(100);
                QApplication::processEvents();
                goto retryEnter;
            }
        }

        // 判断当前平台怪物是否够多
        updateAllMonsters();
        std::vector<Monster> handle_monsters = getLineMonsters(user_in_lineid);
        int monster_num = handle_monsters.size();
        qDebug() << "当前平台怪物数量:" << monster_num;
        Platform cur_plat = Gplayer.curmapinfo.platform_list[user_in_lineid];
        int plat_length = cur_plat.end.x - cur_plat.start.x;
        qDebug() << "当前平台长度:" << plat_length;
        bool is_allow_clean = false;
        if(monster_num > 0 && plat_length / monster_num <= 500){
            qDebug() << "平台怪物数量够多，先清理怪物~";
            is_allow_clean = true;
        }

        // 如果只有一个节点
        if(is_allow_clean){
            // 清理该平台怪物
            if(!handle_monsters.empty()){
                DebugOutput::out("人物所在平台存在怪物，开始清怪~");
                killNearMonster(handle_monsters);
                goto retryEnter;
            }

            DebugOutput::out("循环当前单路线");
            ProcessHelper::programDelay(100);
            QApplication::processEvents();
            goto retryEnter;
        }

        PathNode node = Gplayer.curmapinfo.path_nodes[user_in_lineid];
        qDebug() << "寻找路径节点，准备前往平台:" << platform;

        bool move_success = MovePlatform::toPlatform(node, platform);
        if(!move_success){
            qDebug() << "A星移动平台失败 平台:" << platform;
            ProcessHelper::programDelay(100);
            QApplication::processEvents();
            goto retryEnter;
        }

        user_in_lineid = userPlatform();
        if(user_in_lineid == plat_id){
            DebugOutput::out("A星已到达目标路线节点");
            goto retryEnter;
        }
    }

    DebugOutput::out("A星到达路线节点失败~，重新执行路线");
    ProcessHelper::programDelay(100);
    QApplication::processEvents();
    goto retryEnter;
}

void KillMonster::excutePlatformRest(int rest_platform)
{
    retryEnter:
    if(m_paused || !m_running){
        return;
    }

    // 更新任务信息
    m_task_info.excute_time = Helper::timestampToHI(m_task_excute_time);
    sendTaskInfo(m_task_info);

    m_task_excute_time = m_task_timer.elapsed() / 1000;
    int run_time = Gcontroller.m_tasks.at(m_task_id).runTime;
    if(m_task_excute_time > run_time){
        DebugOutput::out("计划任务已执行:" + QString::number(m_task_excute_time) + "秒");
        DebugOutput::out("本次计划任务已执行完毕，即将进行下一个计划");
        m_task_id = (m_task_id == Gcontroller.m_tasks.size() - 1) ? 0 : m_task_id + 1;
        return;
    }

    // 获取人物所在地板
    int user_in_lineid = userPlatform();
    if(user_in_lineid == -1){
        // qDebug() << "人物平台未知，可能在空中";
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        goto retryEnter;
    }

    if(user_in_lineid == rest_platform){
        qDebug() << "人物已在休息平台，等待休息结束~";
        ProcessHelper::programDelay(5000);
        QApplication::processEvents();
        goto retryEnter;
    }

    std::vector<int> paths = PathAlgo::findAStarPath(user_in_lineid, rest_platform, Gplayer.curmapinfo.path_nodes);
    qDebug() << "======A星路径======";
    for(auto const &path : paths){
        qDebug() << path;
    }
    qDebug() << "======A星路径======";

    if(paths.empty()){
        qDebug() << "无法找到前往该平台的路径";
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        return;
    }

    for(auto plat : paths){
        user_in_lineid = userPlatform();

        // 先找物品
        Item cur_item = findDropItem();
        if(cur_item.line_id == user_in_lineid && cur_item.entity != 0){
            DebugOutput::out("人物所在平台存在物品，开始拾取~");
            reciveItem(cur_item);
            goto retryEnter;
        }

        PathNode node = Gplayer.curmapinfo.path_nodes[user_in_lineid];
        qDebug() << "准备前往平台:" << plat;

        bool move_success = MovePlatform::toPlatform(node, plat);
        if(!move_success){
            qDebug() << "A星移动平台失败 平台:" << plat;
            ProcessHelper::programDelay(100);
            QApplication::processEvents();
            goto retryEnter;
        }

        user_in_lineid = userPlatform();
        if(user_in_lineid == rest_platform){
            DebugOutput::out("A星已到达目标路线节点");
            goto retryEnter;
        }
    }
}

void KillMonster::excuteQuest()
{
    retryEnter:
    if(m_paused || !m_running){
        return;
    }

    // 更新任务信息
    m_task_info.excute_time = Helper::timestampToHI(m_task_excute_time);
    sendTaskInfo(m_task_info);

    m_task_excute_time = m_task_timer.elapsed() / 1000;
    int run_time = Gcontroller.m_tasks.at(m_task_id).runTime;
    if(m_task_excute_time > run_time){
        DebugOutput::out("计划任务已执行:" + QString::number(m_task_excute_time) + "秒");
        DebugOutput::out("本次计划任务已执行完毕，即将进行下一个计划");
        m_task_id = (m_task_id == Gcontroller.m_tasks.size() - 1) ? 0 : m_task_id + 1;
        return;
    }

    ProcessHelper::programDelay(5000);
    QApplication::processEvents();
    goto retryEnter;
}

void KillMonster::moveToRoomByUUID(const QString &uuid)
{
    retryEnter:
    if(m_paused || !m_running){
        return;
    }

    qDebug() << "前往地图：" << uuid;

    if(Gplayer.curmapinfo.id == uuid)
    {
        qDebug() << "人物已在目标地图，无需过图";
        return;
    }

    // 搜索算法
    std::vector<TargetRoom> portals = PathAlgo::findAstarRoom(Gplayer.curmapinfo.id, uuid);

    if(portals.empty()){
        qDebug() << "暂不支持前往该地图";
        Helper::console("暂不支持前往该地图");
        return;
    }

    TargetRoom target_room = portals.front();
    int platform = target_room.platform;
    cv::Point area = {target_room.area.x_left, target_room.area.x_right};

    int user_in_lineid;
    user_in_lineid = userPlatform();

    if(user_in_lineid == platform){
        qDebug() << "人物已到达传送阵所在平台";

        WalkBase walk_to_platform(0);
        qDebug() << "寻路到传送阵";
        Gcontroller.m_disable_attack = true;
        // 判断自身是否在攀爬物体中
        bool is_climb = Gplayer.curmapinfo.platform_list[platform].type == 1;
        bool success = walk_to_platform.moveToPlatform(area,is_climb); // 走到范围
        Gcontroller.m_disable_attack = false;
        if(success){
            qDebug() << "已到达传送阵位置";
            ProcessHelper::programDelay(100);
            QApplication::processEvents();

            qDebug() << "切换地图中...";
            MSDriverPRO::keyboardClick(MSKeyboard_UpArrow, 1);
            ProcessHelper::programDelay(1500);
            QApplication::processEvents();
            return;
        }
        qDebug() << "移动传送阵位置失败";
        ProcessHelper::programDelay(500);
        goto retryEnter;
    }

    std::vector<int> paths = PathAlgo::findAStarPath(user_in_lineid, platform, Gplayer.curmapinfo.path_nodes);
    qDebug() << "======A星路径======";
    for(auto const &path : paths){
        qDebug() << path;
    }
    qDebug() << "======A星路径======";

    if(paths.empty()){
        qDebug() << "无法找到前往该平台的路径";
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        goto retryEnter;
    }

    for(auto plat : paths){
        user_in_lineid = userPlatform();

        PathNode node = Gplayer.curmapinfo.path_nodes[user_in_lineid];
        qDebug() << "准备前往平台:" << plat;
        Gcontroller.m_disable_attack = true;
        bool move_success = MovePlatform::toPlatform(node, plat);
        Gcontroller.m_disable_attack = false;
        if(!move_success){
            qDebug() << "A星移动平台失败 平台:" << plat;
            ProcessHelper::programDelay(100);
            QApplication::processEvents();
            goto retryEnter;
        }

        user_in_lineid = userPlatform();
        if(user_in_lineid == platform){
            DebugOutput::out("A星已到达目标路线节点");
            goto retryEnter;
        }
    }

    DebugOutput::out("A星到达路线节点失败~，重新执行路线");
    ProcessHelper::programDelay(100);
    QApplication::processEvents();
    goto retryEnter;

}

void KillMonster::buyPotion()
{
    retryEnter:
    if(m_paused || !m_running){
        return;
    }

    if (Gplayer.room_nodes.find(Gplayer.curmapinfo.id) == Gplayer.room_nodes.end()) {
        qDebug() << "当前地图不支持寻路！";
        return;
    }

    // 如果当前地图是怪物房，使用回城卷轴
    RoomNode cur_roomnode = Gplayer.room_nodes[Gplayer.curmapinfo.id];
    qDebug() << "当前地图：" << cur_roomnode.name;
    if(cur_roomnode.type == 0){
        qDebug() << "当前未在城镇，准备使用回城卷轴~";
        int homekey = Gconfig.getValue("bindkey/gohome").toInt();
        MSDriverPRO::keyboardClick(homekey,1);
        ProcessHelper::programDelay(1500);
        QApplication::processEvents();
        return;
    }

    if(cur_roomnode.type == 1)
    {
        qDebug() << "当前未在商店，准备前往商店~";

        std::vector<TargetRoom> portals = PathAlgo::findPotionRoom(Gplayer.curmapinfo.id);

        if(portals.empty()){
            qDebug() << "暂不支持前往该地图";
            Helper::console("暂不支持前往该商店");
            return;
        }

        TargetRoom target_room = portals.front();
        int platform = target_room.platform;
        cv::Point area = {target_room.area.x_left, target_room.area.x_right};

        int user_in_lineid;
        user_in_lineid = userPlatform();

        if(user_in_lineid == platform){
            qDebug() << "人物已到达传送阵所在平台";

            WalkBase walk_to_platform(0);
            qDebug() << "寻路到传送阵";
            Gcontroller.m_disable_attack = true;
            bool success = walk_to_platform.moveToPlatform(area); // 走到范围
            Gcontroller.m_disable_attack = false;
            if(success){
                qDebug() << "已到达传送阵位置";
                ProcessHelper::programDelay(100);
                QApplication::processEvents();

                qDebug() << "切换地图中...";
                MSDriverPRO::keyboardClick(MSKeyboard_UpArrow, 1);
                ProcessHelper::programDelay(1500);
                QApplication::processEvents();
                return;
            }
            qDebug() << "移动传送阵位置失败";
            ProcessHelper::programDelay(500);
            goto retryEnter;
        }

        std::vector<int> paths = PathAlgo::findAStarPath(user_in_lineid, platform, Gplayer.curmapinfo.path_nodes);
        qDebug() << "======A星路径======";
        for(auto const &path : paths){
            qDebug() << path;
        }
        qDebug() << "======A星路径======";

        if(paths.empty()){
            qDebug() << "无法找到前往该平台的路径";
            ProcessHelper::programDelay(100);
            QApplication::processEvents();
            goto retryEnter;
        }

        for(auto plat : paths){
            user_in_lineid = userPlatform();

            PathNode node = Gplayer.curmapinfo.path_nodes[user_in_lineid];
            qDebug() << "准备前往平台:" << plat;
            Gcontroller.m_disable_attack = true;
            bool move_success = MovePlatform::toPlatform(node, plat);
            Gcontroller.m_disable_attack = false;
            if(!move_success){
                qDebug() << "A星移动平台失败 平台:" << plat;
                ProcessHelper::programDelay(100);
                QApplication::processEvents();
                goto retryEnter;
            }

            user_in_lineid = userPlatform();
            if(user_in_lineid == platform){
                DebugOutput::out("A星已到达目标路线节点");
                goto retryEnter;
            }
        }

        DebugOutput::out("A星到达路线节点失败~，重新执行路线");
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        goto retryEnter;
    }

    if(cur_roomnode.type == 2)
    {
        HWND game_hwnd = FindWindow(NULL,TEXT("MapleStory Worlds-Artale (繁體中文版)"));
        if(game_hwnd == NULL){
            DebugOutput::out("游戏窗口不存在");
            return;
        }
        RECT game_rect;
        GetWindowRect(game_hwnd, &game_rect);

        QScreen* m_pScreen  = QGuiApplication::primaryScreen();
        QPixmap cur_capture = m_pScreen->grabWindow((WId)GetDesktopWindow());
        int game_x          = game_rect.left;
        int game_y          = game_rect.top;
        int game_width      = game_rect.right - game_rect.left;
        int game_height     = game_rect.bottom - game_rect.top;
        cur_capture         = cur_capture.copy(game_x,game_y,game_width,game_height);
        Gplayer.game_window.x = game_x;
        Gplayer.game_window.y = game_y;
        Gplayer.game_window.width = game_width;
        Gplayer.game_window.height = game_height;

        // 如果游戏窗口大小不是1296*759,调整至1296*759
        if (game_width != BASE::game_width || game_height != BASE::game_height) {
            // 调整窗口大小和位置（可选保持原位置）
            SetWindowPos(game_hwnd,  // 窗口句柄
                         NULL,                // 不改变Z顺序
                         game_rect.left,      // X位置（保持不变）
                         game_rect.top,       // Y位置（保持不变）
                         BASE::game_width,    // 新宽度
                         BASE::game_height,   // 新高度
                         SWP_NOZORDER | SWP_NOACTIVATE);  // 标志位

            GetWindowRect(game_hwnd, &game_rect);
            m_pScreen  = QGuiApplication::primaryScreen();
            cur_capture = m_pScreen->grabWindow((WId)GetDesktopWindow());
            game_x          = game_rect.left;
            game_y          = game_rect.top;
            game_width      = game_rect.right - game_rect.left;
            game_height     = game_rect.bottom - game_rect.top;
            cur_capture     = cur_capture.copy(game_x,game_y,game_width,game_height);
            Gplayer.game_window.x = game_x;
            Gplayer.game_window.y = game_y;
            Gplayer.game_window.width = game_width;
            Gplayer.game_window.height = game_height;
        }

        qDebug() << "当前商店id:" << Gplayer.curmapinfo.id;
        QString npc_id;

        // 判断当前在哪个商店
        if(Gplayer.curmapinfo.id == "a6dd12a3-f252-4c7f-94c3-16457d084c24"){
            qDebug() << "弓箭手村杂货店购买";
            npc_id = "npc-1011100";
            potionShopBuyItem(npc_id);
            return;
        }

        if(Gplayer.curmapinfo.id == "45681289-4726-447f-9aa6-d690072a3291"){
            qDebug() << "勇士之村杂货店购买";
            npc_id = "npc-1021100";
            potionShopBuyItem(npc_id);
            return;
        }

        if(Gplayer.curmapinfo.id == "c812231b-55cf-449c-879f-8d5674951c01"){
            qDebug() << "维多利亚港杂货店购买";
            npc_id = "npc-1001100";
            potionShopBuyItem(npc_id);
            return;
        }

        qDebug() << "不支持当前商店购买物品";
        ProcessHelper::programDelay(1500);
        QApplication::processEvents();
        return;
    }
}

void KillMonster::potionShopBuyItem(QString npc)
{
    // 找到当前NPC
    cv::Point screen_coor = RoomMap::npcScreenCoor(npc);
    if(screen_coor.x == -1){
        qDebug() << "获取NPC坐标失败";
        ProcessHelper::programDelay(1500);
        QApplication::processEvents();
        return;
    }

    qDebug() << "选择NPC";
    MSKM::humanMove(screen_coor.x, screen_coor.y);
    MSDriverPRO::mouseLeftClick();
    ProcessHelper::programDelay(1000);
    QApplication::processEvents();

    // 判断商店是否打开
    MatchTemp tempmatch(":assets/templat/potion_shop/leave_shop");
    cv::Point leave_coor;
    if(tempmatch.match(leave_coor)){
        qDebug() << "选择药水";
        MSKM::humanMove(560, 400);
        ProcessHelper::programDelay(500);

        // 如果滚动条不在顶端
        // float scrool_position = readFloat(readLong());

        quint64 ui_entity = Gplayer.p_uientity;
        quint64 npc_group = Helper::findChild(ui_entity, "NPCGroup");
        quint64 shop_panel = Helper::findChild(npc_group,"ShopPanel");
        quint64 meso = Helper::findChild(shop_panel,"Meso");
        quint64 shop_scroll = Helper::findChild(shop_panel,"Scroll_전체");
        qDebug() << "商店:" << QString::number(shop_scroll,16).toUpper();

        float scroll_position = readFloat(readLong(shop_scroll + BASE::MODEntity::ScrollLayoutGroupComponent) + BASE::ScrollLayoutGroupComponent::Position + 4);
        if(scroll_position > 0){
            int position = scroll_position * 1000;
            int up_times = position / 22 + 3;

            for(int i = 0; i < up_times; i++){
                MSDriverPRO::mouseWheel(1);
                ProcessHelper::programDelay(300);
            }
        }


        QString coin_text = readString(readLong(readLong(meso
            + BASE::MODEntity::TextComponent)
            +BASE::TextComponent::_text)
            + BASE::String::text,100);

        int coin_num = Helper::coinToInt(coin_text);
        int potion_price = 99999;
        qDebug() << "当前金币:" << coin_num;
        // 商店滚动条Scroll_전체

        int hp_type = Gconfig.getValue("potion_buy/hp_type").toInt();
        int hp_num = Gconfig.getValue("potion_buy/hp_num").toInt();
        hp_num = hp_num - Gplayer.quick_slot_pageup_count;
        if(hp_num > 0){
            QString hp_type_src;
            switch (hp_type) {
            case 0:
                potion_price = 50;
                hp_type_src = ":assets/templat/potion_shop/red_potion";
                break;
            case 1:
                potion_price = 160;
                hp_type_src = ":assets/templat/potion_shop/orange_potion";
                break;
            case 2:
                potion_price = 320;
                hp_type_src = ":assets/templat/potion_shop/white_potion";
                break;
            default:
                potion_price = 50;
                hp_type_src = ":assets/templat/potion_shop/red_potion";
                break;
            }

            if(hp_num * potion_price > coin_num){
                hp_num = coin_num / potion_price;
            }

            if(hp_num * potion_price <= coin_num && hp_num > 0)
            {
                MatchTemp match_redpotion(hp_type_src);
                cv::Point redpotion_coor;
                if(match_redpotion.match(redpotion_coor)){
                    qDebug() << "购买血药";
                    MSKM::humanMove(redpotion_coor.x + 120, redpotion_coor.y + 25);
                    ProcessHelper::programDelay(200);
                    MSDriverPRO::mouseLeftClick();
                    ProcessHelper::programDelay(50);
                    MSDriverPRO::mouseLeftClick();
                    ProcessHelper::programDelay(200);

                    MatchTemp match_ok(":assets/templat/potion_shop/ok");
                    cv::Point ok_coor;
                    if(match_ok.match(ok_coor)){
                        qDebug() << "点击输入框";
                        MSKM::humanMove(500, 404);
                        MSDriverPRO::mouseLeftClick();
                        ProcessHelper::programDelay(500);

                        qDebug() << "输入购买数量";
                        // MSDriverPRO::keyboardClick(MSKeyboard_1,1);
                        MSKM::inputNum(hp_num);
                        ProcessHelper::programDelay(500);

                        qDebug() << "确认购买";
                        MSKM::humanMove(ok_coor.x + 30, ok_coor.y + 10);
                        MSDriverPRO::mouseLeftClick();
                        qDebug() << "购买动作执行完毕，等待1秒";
                        ProcessHelper::programDelay(1000);
                    }
                }
            }else{
                Helper helper_tool;
                helper_tool.warning("金币金额不足");
                while(true){
                    if(!Gplayer.show_warning){
                        break;
                    }
                }
                qDebug() << "金币余额不足~";
                ProcessHelper::programDelay(3000);
            }
        }

        coin_text = readString(readLong(readLong(meso
            + BASE::MODEntity::TextComponent)
            +BASE::TextComponent::_text)
            + BASE::String::text,100);

        coin_num = Helper::coinToInt(coin_text);
        qDebug() << "当前金币:" << coin_num;

        int mp_type = Gconfig.getValue("potion_buy/mp_type").toInt();
        int mp_num = Gconfig.getValue("potion_buy/mp_num").toInt();
        mp_num = mp_num - Gplayer.quick_slot_pagedown_count;
        if(mp_num > 0){
            QString mp_type_src;
            switch (mp_type) {
            case 0:
                potion_price = 200;
                mp_type_src = ":assets/templat/potion_shop/blue_potion";
                break;
            case 1:
                potion_price = 620;
                mp_type_src = ":assets/templat/potion_shop/vitality_potion";
                break;
            default:
                potion_price = 200;
                mp_type_src = ":assets/templat/potion_shop/blue_potion";
                break;
            }

            if(mp_num * potion_price > coin_num){
                mp_num = coin_num / potion_price;
            }

            if(mp_num * potion_price <= coin_num && mp_num > 0)
            {
                MatchTemp match_redpotion(mp_type_src);
                cv::Point redpotion_coor;
                if(match_redpotion.match(redpotion_coor)){
                    qDebug() << "购买蓝药";
                    MSKM::humanMove(redpotion_coor.x + 120, redpotion_coor.y + 25);
                    ProcessHelper::programDelay(200);
                    MSDriverPRO::mouseLeftClick();
                    ProcessHelper::programDelay(50);
                    MSDriverPRO::mouseLeftClick();
                    ProcessHelper::programDelay(200);

                    MatchTemp match_ok(":assets/templat/potion_shop/ok");
                    cv::Point ok_coor;
                    if(match_ok.match(ok_coor)){
                        qDebug() << "点击输入框";
                        MSKM::humanMove(500, 404);
                        MSDriverPRO::mouseLeftClick();
                        ProcessHelper::programDelay(500);

                        qDebug() << "输入购买数量";
                        // MSDriverPRO::keyboardClick(MSKeyboard_1,1);
                        MSKM::inputNum(mp_num);
                        ProcessHelper::programDelay(500);

                        qDebug() << "确认购买";
                        MSKM::humanMove(ok_coor.x + 30, ok_coor.y + 10);
                        MSDriverPRO::mouseLeftClick();
                        qDebug() << "购买动作执行完毕，等待1秒";
                        ProcessHelper::programDelay(1000);
                    }
                }
            }else{
                Helper helper_tool;
                helper_tool.warning("金币金额不足");
                while(true){
                    if(!Gplayer.show_warning){
                        break;
                    }
                }
                qDebug() << "金币余额不足~";
               ProcessHelper::programDelay(3000);
            }
        }

        // 如果有确认按钮
        MatchTemp match_ok(":assets/templat/potion_shop/close_ok");
        cv::Point ok_coor;
        if(match_ok.match(ok_coor)){
            ProcessHelper::programDelay(5000);
            qDebug() << "关闭确认框";
            MSKM::humanMove(ok_coor.x + 40, ok_coor.y + 15);
            MSDriverPRO::mouseLeftClick();
            ProcessHelper::programDelay(3000);
        }

        qDebug() << "关闭商店";
        MSKM::humanMove(leave_coor.x + 40, leave_coor.y + 11);
        MSDriverPRO::mouseLeftClick();
        ProcessHelper::programDelay(1000);
        QApplication::processEvents();
    }else{
        qDebug() << "商店打开失败";
        ProcessHelper::programDelay(500);
        QApplication::processEvents();
        return;
    }

    ProcessHelper::programDelay(500);
    QApplication::processEvents();
    return;
}

void KillMonster::handleDied()
{
    if(!Gplayer.DIED){
        return;
    }

    qDebug() << "角色已死亡即将返回城镇";

    MatchTemp tempmatch(":assets/templat/main_pannel/user_died/died_ok");
    cv::Point died_ok;
    if(tempmatch.match(died_ok)){
        qDebug() << "点击确定";
        MSKM::humanMove(died_ok.x + 27, died_ok.y + 10);
        ProcessHelper::programDelay(500);
        MSDriverPRO::mouseLeftClick();
        ProcessHelper::programDelay(1500);
    }
}

void KillMonster::handleMousemove()
{
    Helper::warningSound();
    ProcessHelper::programDelay(3000);
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

void KillMonster::taskCleanRoom()
{
    qDebug() << "当前执行计划：刷图";

    // 检查药水
    if(Gplayer.need_hp_potion || Gplayer.need_mp_potion)
    {
        qDebug() << "缺少药水，准备前往商店";
        buyPotion();
        ProcessHelper::programDelay(100);
        QApplication::processEvents();
        return;
    }

    m_task_info.type = "刷图";
    m_task_info.option1_label = "地图";

    int map_index = Gcontroller.m_tasks.at(m_task_id).option1_key;
    int path_index = Gcontroller.m_tasks.at(m_task_id).option2_key;

    // 读取地图
    MapRoomPath task_map = Gcontroller.m_clean_room_list.at(map_index);
    m_task_info.option1_text = task_map.name;

    // 当前地图UUID
    if(Gplayer.curmapinfo.id != task_map.uuid){
        // 前往该地图
        Helper::console("前往地图:" + task_map.name);
        moveToRoomByUUID(task_map.uuid);
        ProcessHelper::programDelay(1000);
        QApplication::processEvents();
        return;
    }

    // 读取路线
    m_task_info.option2_label = "路线";
    m_task_info.option2_text = task_map.paths.at(path_index).name;

    // 默认第一条路线都是自动路线
    if(path_index == 0){
        qDebug() << "刷图路线：自动规划路线";
        autoCleanLine();
    }else{
        qDebug() << "刷图路线：" + m_task_info.option2_text;
        m_platform_sequence.platform_set = task_map.paths.at(path_index).platforms;
        m_platform_sequence.index = 0;
        // 执行路线刷图
        excutePlatformSequence();
    }
}

void KillMonster::taskRest()
{
    qDebug() << "当前执行计划：休息";
    m_task_info.type = "休息";
    m_task_info.option1_label = "地图";

    int map_index = Gcontroller.m_tasks.at(m_task_id).option1_key;
    int rest_index = Gcontroller.m_tasks.at(m_task_id).option2_key;

    // 读取地图
    MapRoomPath task_map = Gcontroller.m_rest_point_list.at(map_index);
    m_task_info.option1_text = task_map.name;

    // 当前地图UUID
    if(Gplayer.curmapinfo.id != task_map.uuid){
        // 前往该地图
        Helper::console("前往地图:" + task_map.name);
        moveToRoomByUUID(task_map.uuid);
        ProcessHelper::programDelay(1000);
        QApplication::processEvents();
        return;
    }

    // 读取休息点
    m_task_info.option2_label = "休息点";
    m_task_info.option2_text = task_map.rests.at(rest_index).name;

    qDebug() << "休息点：" + m_task_info.option2_text;
    int rest_platform = task_map.rests.at(rest_index).platform;
    // 执行休息
    excutePlatformRest(rest_platform);
}

void KillMonster::taskQuest()
{
    qDebug() << "当前执行计划：NPC任务";
    m_task_info.type = "任务";
    m_task_info.option1_label = "任务名称";

    int quest_index = Gcontroller.m_tasks.at(m_task_id).option1_key;

    // 读取任务
    NPCQuest task_quest = Gcontroller.m_quest_list.at(quest_index);
    m_task_info.option1_text = task_quest.name;

    // 执行任务
    excuteQuest();
}






















