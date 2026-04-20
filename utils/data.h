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
};

};


#endif // DATA_H
