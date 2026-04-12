#include "moveplatform.h"
#include "pathfinding/walkbase.h"
#include "global/gameoffset.h"
#include "global/globalplayer.h"
#include "driver/mem.h"
#include "utils/helper.h"
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
            qDebug() << "当前位于地板，已找到到达该平台的方式";
            if(target.second.visit_list.empty()){
                qDebug() << "访问方式为空？？？";
                continue;
            }

            // 找到离自己最近的区域
            int distance = 999999;
            cv::Point target_pos = {-1,-1};
            VISIT_TYPE visit_type;
            for(const auto &visit : target.second.visit_list){
                for(const auto &segment : visit.area){
                    int dist = (user_x < segment.first) ? (segment.first - user_x) : (user_x > segment.second) ? (user_x - segment.second) : segment.first;
                    if(dist < distance){
                        distance = dist;
                        target_pos = cv::Point(segment.first, segment.second);
                        visit_type = visit.visit_type;
                        qDebug() << "当前在地板，找到最近距离的区域,距离人物" << distance;
                    }
                }
            }

            if(target.second.type == 0){
                qDebug() << "目标平台为地平线";
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
                        if(jumpRight(target_pos, platform, true)){
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
                        if(down(target_pos, platform, true)){
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
            for(const auto &visit : target.second.visit_list){
                for(const auto &segment : visit.area){
                    int dist = (user_x < segment.first) ? (segment.first - user_x) : (user_x > segment.second) ? (user_x - segment.second) : segment.first;
                    if(dist < distance){
                        distance = dist;
                        target_pos = cv::Point(segment.first, segment.second);
                        visit_type = visit.visit_type;
                        qDebug() << "当前在攀爬体，找到最近距离的区域,距离人物" << distance;
                    }
                }
            }

            if(target.second.type == 0){
                qDebug() << "目标平台为地平线";

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
                    if(down(target_pos, platform)){
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
    bool arrived = walk_to_platform.moveToPlatform(range, self_climb); // 走到范围
    if(!arrived) return false;

    // 这时候读人物的坐标
    if(is_climb){
        // 到达地点之后进行跳跃操作
        MSDriverPRO::keyboardDown(MSKeyboard_LeftArrow);
        MSDriverPRO::keyboardClick(jump_key);
        MSDriverPRO::keyboardDown(MSKeyboard_UpArrow);

        quint64 foothold_p = getUserFootholdP();
        quint64 climb_p = getUserClimbP();
        while(climb_p != 0 && foothold_p == 0){
            ProcessHelper::programDelay(10);
            QApplication::processEvents();
            climb_p = getUserClimbP();
            foothold_p = getUserFootholdP();
        }

        MSDriverPRO::keyboardUp(MSKeyboard_LeftArrow);
        MSDriverPRO::keyboardUp(MSKeyboard_UpArrow);

        if(platform == getUserPlatform(true)){
            qDebug() << "向左跳跃已到达指定攀爬物";
            return true;
        }
    }else{
        // 到达地点之后进行跳跃操作
        MSDriverPRO::keyboardDown(MSKeyboard_LeftArrow);
        MSDriverPRO::keyboardClick(jump_key);
        MSDriverPRO::keyboardUp(MSKeyboard_LeftArrow);

        quint64 foothold_p = getUserFootholdP();
        while(foothold_p == 0){
            ProcessHelper::programDelay(50);
            QApplication::processEvents();
            foothold_p = getUserFootholdP();
        }

        if(platform == getUserPlatform()){
            qDebug() << "向左跳跃已到达指定平台";
            return true;
        }
    }

    return false;
}

bool MovePlatform::jumpRight(cv::Point range, int platform, bool is_climb, bool self_climb)
{
    int jump_key = Gplayer.m_bindkey.jump;

    WalkBase walk_to_platform(0);
    bool arrived = walk_to_platform.moveToPlatform(range, self_climb); // 走到范围
    if(!arrived) return false;

    // 这时候读人物的坐标
    if(is_climb){
        // 到达地点之后进行跳跃操作
        MSDriverPRO::keyboardDown(MSKeyboard_RightArrow);
        MSDriverPRO::keyboardClick(jump_key);
        MSDriverPRO::keyboardDown(MSKeyboard_UpArrow);

        quint64 foothold_p = getUserFootholdP();
        quint64 climb_p = getUserClimbP();
        while(climb_p != 0 && foothold_p == 0){
            ProcessHelper::programDelay(10);
            QApplication::processEvents();
            climb_p = getUserClimbP();
            foothold_p = getUserFootholdP();
        }

        MSDriverPRO::keyboardUp(MSKeyboard_RightArrow);
        MSDriverPRO::keyboardUp(MSKeyboard_UpArrow);

        if(platform == getUserPlatform(true)){
            qDebug() << "向右跳跃已到达指定攀爬物";
            return true;
        }
    }else{
        // 到达地点之后进行跳跃操作
        MSDriverPRO::keyboardDown(MSKeyboard_RightArrow);
        MSDriverPRO::keyboardClick(jump_key);
        MSDriverPRO::keyboardUp(MSKeyboard_RightArrow);


        quint64 foothold_p = getUserFootholdP();
        while(foothold_p == 0){
            ProcessHelper::programDelay(50);
            QApplication::processEvents();
            foothold_p = getUserFootholdP();
        }

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
        // 到达地点之后进行跳跃操作
        MSDriverPRO::keyboardClick(jump_key);
        MSDriverPRO::keyboardDown(MSKeyboard_UpArrow);

        quint64 foothold_p = getUserFootholdP();
        quint64 climb_p = getUserClimbP();
        while(climb_p != 0 && foothold_p == 0){
            ProcessHelper::programDelay(10);
            QApplication::processEvents();
            climb_p = getUserClimbP();
            foothold_p = getUserFootholdP();
        }

        MSDriverPRO::keyboardUp(MSKeyboard_UpArrow);

        if(platform == getUserPlatform(true)){
            qDebug() << "向上跳跃已到达指定攀爬物";
            return true;
        }
    }else{
        // 到达地点之后进行跳跃操作
        MSDriverPRO::keyboardClick(jump_key);

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

    // 到达地点之后进行跳跃操作
    MSDriverPRO::keyboardDown(MSKeyboard_DownArrow);
    MSDriverPRO::keyboardClick(jump_key);
    MSDriverPRO::keyboardUp(MSKeyboard_DownArrow);

    quint64 foothold_p = getUserFootholdP();
    while(foothold_p == 0){
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
    WalkBase walk_to_platform(0);
    bool arrived = walk_to_platform.moveToPlatform(range, self_climb); // 走到范围
    if(!arrived) return false;

    if(self_climb){
        ProcessHelper::programDelay(100);
        if(platform == getUserPlatform()){
            qDebug() << "向上攀爬已到达指定平台";
            return true;
        }

    }else{
        // 到达地点之后进行跳跃操作
        MSDriverPRO::keyboardClick(MSKeyboard_UpArrow);

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

bool MovePlatform::down(cv::Point range, int platform, bool is_climb, bool self_climb)
{
    WalkBase walk_to_platform(0);
    bool arrived = walk_to_platform.moveToPlatform(range, self_climb); // 走到范围
    if(!arrived) return false;

    if(is_climb){
        // 到达地点之后进行跳跃操作
        MSDriverPRO::keyboardClick(MSKeyboard_DownArrow);

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
    }else{
        quint64 foothold_p = getUserFootholdP();
        while(foothold_p == 0){
            ProcessHelper::programDelay(50);
            QApplication::processEvents();
            foothold_p = getUserFootholdP();
        }

        if(platform == getUserPlatform()){
            qDebug() << "向下攀爬已到达指定平台";
            return true;
        }
    }

    if(platform == getUserPlatform()){
        qDebug() << "向下已到达指定平台";
        return true;
    }

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
