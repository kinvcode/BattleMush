#include "moveplatform.h"
#include "pathfinding/walkbase.h"
#include "global/gameoffset.h"
#include "global/globalplayer.h"
#include "driver/mem.h"
#include "utils/helper.h"
#include "driver/msdk/msdkDriver.h"
#include "global/globalconfig.h"
#include <QElapsedTimer>

using namespace Mem;

bool MovePlatform::toPlatform(const PathNode &node, int platform, bool is_town)
{
    if(node.id == platform){
        qDebug() << "当前平台与目标平台相同，退出~";
        return true;
    }

    quint64 transform = readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent);
    int user_x = getUserX(transform);

    if(node.type == 0){
        for(const auto &target : node.targets){
            if(target.first != platform){
                continue;
            }
            // qDebug() << "当前位于地板，已找到到达该平台的方式";
            if(target.second.visit_list.empty()){
                qDebug() << "访问方式为空？？？";
                continue;
            }

            // 找到离自己最近的区域
            int distance = 999999;
            cv::Point target_pos = {-1,-1};
            VISIT_TYPE visit_type;
            for(const auto &visit : target.second.visit_list){
                if(!Gplayer.use_skillteleport){
                    if(visit.visit_type == VISIT_TYPE::SKILL)
                    {
                        continue;
                    }
                }

                for(const auto &segment : visit.area){
                    int dist = (user_x < segment.first) ? (segment.first - user_x) : (user_x > segment.second) ? (user_x - segment.second) : segment.first;
                    if(dist < distance){
                        distance = dist;
                        target_pos = cv::Point(segment.first, segment.second);
                        visit_type = visit.visit_type;
                        // qDebug() << "当前在地板，找到最近距离的区域" << " 访问类型:" << visit_type << " 距离人物:" << distance;
                    }
                }
            }

            // qDebug() << "移动区域:" << target_pos.x << "," << target_pos.y;
            if(target.second.type == 0){
                // qDebug() << "目标平台为地平线";
                switch (visit_type) {
                case VISIT_TYPE::WALK_LEFT:
                    qDebug() << "步行到左边可以到达目标平台";
                    if(walkLeft(target_pos, platform)){
                        return true;
                    }
                    qDebug() << "步行到左边未到目的平台，即将尝试其他方式";
                    break;
                case VISIT_TYPE::WALK_RIGHT:
                    qDebug() << "步行到右边可以到达目标平台";
                    if(walkRight(target_pos, platform)){
                        return true;
                    }
                    qDebug() << "步行到右边未到目的平台，即将尝试其他方式";
                    break;
                case VISIT_TYPE::JUMP_LEFT:
                    qDebug() << "向左跳跃可以到达目标平台";
                    if(jumpLeft(target_pos, platform)){
                        return true;
                    }
                    qDebug() << "向左跳跃未到目的平台，即将尝试其他方式";
                    break;
                case VISIT_TYPE::JUMP_RIGHT:
                    qDebug() << "向右跳跃可以到达目标平台";
                    if(jumpRight(target_pos, platform)){
                        return true;
                    }
                    qDebug() << "向右跳跃未到目的平台，即将尝试其他方式";
                    break;
                case VISIT_TYPE::JUMP_UP:
                    qDebug() << "向上跳跃可以到达目标平台";
                    if(jumpUP(target_pos, platform)){
                        return true;
                    }
                    qDebug() << "向上跳跃未到目的平台，即将尝试其他方式";
                    break;
                case VISIT_TYPE::JUMP_DOWN:
                    qDebug() << "向下跳跃可以到达目标平台";
                    if(jumpDown(target_pos, platform)){
                        return true;
                    }
                    qDebug() << "向下跳跃未到目的平台，即将尝试其他方式";
                    break;
                case VISIT_TYPE::TELEPORT:
                    qDebug() << "通过传送阵可以到达目标平台";
                    if(teleport(target_pos, platform)){
                        return true;
                    }
                    qDebug() << "通过传送阵未到目的平台，即将尝试其他方式";
                    break;
                case VISIT_TYPE::SKILL:
                    qDebug() << "使用技能可以到达目标平台";
                    if(skillMove(target_pos, platform)){
                        return true;
                    }
                    qDebug() << "使用技能未到目的平台，即将尝试其他方式";
                    break;
                default:
                    break;
                }
            }else{
                for(const auto &visit : target.second.visit_list){

                    switch (visit.visit_type) {
                    case VISIT_TYPE::JUMP_LEFT:
                        qDebug() << "向左跳跃可以到达目标攀爬体";
                        if(jumpLeft(target_pos, platform, true)){
                            return true;
                        }
                        qDebug() << "向左跳跃未到攀爬体，即将尝试其他方式";
                        break;
                    case VISIT_TYPE::JUMP_RIGHT:
                        qDebug() << "向右跳跃可以到达目标攀爬体";
                        if(jumpRight(target_pos, platform, true)){
                            return true;
                        }
                        qDebug() << "向右跳跃未到攀爬体，即将尝试其他方式";
                        break;
                    case VISIT_TYPE::JUMP_UP:
                        qDebug() << "向上跳跃可以到达目标攀爬体";
                        if(jumpUP(target_pos, platform, true)){
                            return true;
                        }
                        qDebug() << "向上跳跃未到攀爬体，即将尝试其他方式";
                        break;
                    case VISIT_TYPE::UP:
                        qDebug() << "向上攀爬可以到达目标攀爬体";
                        if(up(target_pos, platform)){
                            return true;
                        }
                        qDebug() << "向上未到攀爬体，即将尝试其他方式";
                        break;
                    case VISIT_TYPE::DOWN:
                        qDebug() << "向下可以到达目标攀爬体";
                        if(down(target_pos, platform, false)){
                            return true;
                        }
                        qDebug() << "向下未到目标攀爬体，即将尝试其他方式";
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }else{
        for(const auto &target : node.targets){
            if(target.first != platform){
                continue;
            }
            qDebug() << "当前位于攀爬体，已找到达该平台的方式";
            if(target.second.visit_list.empty()){
                qDebug() << "访问方式为空？？？";
                continue;
            }

            // 找到离自己最近的区域
            int distance = 999999;
            cv::Point target_pos = {-1,-1};
            VISIT_TYPE visit_type;
            bool is_find = false;
            for(const auto &visit : target.second.visit_list){
                if(is_find) break;
                for(const auto &segment : visit.area){
                    int dist = (user_x < segment.first) ? (segment.first - user_x) : (user_x > segment.second) ? (user_x - segment.second) : segment.first;
                    if(dist < distance){
                        distance = dist;
                        target_pos = cv::Point(segment.first, segment.second);
                        visit_type = visit.visit_type;
                        qDebug() << "当前在攀爬体，找到最近距离的区域,距离人物" << distance;
                        if(target.second.type == 0 && visit_type == VISIT_TYPE::UP){
                            // 优先选择向上攀爬,不要跳
                            is_find = true;
                            break;
                        }
                    }
                }
            }

            if(target.second.type == 0){
                // qDebug() << "目标平台为地平线";

                switch (visit_type) {
                case VISIT_TYPE::JUMP_LEFT:
                    qDebug() << "向左跳跃可以到达目标平台";
                    if(jumpLeft(target_pos, platform, false, true)){
                        return true;
                    }
                    qDebug() << "向左跳跃未到目标平台，即将尝试其他方式";
                    break;
                case VISIT_TYPE::JUMP_RIGHT:
                    qDebug() << "向右跳跃可以到达目标平台";
                    if(jumpRight(target_pos, platform, false, true)){
                        return true;
                    }
                    qDebug() << "向右跳跃未到目的平台，即将尝试其他方式";
                    break;
                case VISIT_TYPE::UP:
                    qDebug() << "向上攀爬可以到达目标平台";
                    if(up(target_pos, platform, true)){
                        return true;
                    }
                    qDebug() << "向上未到攀爬体，即将尝试其他方式";
                    break;
                case VISIT_TYPE::DOWN:
                    qDebug() << "向下可以到达目标平台";
                    if(down(target_pos, platform, true)){
                        return true;
                    }
                    qDebug() << "向下未到目标平台，即将尝试其他方式";
                    break;
                default:
                    break;
                }
            }else{
                switch (visit_type) {
                case VISIT_TYPE::JUMP_LEFT:
                    qDebug() << "向左跳跃可以到达目标攀爬体";
                    if(jumpLeft(target_pos, platform, true, true)){
                        return true;
                    }
                    qDebug() << "向左跳跃未到攀爬体，即将尝试其他方式";
                    break;
                case VISIT_TYPE::JUMP_RIGHT:
                    qDebug() << "向右跳跃可以到达目标攀爬体";
                    if(jumpRight(target_pos, platform, true, true)){
                        return true;
                    }
                    qDebug() << "向右跳跃未到攀爬体，即将尝试其他方式";
                    break;
                default:
                    break;
                }
            }
        }
    }
    return false;
}

bool MovePlatform::walkLeft(cv::Point range, int platform)
{
    WalkBase walk_to_platform(0);
    walk_to_platform.moveToPlatform(range); // 走到范围

    // 这时候读人物的坐标
    quint64 foothold_p = getUserFootholdP();
    while(foothold_p == 0){
        ProcessHelper::programDelay(50);
        QApplication::processEvents();
        foothold_p = getUserFootholdP();
    }

    if(platform == getUserPlatform()){
        qDebug() << "向左走已到达指定平台";
        return true;
    }
    return false;
}

bool MovePlatform::walkRight(cv::Point range, int platform)
{
    WalkBase walk_to_platform(0);
    walk_to_platform.moveToPlatform(range); // 走到范围

    // 这时候读人物的坐标
    quint64 foothold_p = getUserFootholdP();
    while(foothold_p == 0){
        ProcessHelper::programDelay(50);
        QApplication::processEvents();
        foothold_p = getUserFootholdP();
    }

    if(platform == getUserPlatform()){
        qDebug() << "向右走已到达指定平台";
        return true;
    }
    return false;
}

bool MovePlatform::jumpLeft(cv::Point range, int platform, bool is_climb, bool self_climb)
{
    int jump_key = Gplayer.m_bindkey.jump;

    WalkBase walk_to_platform(0);
    if(!is_climb && range.y - range.x > 15){
        range.y = range.x + 15;
    }
    bool arrived = walk_to_platform.moveToPlatform(range, self_climb); // 走到范围
    if(!arrived) {
        qDebug() << "到达区域失败，取消向左跳跃";
        return false;
    }

    qDebug() << "延迟后向左跳跃";
    ProcessHelper::programDelay(250);
    QApplication::processEvents();

    // 这时候读人物的坐标
    if(is_climb){
        qDebug() << "向左跳跃到攀爬物体";
        // 到达地点之后进行跳跃操作
        MSDK::KeyDown(Keyboard_LeftArrow);
        MSDK::keyPress(jump_key,1);
        MSDK::KeyDown(Keyboard_UpArrow);

        quint64 foothold_p = getUserFootholdP();
        quint64 climb_p = getUserClimbP();
        while(climb_p == 0 && foothold_p == 0){
            ProcessHelper::programDelay(10);
            QApplication::processEvents();
            climb_p = getUserClimbP();
            foothold_p = getUserFootholdP();
        }

        MSDK::KeyUp(Keyboard_LeftArrow);
        MSDK::KeyUp(Keyboard_UpArrow);

        if(platform == getUserPlatform(true)){
            qDebug() << "向左跳跃已到达指定攀爬物";
            return true;
        }
        qDebug() << "没到达指定攀爬物";
    }else{
        qDebug() << "向左跳跃到地板";
        // 到达地点之后进行跳跃操作
        MSDK::KeyDown(Keyboard_LeftArrow);
        MSDK::keyPress(jump_key,1);

        quint64 foothold_p = getUserFootholdP();
        while(foothold_p == 0){
            ProcessHelper::programDelay(50);
            QApplication::processEvents();
            foothold_p = getUserFootholdP();
        }
        MSDK::KeyUp(Keyboard_LeftArrow);

        if(platform == getUserPlatform()){
            qDebug() << "向左跳跃已到达指定平台";
            return true;
        }
        qDebug() << "没到达指定平台";
    }

    qDebug() << "向左跳跃失败~";
    return false;
}

bool MovePlatform::jumpRight(cv::Point range, int platform, bool is_climb, bool self_climb)
{
    int jump_key = Gplayer.m_bindkey.jump;

    WalkBase walk_to_platform(0);
    // qDebug() << "移动到范围:" << range.x << "," << range.y;
    if(!is_climb && range.y - range.x > 15){
        range.x = range.y - 15;
    }
    bool arrived = walk_to_platform.moveToPlatform(range, self_climb); // 走到范围
    if(!arrived) return false;

    qDebug() << "延迟后向右跳跃";
    ProcessHelper::programDelay(250);
    QApplication::processEvents();

    // 这时候读人物的坐标
    if(is_climb){
        // 到达地点之后进行跳跃操作
        // qDebug() << "向右跳跃";
        MSDK::KeyDown(Keyboard_RightArrow);
        MSDK::KeyDown(Keyboard_UpArrow);
        MSDK::keyPress(jump_key,1);

        quint64 foothold_p = getUserFootholdP();
        quint64 climb_p = getUserClimbP();
        while(climb_p == 0 && foothold_p == 0){
            ProcessHelper::programDelay(10);
            QApplication::processEvents();
            climb_p = getUserClimbP();
            foothold_p = getUserFootholdP();
        }

        MSDK::KeyUp(Keyboard_RightArrow);
        MSDK::KeyUp(Keyboard_UpArrow);

        if(platform == getUserPlatform(true)){
            qDebug() << "向右跳跃已到达指定攀爬物";
            return true;
        }
    }else{
        // 到达地点之后进行跳跃操作
        MSDK::KeyDown(Keyboard_RightArrow);
        MSDK::keyPress(jump_key,1);

        quint64 foothold_p = getUserFootholdP();
        while(foothold_p == 0){
            ProcessHelper::programDelay(50);
            QApplication::processEvents();
            foothold_p = getUserFootholdP();
        }
        MSDK::KeyUp(Keyboard_RightArrow);

        if(platform == getUserPlatform()){
            qDebug() << "向右跳跃已到达指定平台";
            return true;
        }
    }

    return false;
}

bool MovePlatform::jumpUP(cv::Point range, int platform, bool is_climb)
{
    int jump_key = Gplayer.m_bindkey.jump;

    WalkBase walk_to_platform(0);
    bool arrived = walk_to_platform.moveToPlatform(range); // 走到范围
    if(!arrived) return false;
    // 这时候读人物的坐标
    if(is_climb){
        qDebug() << "延迟后跳跃";
        ProcessHelper::programDelay(250);
        QApplication::processEvents();

        // 到达地点之后进行跳跃操作
        MSDK::KeyDown(Keyboard_UpArrow);
        MSDK::keyPress(jump_key,1);

        quint64 foothold_p = getUserFootholdP();
        quint64 climb_p = getUserClimbP();
        while(climb_p == 0 && foothold_p == 0){
            ProcessHelper::programDelay(10);
            QApplication::processEvents();
            climb_p = getUserClimbP();
            foothold_p = getUserFootholdP();
        }

        MSDK::KeyUp(Keyboard_UpArrow);

        if(platform == getUserPlatform(true)){
            qDebug() << "向上跳跃已到达指定攀爬物";
            return true;
        }
    }else{
        ProcessHelper::programDelay(250);
        QApplication::processEvents();

        // 到达地点之后进行跳跃操作
        MSDK::keyPress(jump_key,1);

        quint64 foothold_p = getUserFootholdP();
        while(foothold_p == 0){
            ProcessHelper::programDelay(50);
            QApplication::processEvents();
            foothold_p = getUserFootholdP();
        }

        if(platform == getUserPlatform()){
            qDebug() << "向上跳跃已到达指定平台";
            return true;
        }
    }
    return false;
}

bool MovePlatform::jumpDown(cv::Point range, int platform)
{
    int jump_key = Gplayer.m_bindkey.jump;

    WalkBase walk_to_platform(0);
    bool arrived = walk_to_platform.moveToPlatform(range); // 走到范围
    if(!arrived) return false;

    ProcessHelper::programDelay(250);
    QApplication::processEvents();

    // 到达地点之后进行跳跃操作
    MSDK::KeyDown(Keyboard_DownArrow);
    MSDK::keyPress(jump_key,1);
    MSDK::KeyUp(Keyboard_DownArrow);

    quint64 foothold_p = getUserFootholdP();
    while(foothold_p == 0){
        if(getUserClimbP() != 0){
            break;
        }
        ProcessHelper::programDelay(50);
        QApplication::processEvents();
        foothold_p = getUserFootholdP();
    }

    if(platform == getUserPlatform()){
        qDebug() << "向下跳跃已到达指定平台";
        return true;
    }


    return false;
}

bool MovePlatform::up(cv::Point range, int platform, bool self_climb)
{
    if(self_climb){
        MSDK::KeyDown(Keyboard_UpArrow);

        quint64 foothold_p = getUserFootholdP();
        quint64 climb_p = getUserClimbP();
        while(foothold_p == 0 && climb_p != 0){
            ProcessHelper::programDelay(50);
            QApplication::processEvents();
            climb_p = getUserClimbP();
            foothold_p = getUserFootholdP();
        }

        MSDK::KeyUp(Keyboard_UpArrow);
        if(platform == getUserPlatform()){
            qDebug() << "向上攀爬已到达指定平台";
            return true;
        }

    }else{
        WalkBase walk_to_platform(0);
        bool arrived = walk_to_platform.moveToPlatform(range, self_climb); // 走到范围
        if(!arrived) return false;

        qDebug() << "延迟后向上攀爬";
        ProcessHelper::programDelay(250);
        QApplication::processEvents();

        // 到达地点之后按上键
        MSDK::keyPress(Keyboard_UpArrow,1);

        ProcessHelper::programDelay(100);
        quint64 climb_p = getUserClimbP();
        if(climb_p == 0){
            qDebug() << "向上未能攀爬到攀爬体";
            return false;
        }

        if(platform == getUserPlatform(true)){
            qDebug() << "向上已到达指定攀爬物";
            return true;
        }
    }

    if(platform == getUserPlatform()){
        qDebug() << "向上已到达指定平台";
        return true;
    }


    return false;
}

bool MovePlatform::down(cv::Point range, int platform, bool self_climb)
{
    if(self_climb){
        MSDK::KeyDown(Keyboard_DownArrow);

        quint64 foothold_p = getUserFootholdP();
        quint64 climb_p = getUserClimbP();
        while(foothold_p == 0 && climb_p != 0){
            ProcessHelper::programDelay(50);
            QApplication::processEvents();
            climb_p = getUserClimbP();
            foothold_p = getUserFootholdP();
        }

        MSDK::KeyUp(Keyboard_DownArrow);
        if(platform == getUserPlatform()){
            qDebug() << "向下攀爬已到达指定平台";
            return true;
        }
    }else{
        WalkBase walk_to_platform(0);
        bool arrived = walk_to_platform.moveToPlatform(range, self_climb); // 走到范围
        if(!arrived) return false;

        // 到达地点之后按下方向键
        MSDK::keyPress(Keyboard_DownArrow,1);

        ProcessHelper::programDelay(100);
        quint64 climb_p = getUserClimbP();
        if(climb_p == 0){
            qDebug() << "向下未能攀爬到攀爬体";
            return false;
        }
        if(platform == getUserPlatform(true)){
            qDebug() << "向下已到达指定攀爬物";
            return true;
        }
    }

    if(platform == getUserPlatform()){
        qDebug() << "向下已到达指定平台";
        return true;
    }

    return false;
}

bool MovePlatform::teleport(cv::Point range, int platform)
{
    WalkBase walk_to_platform(0);
    bool arrived = walk_to_platform.moveToPlatform(range); // 走到范围
    if(!arrived) return false;

    // 到达地点之后按向上方向键
    MSDK::keyPress(Keyboard_UpArrow,1);

    ProcessHelper::programDelay(100);

    quint64 foothold_p = getUserFootholdP();
    while(foothold_p == 0){
        if(getUserClimbP() != 0){
            break;
        }
        ProcessHelper::programDelay(50);
        QApplication::processEvents();
        foothold_p = getUserFootholdP();
    }

    if(platform == getUserPlatform()){
        qDebug() << "通过传送已到达指定平台";
        return true;
    }

    return false;
}

bool MovePlatform::skillMove(cv::Point range, int platform)
{
    return false;
}

quint64 MovePlatform::getUserFootholdP()
{
    return readLong(readLong(Gplayer.p_localplayer
        + BASE::MODEntity::RigidbodyComponent)
        + BASE::RigidbodyComponent::currentFoothold);
}

quint64 MovePlatform::getUserClimbP()
{
    return readLong(readLong(Gplayer.p_localplayer
        + BASE::MODEntity::RigidbodyComponent)
        + BASE::RigidbodyComponent::attachEntity);
}



int MovePlatform::getUserPlatform(bool is_climb)
{
    quint64 transform = readLong(Gplayer.p_localplayer + BASE::MODEntity::TransformComponent);

    if(is_climb){
        quint64 user_attach_p = getUserClimbP();
        if(user_attach_p == 0){
            return -1;
        }
        cv::Point pos = getUserPos(transform);
        int platform = RoomMap::getClimbPlatform(pos, Gplayer.curmapinfo.platform_list, Gplayer.curmapinfo.map_mat);
        return platform;

    }else{
        quint64 user_foothold_p = getUserFootholdP();
        if(user_foothold_p == 0){
            return -1;
        }

        int user_foothold_id = readInt(user_foothold_p + BASE::Foothold::ID);
        int user_in_lineid = RoomMap::getFootPlatform(user_foothold_id, Gplayer.curmapinfo.platform_list);
        return user_in_lineid;
    }


    // quint64 user_attach_p = readLong(readLong(Gplayer.p_localplayer + BASE::MODEntity::RigidbodyComponent) + BASE::RigidbodyComponent::attachEntity);
    // int user_foothold_id = 0;
    // int user_climb_id = 0;
    // int user_in_lineid = -1;

    // if(user_foothold_p){
    //     user_foothold_id = readInt(user_foothold_p + BASE::Foothold::ID);
    //     user_in_lineid = RoomMap::getFootPlatform(user_foothold_id, Gplayer.curmapinfo.platform_list);
    // }
}

int MovePlatform::getUserX(quint64 transform)
{
    float user_x = readFloat(transform + BASE::TransformComponent::PositionX);

    return RoomMap::toMapPosX(user_x,Gplayer.curmapinfo.map_bounding.x_offset);
}

int MovePlatform::getUserY(quint64 transform)
{
    float user_y = readFloat(transform + BASE::TransformComponent::PositionY);

    return RoomMap::toMapPosY(user_y,Gplayer.curmapinfo.map_bounding.y_offset,Gplayer.curmapinfo.map_bounding.height);
}

cv::Point MovePlatform::getUserPos(quint64 transform)
{
    cv::Point pos = {-1,-1};
    float user_x = readFloat(transform + BASE::TransformComponent::PositionX);
    float user_y = readFloat(transform + BASE::TransformComponent::PositionY);
    pos.x = RoomMap::toMapPosX(user_x,Gplayer.curmapinfo.map_bounding.x_offset);
    pos.y = RoomMap::toMapPosY(user_y,Gplayer.curmapinfo.map_bounding.y_offset, Gplayer.curmapinfo.map_bounding.height);
    return pos;
}
