#include "killmonster.h"
#include <QDebug>
#include "utils/helper.h"
#include "process/process.h"
#include "global/globalplayer.h"
#include "global/gameoffset.h"
#include "driver/mem.h"
#include "utils/gamestruct.h"

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
    m_paused = false;
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
    m_paused = false;
}

void KillMonster::process()
{
    int start_index = 0;
    while (m_running) {
        // 处理暂停状态
        while (m_paused && m_running) {
            start_index = 0;
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

        // 遍历怪物列表
        // 注意，当前地图的childList不断在变化，需要实时读取
        quint64 map_entity_count = readInt(readLong(Gplayer.p_curmapentity + BASE::MODEntity::children) + BASE::List::size);
        quint64 entity_head = readLong(readLong(Gplayer.p_curmapentity + BASE::MODEntity::children) + BASE::List::InternalArray) + BASE::InternalArray::Head;

        std::vector<Monster> monster_list;
        for(int i = start_index; i < map_entity_count; i++){
            // 节省遍历时间

            quint64 cur_entity_address = entity_head + i * 8;
            quint64 entity = readLong(cur_entity_address);

            // 找到End标记位
            QString name = GameStruct::readEntityName(entity);
            if(name == "EndObj"){
                start_index = i + 1;
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
            quint64 monster_entity = entity;
            int foothold_id = readInt(readLong(readLong(entity + BASE::MODEntity::RigidbodyComponent)
                                                + BASE::RigidbodyComponent::currentFoothold)
                                                + BASE::Foothold::ID);
            int line_id = RoomMap::getPlatformByFoothold(foothold_id, Gplayer.curmapinfo.line_list);
            if(line_id == -1){
                continue;
            }
            Monster cur_monster = {monster_entity, foothold_id, line_id, {-1,-1}};
            monster_list.push_back(cur_monster);
        }

        // 获取人物所在地板
        int user_foothold_id = readInt(readLong(readLong(Gplayer.p_localplayer +
                                                        BASE::MODEntity::RigidbodyComponent) +
                                                        BASE::RigidbodyComponent::currentFoothold) +
                                                        BASE::Foothold::ID);
        int user_in_lineid = RoomMap::getPlatformByFoothold(user_foothold_id, Gplayer.curmapinfo.line_list);

        // 待处理的怪物列表，平台ID，怪物指针，怪物坐标
        std::vector<Monster> handle_monsters;
        for(auto item : monster_list){
            // 读取怪物坐标
            if(item.line_id == user_in_lineid){
                quint64 pos_p = readLong(readLong(item.entity) + BASE::MODEntity::RigidbodyComponent);
                float float_x = readFloat(pos_p + BASE::RigidbodyComponent::airPos);
                float float_y = readFloat(pos_p + BASE::RigidbodyComponent::airPos + 4);
                int monster_x = RoomMap::toMapPosX(float_x, Gplayer.curmapinfo.map_bounding.x_offset);
                int monster_y = RoomMap::toMapPosY(float_y, Gplayer.curmapinfo.map_bounding.y_offset,Gplayer.curmapinfo.map_bounding.height);
                // qDebug() << "当前地板怪物：" << item.line_id << "->" << QString::number(item.entity,16).toUpper() << ":" << monster_x << "," << monster_y;
                handle_monsters.push_back({item.entity,item.foothold_id,item.line_id,{monster_x,monster_y}});
            }
        }

        if(handle_monsters.empty()){
            DebugOutput::out("当前平台不存在怪物");
            ProcessHelper::programDelay(1000);
            QApplication::processEvents();
            continue;
        }

        // 找到离自己最近的怪物
        // 获取人物坐标
        float user_x = readFloat(readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent) + BASE::TransformComponent::PositionX);
        Monster target_monster = Math::getNearMonster(handle_monsters, user_x);
        qDebug() << "目标怪物：" << target_monster.line_id << "->" << QString::number(target_monster.entity,16).toUpper() << ":" << target_monster.pos.x << "," << target_monster.pos.y;

        qDebug() << "开始寻路";

        DebugOutput::out("击杀怪物暂时没有任务执行，等待中...");
        ProcessHelper::programDelay(3000);
        QApplication::processEvents();
    }

    qDebug() << "KillMonster" << m_id << "finished";
}
