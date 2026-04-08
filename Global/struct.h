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
    int id;
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
}FootholdRect;

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
}Line;

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
    QString id;
    QString name;
    MapBounding map_bounding;
    std::vector<ClimbRect> climbs;
    std::vector<FootholdRect> footholds;
    std::vector<Wall> wall_list;
    std::vector<Line> line_list;
}MapInfo;

typedef struct{
    quint64 entity;
    int foothold_id;
    int line_id;
    cv::Point pos;
}Monster;

#endif // STRUCT_H
