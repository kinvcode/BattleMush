#ifndef WALKBASE_H
#define WALKBASE_H

#include "global/struct.h"

class WalkBase
{
public:
    int m_target_x;

    int m_deviation;

    int m_walk_timeout;

    quint64 m_transform;

    WalkBase(int target_x);

    bool run();

private:
    int getUserX();

    void clearXkey();
};

#endif // WALKBASE_H
