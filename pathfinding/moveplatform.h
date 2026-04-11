#ifndef MOVEPLATFORM_H
#define MOVEPLATFORM_H
#include "global/struct.h"

class MovePlatform
{
public:
    static bool toPlatform(const PathNode& plat, int target);

    static bool walkLeft(LineSegment area, int platform);

    static bool walkRight(LineSegment area, int platform);

    static bool jumpLeft(LineSegment area, int platform, bool is_climb = false);

    static bool jumpRight(LineSegment area, int platform, bool is_climb = false);

    static bool jumpUP(LineSegment area, int platform, bool is_climb = false);

    static bool jumpDown(LineSegment area, int platform);

    static bool up(LineSegment area, int platform);

    static bool down(LineSegment area);

private :
    static quint64 getUserFootholdP();

    static quint64 getUserClimbP();

    static int getUserLine();

    static int getUserX(quint64 transform);
};

#endif // MOVEPLATFORM_H
