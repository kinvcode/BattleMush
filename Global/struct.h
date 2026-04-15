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
    int x_left;
    int y_top;
    int x_right;
    int y_bottom;
}ClimbRect;

typedef struct{
    int id; // footholdID
    int x_left;
    int y_left;
    int x_right;
    int y_right;
}FootholdRect;

typedef struct  {
    int foot_id; // footholdID
    int x;
    int bottom;  // 较大的y值
    int top;     // 较小的y值
}Wall;

typedef struct  {
    int foot_id; // footholdID
    cv::Point start; // x_left,y_left
    cv::Point end; // x_right,y_right
    int length; // 长度
}LineChild; // 该线可能是斜线，如果是斜线：长度是欧几里得距离

// typedef struct{
//     int lineid;
//     cv::Point start;
//     cv::Point end;
//     bool hasLeftWall;
//     bool hasRightWall;
//     std::vector<LineChild> line_child;
// }Line; // 舍弃

typedef struct {
    int id; // 平台id，由算法内部自动分配。一般就是vector的索引
    int type; // 平台类型：0地平线（由一系列连续的地板拼接而成） 1攀爬体（梯子，绳索等）
    cv::Point start; // 地平线：x_left,y_left 攀爬体：x_left,y_top
    cv::Point end;   // 地平线：x_right,y_right 攀爬体：x_right,y_bottom
    bool hasLeftWall; // 地平线：左边是否有墙壁
    bool hasRightWall; // 地平线：左边是否有墙壁
    std::vector<LineChild> line_child; // 地板列表
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

// A平台到达B平台的方式
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

// 更详细的结构
typedef struct{
    std::pair<int,int> platform_a;
    std::pair<int,int> platform_b;
    int length; // 访问路径长度
}VisitSegment;
typedef std::vector<VisitSegment> VisitSegmentList; // 暂时用不到

// 段的集合。段：x_left,x_right。比如：100,200。一个段表示一个x轴范围
typedef std::vector<std::pair<int,int>> LineSegment;

typedef struct{
    VISIT_TYPE visit_type; // 到达方式
    LineSegment area; // A到达B时，A平台的位置
}NodeVisit;

typedef struct{
    int id; // 目标平台ID
    int type; // 目标平台类型 0地平线 1攀爬体
    std::vector<NodeVisit> visit_list; // 到达目标平台的方式集合
}TargetNode;

typedef struct {
    int id; // 当前平台ID
    int type; // 0地平线 1攀爬体
    // std::vector<TargetNode> targets; //可以到达的平台
    std::unordered_map<int, TargetNode> targets;
}PathNode;

// 坐标集合
typedef std::vector<cv::Point> TargetPoints;

typedef struct{
    QString id; // 原子数据
    QString name; // 原子数据
    MapBounding map_bounding; // 原子数据
    std::vector<ClimbRect> climbs; // 原子数据
    std::vector<FootholdRect> footholds; // 原子数据
    std::vector<Wall> wall_list; // 原子数据
    std::vector<Platform> platform_list; // 平台列表
    std::unordered_map<int, PathNode> path_nodes;
    cv::Mat map_mat; // 二次处理数据
}MapInfo;


typedef struct {
    int general_atk;
    int skill_atk1;
    int jump;
    int gohome;
}BindKey;

typedef struct {
    int type; // 攻击方式 0普通攻击 1技能攻击1
    cv::Point range; // 攻击范围
}AttackInfo;

#endif // STRUCT_H
