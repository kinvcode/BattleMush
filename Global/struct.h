#ifndef STRUCT_H
#define STRUCT_H

#include <QCoreApplication>
#include <QPoint>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include <set>
#include <utility>
#include <cmath>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include <vector>

#define COLOR_CLIMB cv::Vec3b(0,0xFF,00)
#define COLOR_LINE cv::Vec3b(0,0,0)
#define COLOR_WALL cv::Vec3b(0xFF,0,0)
#define COLOR_AIR cv::Vec3b(0xFF,0xFF,0xFF)

template<typename T>
const T& clamp(const T& value, const T& low, const T& high) {
    return std::min(std::max(value, low), high);
}

typedef struct {
    float x;
    float y;
    float z;
}COORDINATE;

typedef struct{
    float x_left;
    float y_top;
    float x_right;
    float y_bottom;
}FloatRect;

typedef struct  {
    float left;
    float top;
    float factor1;
    float factor2;
}ClimbOrigin;

typedef struct {
    // int id; // 字段不需要
    int x_left;
    int y_top;
    int x_right;
    int y_bottom;
}ClimbRect;

typedef struct{
    int id;
    int x_left;
    int y_left;
    int x_right;
    int y_right;
}FootholdRect; // 需要改名

typedef struct  {
    int foot_id;
    int x;
    int bottom;  // 较大的y值
    int top;     // 较小的y值
}Wall;

typedef struct  {
    int foot_id;
    cv::Point start;
    cv::Point end;
    int length;
}LineChild;

typedef struct{
    int lineid;
    cv::Point start;
    cv::Point end;
    bool hasLeftWall;
    bool hasRightWall;
    std::vector<LineChild> line_child;
}Line; // 舍弃

typedef struct {
    int id;
    int type; //0线 1攀爬
    cv::Point start; // x_left,y_top
    cv::Point end;   // x_right,y_bottom
    bool hasLeftWall;
    bool hasRightWall;
    std::vector<LineChild> line_child;
}Platform; // 平台结构，同时包含地板和攀爬。是数据二次处理的结果

typedef struct{
    int x_left;
    int y_bottom;
    int x_right;
    int y_top;
    int x_offset;
    int y_offset;
    int width;
    int height;
}MapBounding;

typedef struct{
    QString id; // 原子数据
    QString name; // 原子数据
    MapBounding map_bounding; // 原子数据
    std::vector<ClimbRect> climbs; // 原子数据
    std::vector<FootholdRect> footholds; // 原子数据
    std::vector<Wall> wall_list; // 原子数据
    std::vector<Platform> platform_list; // 平台列表
    cv::Mat map_mat; // 二次处理数据

}MapInfo;

typedef struct{
    quint64 entity;
    int foothold_id;
    int line_id;
    cv::Point pos;
}Monster;

typedef struct{
    quint64 entity;
    int line_id;
    cv::Point pos;
}Item;


// enum PATH_TYPE{
//     walk_left = 0,
//     walk_right = 1,
//     jump_left = 3,
//     jump_right = 4,
//     jump_up = 5,
//     jump_down = 6,
//     up = 7,
//     down = 8,
//     teleport = 9,
//     skill = 10
// };

// typedef struct{
//     int line_id;
//     PATH_TYPE path_type;
// }NodeLink;

// typedef struct{
//     int line_id;
//     std::vector<NodeLink> targets;
// }PathNode;

/*================================================*/
enum VISIT_TYPE{
    WALK_LEFT = 0,
    WALK_RIGHT = 1,
    JUMP_LEFT = 3,
    JUMP_RIGHT = 4,
    JUMP_UP = 5,
    JUMP_DOWN = 6,
    UP = 7,
    DOWN = 8,
    TELEPORT = 9,
    SKILL = 10
};

typedef std::vector<std::pair<int,int>> LineSegment;

struct NodeVisit{
    VISIT_TYPE visit_type; // 到达方式
    LineSegment area; // 可以到达的区域
};

struct TargetNode{
    int id; // 目标平台ID
    int type; // 目标平台类型
    std::vector<NodeVisit> visit_list; // 到达目标平台的方式集合
};

struct PathNode{
    int id; // 当前平台ID
    int type; // 0线 1攀爬
    std::vector<TargetNode> targets; //可以到达的平台
};

// 坐标集合
typedef std::vector<cv::Point> TargetPoints;

/*=====================================================*/






















#endif // STRUCT_H
