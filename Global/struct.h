#ifndef STRUCT_H
#define STRUCT_H

#include <QCoreApplication>
#include <QPoint>
#include <QJsonArray>
#include <QJsonObject>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include <set>
#include <utility>
#include <cmath>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include <vector>
#include <QTime>
#include <unordered_set>

#define COLOR_CLIMB cv::Vec3b(0,0xFF,00)
#define COLOR_LINE cv::Vec3b(0,0,0)
#define COLOR_WALL cv::Vec3b(0xFF,0,0)
#define COLOR_AIR cv::Vec3b(0xFF,0xFF,0xFF)

template<typename T>
const T& clamp(const T& value, const T& low, const T& high) {
    return std::min(std::max(value, low), high);
}

typedef struct
{
    float m[16];
}Matrix4x4;

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

struct IntRect{
    int x_left;
    int y_top;
    int x_right;
    int y_bottom;

    static IntRect fromJsonArray(const QJsonArray& arr) {
        if (arr.size() >= 4) {
            return {arr[0].toInt(), arr[1].toInt(), arr[2].toInt(), arr[3].toInt()};
        }
        return {0, 0, 0, 0};
    }
};

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

typedef struct{
    int platform_id;
    int type;// -1空气；0地板；1攀爬；2墙体
    int x;
    int y;
}PlatCoor;

struct MapPlat {
    cv::Mat room_map;                          // 可视化地图
    std::vector<std::vector<PlatCoor>> plat_map;  // 坐标数据
};

typedef struct {
    int general_atk;
    int skill_atk1;
    int jump;
    int gohome;
    int skillteleport;
}BindKey;

typedef struct {
    int type; // 攻击方式 0普通攻击 1技能攻击1
    cv::Point range; // 攻击范围
}AttackInfo;

// ====== 计划任务 ======

enum TaskType
{
    CleanRoom = 0,  // 刷图
    Rest = 1,   // 休息
    Quest = 2    // 任务
};

typedef struct
{
    int id;
    TaskType type;
    int option1_key;
    QString option1_value;
    int option2_key;
    QString option2_value;
    int runTime;
}Task;

typedef struct{
    int id;
    QString type;
    QString option1_label;
    QString option1_text;
    QString option2_label;
    QString option2_text;
    QString excute_time;
}TaskInfo;

typedef struct {
    int id;
    QString name;
    std::vector<int> platforms;
}RoomPath;

typedef struct{
    int id;
    QString name;
    int platform;
}RoomRest;

typedef struct {
    int id;
    QString name;
    QString uuid;
    std::vector<RoomPath> paths;
    std::vector<RoomRest> rests;
}MapRoomPath;

typedef struct{
    int id;
    QString name;
}NPCQuest;

typedef struct{
    int buff1;
}SkillRecast;

typedef struct{
    QString uuid;
    QString name;
    int     type;
    int platform;
    IntRect area;
}TargetRoom;

typedef struct{
    QString uuid;
    QString name;
    int     type;
    std::vector<TargetRoom> targets;
}RoomNode;

enum RoomType
{
    RoomMonster = 0,  // 怪物房
    RoomTown = 1,   // 城镇
    RoomPotionShop = 2    // 药水店
};

typedef struct{
    int x;
    int y;
    int width;
    int height;
}GameWindow;

// 检测结果结构体
struct Detection {
    int class_id;
    float confidence;
    cv::Rect box;
    int track_id;  // 用于跟踪
};

// 跟踪器状态结构体
struct TrackState {
    int id;
    cv::Rect box;
    cv::KalmanFilter kf;
    int missed_frames;
    int age;
    bool active;
    float confidence;

    TrackState() : id(-1), missed_frames(0), age(0), active(true), confidence(0) {}
};

#endif // STRUCT_H
