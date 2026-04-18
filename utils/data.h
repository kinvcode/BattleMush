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

    // 打印物品
    static void roomMapItems();

    // 打印碰撞列表
    static void roomMapCollision();
};

};


#endif // DATA_H
