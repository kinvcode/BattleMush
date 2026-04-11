#ifndef MOVEPLATFORM_H
#define MOVEPLATFORM_H
#include "global/struct.h"

class MovePlatform
{
public:
    static void toPlatform(const PathNode& plat, int target);

    static void walkLeft(LineSegment area);

    static void walkRight(LineSegment area);

    static void jumpLeft(LineSegment area);

    static void jumpRight(LineSegment area);

    static void jumpUP(LineSegment area);

    static void jumpDown(LineSegment area);

    static void up(LineSegment area);

    static void down(LineSegment area);
};

#endif // MOVEPLATFORM_H
