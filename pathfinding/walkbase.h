#ifndef WALKBASE_H
#define WALKBASE_H

#include "global/struct.h"

class WalkBase
{
public:
    int m_target_x;

    int m_target_y;

    int m_deviation;

    int m_walk_timeout;

    quint64 m_transform;

    WalkBase(int target_x);

    bool run();

    bool runY();

    void clearMonster(cv::Point range);

    bool moveToItem(cv::Point range);

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
    cv::Point m_skill_range; // 监测怪物的

    cv::Point monsterInRange(quint64 head, int count, int &start_index);

    void attackMonster(cv::Point pos);
};

#endif // WALKBASE_H
