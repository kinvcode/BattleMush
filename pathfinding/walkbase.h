#ifndef WALKBASE_H
#define WALKBASE_H

#include "global/struct.h"

// 走进范围的同一时刻执行。None 只表示走到范围。
enum class AreaAction {
    None,
    Jump,
    JumpClimb,
    JumpLeft,
    JumpLeftClimb,
    JumpRight,
    JumpRightClimb,
    JumpDown,
    Up,
    Down,
    Teleport
};

class WalkBase
{
public:
    WalkBase(int target_x);

    int m_target_x;

    int m_target_y;

    int m_deviation;

    int m_walk_timeout;

    quint64 m_transform;

    bool run();

    bool runY();

    void clearMonster(const Monster &monster);

    bool moveToItem();

    // action 不为 None 时，坐标进入范围后立刻按键，中间不再等待。
    bool moveToArea(cv::Point range, bool self_climb = false, AreaAction action = AreaAction::None);

    bool moveToPlatform(int platform,cv::Point range, bool self_climb = false);

    bool moveToPortal(cv::Point range);

    // 按30帧把人物、怪物、物品画到实时地图
    static void paintUser();

private:
    quint64 getUserFootholdP();

    quint64 getUserClimbP();

    int getUserPlatform(bool is_climb = false);

    int getUserX();

    int getUserY();

    cv::Point getUserPos();

    void clearXkey();

    void clearYkey();

    void performAreaAction(AreaAction action);

    bool isAttaking();

    bool isFaceLeft();

    bool monsterExists(const Monster &monster);

    // 找怪的函数
private:
    // quint64 m_entity_head = 0;
    // cv::Point m_skill_range = {150,40}; // 这里实际会被上层的清怪线程参数覆盖

    Monster monsterInRange(bool is_left, int &start_index);

    bool itemExists(int start_index);

    void attackMonster(cv::Point user_pos, const Monster &monster);
};

#endif // WALKBASE_H
