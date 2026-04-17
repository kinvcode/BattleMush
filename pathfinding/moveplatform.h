#ifndef MOVEPLATFORM_H
#define MOVEPLATFORM_H
#include "global/struct.h"

class MovePlatform
{
public:
    static bool toPlatform(const PathNode& plat, int target, bool is_town = true);

    static bool walkLeft(cv::Point range, int platform);

    static bool walkRight(cv::Point range, int platform);

    static bool jumpLeft(cv::Point range, int platform, bool is_climb = false, bool self_climb = false);

    static bool jumpRight(cv::Point range, int platform, bool is_climb = false, bool self_climb = false);

    static bool jumpUP(cv::Point range, int platform, bool is_climb = false);

    static bool jumpDown(cv::Point range, int platform);

    static bool up(cv::Point range, int platform,bool self_climb = false);

    static bool down(cv::Point range, int platform, bool self_climb = false);

private :
    static quint64 getUserFootholdP();

    static quint64 getUserClimbP();

    static int getUserPlatform(bool is_climb = false);

    static int getUserX(quint64 transform);

    static int getUserY(quint64 transform);

    static cv::Point getUserPos(quint64 transform);
};

#endif // MOVEPLATFORM_H
