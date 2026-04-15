#ifndef WALKBASE_H
#define WALKBASE_H

#include "global/struct.h"

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

    void clearMonster(Monster monster);

    bool moveToItem();

    bool moveToPlatform(cv::Point range, bool self_climb = false);

private:
    int getUserX();

    int getUserY();

    cv::Point getUserPos();

    void clearXkey();

    void clearYkey();

    bool isAttaking();

    // 找怪的函数
private:
    // quint64 m_entity_head = 0;
    // cv::Point m_skill_range = {150,40}; // 这里实际会被上层的清怪线程参数覆盖

    cv::Point monsterInRange(bool is_left, int &start_index);

    void attackMonster(cv::Point user_pos, cv::Point monster_pos);
};

#endif // WALKBASE_H
