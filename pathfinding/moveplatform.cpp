#include "moveplatform.h"
#include "pathfinding/walkbase.h"



void MovePlatform::toPlatform(const PathNode &node, int platform)
{
    if(node.id == platform){
        qDebug() << "当前平台与目标平台相同，退出~";
        return;
    }

    if(node.type == 0){
        for(const auto &target : node.targets){
            if(target.first != platform){
                continue;
            }
            qDebug() << "已找到到达该平台的方式";
            if(target.second.visit_list.empty()){
                qDebug() << "访问方式为空？？？";
                continue;
            }
            if(target.second.type == 0){
                qDebug() << "目标平台为地平线";
                for(auto visit : target.second.visit_list){
                    switch (visit.visit_type) {
                    case VISIT_TYPE::WALK_LEFT:
                        qDebug() << "步行到左边可以到达目标平台";
                        walkLeft(visit.area);
                        qDebug() << "步行左边完毕~";
                        break;
                    case VISIT_TYPE::WALK_RIGHT:

                        break;
                    case VISIT_TYPE::JUMP_LEFT:

                        break;
                    case VISIT_TYPE::JUMP_RIGHT:

                        break;
                    case VISIT_TYPE::JUMP_UP:

                        break;
                    case VISIT_TYPE::JUMP_DOWN:

                        break;
                    default:
                        break;
                    }
                }
            }else{
                for(auto visit : target.second.visit_list){
                    switch (visit.visit_type) {
                    case VISIT_TYPE::JUMP_LEFT:

                        break;
                    case VISIT_TYPE::JUMP_RIGHT:

                        break;
                    case VISIT_TYPE::JUMP_UP:

                        break;
                    case VISIT_TYPE::UP:

                        break;
                    case VISIT_TYPE::DOWN:

                        break;
                    default:
                        break;
                    }
                }
            }

        }
    }else{

    }

}

void MovePlatform::walkLeft(LineSegment area)
{
    int target = area[0].first - 5;

    WalkBase walk_to_platform(target);
    walk_to_platform.m_deviation = 0;
    walk_to_platform.run();
}
