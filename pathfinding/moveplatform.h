#ifndef MOVEPLATFORM_H
#define MOVEPLATFORM_H
#include "global/struct.h"

class MovePlatform
{
public:
    static bool toPlatform(const PathNode& plat, int target);

    static bool walkLeft(LineSegment area, int platform);

    static bool walkRight(LineSegment area, int platform);

    static bool jumpLeft(LineSegment area, int platform, bool is_climb = false, bool self_climb = false);

    static bool jumpRight(LineSegment area, int platform, bool is_climb = false, bool self_climb = false);

    static bool jumpUP(LineSegment area, int platform, bool is_climb = false);

    static bool jumpDown(LineSegment area, int platform);

    static bool up(LineSegment area, int platform,bool self_climb = false);

    static bool down(LineSegment area, int platform, bool is_climb = false);

private :
    static quint64 getUserFootholdP();

    static quint64 getUserClimbP();

    static int getUserPlatform(bool is_climb = false);

    static int getUserX(quint64 transform);

    static int getUserY(quint64 transform);

    static cv::Point getUserPos(quint64 transform);
};

#endif // MOVEPLATFORM_H
