#ifndef DATA_H
#define DATA_H

#include "global/struct.h"

namespace Data{

class Game
{
public:
    // 获取gamecontext
    static quint64 gamecontext();

    // 获取map entity
    static quint64 mapEntity();

    // 获取UI entity
    static quint64 uiEntity();

    // 打印物品
    static void roomMapItems();

    // 打印碰撞列表
    static void roomMapCollision();

    // 打印符文解除指令
    static void runeUISprites();

    // 打印地图传送阵列表
    static void roomMapPortal();

    // 打印范围内怪物
    static void rangeMonster();

    static std::vector<quint64> npcEntityList();

    // NPC屏幕坐标
    static void NPCScreenCoor();

    // 打印怪物数量
    static void platformMonsters();
};

};


#endif // DATA_H
