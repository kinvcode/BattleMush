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
    m_bindkey_general_atk = Gconfig.getValue("bindkey/general_atk").toInt();
    m_bindkey_skill_atk1  = Gconfig.getValue("bindkey/skill_atk1").toInt();
    m_bindkey_jump        = Gconfig.getValue("bindkey/jump").toInt();
    m_bindkey_gohome      = Gconfig.getValue("bindkey/gohome").toInt();
    m_paused = false;
}

void KillMonster::process()
{
    m_bindkey_general_atk = Gconfig.getValue("bindkey/general_atk").toInt();
    m_bindkey_skill_atk1  = Gconfig.getValue("bindkey/skill_atk1").toInt();
    m_bindkey_jump        = Gconfig.getValue("bindkey/jump").toInt();
    m_bindkey_gohome      = Gconfig.getValue("bindkey/gohome").toInt();

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
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
            continue;
        }

        // 更新全图怪物
        updateAllMonsters();

        // 获取人物所在地板
        quint64 user_foothold_p = readLong(readLong(Gplayer.p_localplayer + BASE::MODEntity::RigidbodyComponent) + BASE::RigidbodyComponent::currentFoothold);
        quint64 user_attach_p = readLong(readLong(Gplayer.p_localplayer + BASE::MODEntity::RigidbodyComponent) + BASE::RigidbodyComponent::attachEntity);
        int user_foothold_id = 0;
        int user_climb_id = 0;
        int user_in_lineid = -1;
        if(user_foothold_p){
            user_foothold_id = readInt(user_foothold_p + BASE::Foothold::ID);
            // user_in_lineid = RoomMap::getPlatformByFoothold(user_foothold_id, Gplayer.curmapinfo.line_list);
        }

        // int user_x = RoomMap::toMapPosX(readFloat(readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent) + BASE::TransformComponent::PositionX),
        //                                 Gplayer.curmapinfo.map_bounding.x_offset);
        // qDebug() << "人物坐标：" << user_x;

        if(user_in_lineid == -1){
            DebugOutput::out("暂不支持当前位置寻怪，人物可能在攀爬");
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
            continue;
        }

        // 待处理的怪物列表
        std::vector<Monster> handle_monsters = getLineMonsters(user_in_lineid);

        if(handle_monsters.empty()){
            DebugOutput::out("当前平台不存在怪物");
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
            continue;
        }

        // 击杀怪物
        killNearMonster(handle_monsters);

        // 搜索一遍物品列表
        findDropItem(m_map_start);

        DebugOutput::out("击杀怪物暂时没有任务执行，等待中...");
        ProcessHelper::programDelay(500);
        QApplication::processEvents();
    }

    qDebug() << "KillMonster" << m_id << "finished";
}

void KillMonster::findDropItem(int start_index)
{
    quint64 map_entity_count = readInt(readLong(Gplayer.p_curmapentity + BASE::MODEntity::children) + BASE::List::size);
    quint64 entity_head = readLong(readLong(Gplayer.p_curmapentity + BASE::MODEntity::children) + BASE::List::InternalArray) + BASE::InternalArray::Head;

    for(int i = start_index; i < map_entity_count; i++){
        // 节省遍历时间

        quint64 cur_entity_address = entity_head + i * 8;
        quint64 entity = readLong(cur_entity_address);

        QString name = GameStruct::readEntityName(entity);

        if(name != "PoolObject"){
            continue;
        }

        float item_x = readFloat(readLong(entity + BASE::MODEntity::TransformComponent)+BASE::TransformComponent::PositionX);
        float item_y = readFloat(readLong(entity + BASE::MODEntity::TransformComponent)+BASE::TransformComponent::PositionY);

        int x = RoomMap::toMapPosX(item_x, Gplayer.curmapinfo.map_bounding.x_offset);
        int y = RoomMap::toMapPosY(item_y, Gplayer.curmapinfo.map_bounding.y_offset,Gplayer.curmapinfo.map_bounding.height);

        // qDebug() << "找到物品坐标：" << x << "," << y;

        // 计算物品所在平台的位置
        cv::Point item_pos = RoomMap::getItemLinePos(Gplayer.curmapinfo.map_mat,{x, y});
        if(item_pos.x == -1){
            continue; // 物品坐标错误
        }

        //
    }
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
        // int line_id = RoomMap::getPlatformByFoothold(foothold_id, Gplayer.curmapinfo.line_list);
        int line_id = -1;
        if(line_id == -1){
            continue;
        }

        quint64 pos_p = readLong(entity + BASE::MODEntity::RigidbodyComponent);
        float float_x = readFloat(pos_p + BASE::RigidbodyComponent::airPos);
        float float_y = readFloat(pos_p + BASE::RigidbodyComponent::airPos + 4);
        int monster_x = RoomMap::toMapPosX(float_x, Gplayer.curmapinfo.map_bounding.x_offset);
        int monster_y = RoomMap::toMapPosY(float_y, Gplayer.curmapinfo.map_bounding.y_offset,Gplayer.curmapinfo.map_bounding.height);

        Monster cur_monster = {entity, foothold_id, line_id, {monster_x, monster_y}};
        m_allmonster.push_back(cur_monster);
    }
}

std::vector<Monster> KillMonster::getLineMonsters(int user_line)
{
    std::vector<Monster> monsters;

    for(auto item : m_allmonster){
        // 读取怪物坐标
        if(item.line_id == user_line){
            monsters.push_back(item);
        }
    }

    return monsters;
}

void KillMonster::killNearMonster(std::vector<Monster> handle_monsters)
{
    // 找到离自己最近的怪物
    // 获取人物坐标
    int user_x = RoomMap::toMapPosX(readFloat(readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent) + BASE::TransformComponent::PositionX),
                                    Gplayer.curmapinfo.map_bounding.x_offset);
    qDebug() << "人物坐标：" << user_x;
    Monster target_monster = Math::getNearMonster(handle_monsters, user_x);
    qDebug() << "目标怪物：" << target_monster.line_id << "->" << QString::number(target_monster.entity,16).toUpper() << ":" << target_monster.pos.x << "," << target_monster.pos.y;

    qDebug() << "开始寻路";
    WalkBase walk_to_monster(target_monster.pos.x);
    if(walk_to_monster.run()){
        qDebug() << "已到达怪物位置";
        bool allow_attack = readByte(readLong(Gplayer.p_localplayer + BASE::MODEntity::StateComponent) + BASE::StateComponent::attacking);
        if(!allow_attack){
            qDebug() << "处于攻击动作中,不允许攻击";
            return;
        }

        // 再次判断怪物是否存在
        bool exists = readByte(readLong(target_monster.entity + BASE::MODEntity::RigidbodyComponent) + BASE::RigidbodyComponent::exists);
        if(!exists){
            qDebug() << "怪物已消失~,取消攻击";
            return;
        }

        QString status = readString(readLong(readLong(target_monster.entity + BASE::MODEntity::StateComponent) + BASE::StateComponent::_CurrentStateName) + BASE::String::text, 10);
        // qDebug() << "怪物状态：" << status;
        if(status == "DEAD"){
            qDebug() << "怪物已死亡~,取消攻击";
            return;
        }

        MSDriverPRO::keyboardClick(m_bindkey_general_atk,1);
    }else{
        qDebug() << "寻怪失败~";
    }
}






















