#include "walkbase.h"
#include "global/gameoffset.h"
#include "global/globalplayer.h"
#include "driver/mem.h"
#include "driver/mskeycode.h"
#include "utils/helper.h"
#include <QElapsedTimer>

using namespace Mem;

WalkBase::WalkBase(int target_x)
{
    m_transform = 0;
    m_deviation = 50;
    m_walk_timeout = 3000;
    m_target_x = target_x;
}

bool WalkBase::run()
{
    m_transform = readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent);

    int user_x = getUserX();

    if(user_x == m_target_x)
    {
        return true;
    }

    // 清空一下按键
    clearXkey();

    // 跑动按键判断
    int direction_x;
    if (m_target_x > user_x) {
        // 向右
        direction_x = MSKeyboard_RightArrow;
    } else {
        // 向左
        direction_x = MSKeyboard_LeftArrow;
    }

    // 是否到达
    bool arrived = false;

    QElapsedTimer begin_timer;
    begin_timer.restart();

    QElapsedTimer run_status_time;
    run_status_time.start();

    // 开始移动
    MSDriverPRO::keyboardDown(direction_x);

    while(true){

        if (begin_timer.elapsed() > m_walk_timeout) {
            DebugOutput::out("寻路超时~");
            clearXkey();
            return false;
        }

        // 到达
        if (arrived) {
            clearXkey();
            return true;
        }

        user_x = getUserX();

        if (abs(m_target_x - user_x) <= m_deviation) {
            arrived = true;
        }
    }

    clearXkey();
    return false;
}

int WalkBase::getUserX()
{
    if(!m_transform){
        return -1;
    }

    float user_x = readFloat(m_transform + BASE::TransformComponent::PositionX);

    return RoomMap::toMapPosX(user_x,Gplayer.curmapinfo.map_bounding.x_offset);
}

void WalkBase::clearXkey()
{
    MSDriverPRO::keyboardUp(MSKeyboard_RightArrow);
    MSDriverPRO::keyboardUp(MSKeyboard_LeftArrow);
}
