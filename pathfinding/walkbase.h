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

private:
    int getUserX();

    int getUserY();

    void clearXkey();

    void clearYkey();
};

#endif // WALKBASE_H
