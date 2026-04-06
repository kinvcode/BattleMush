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

typedef struct {
    float x;
    float y;
    float z;
}COORDINATE;

typedef struct  {
    float left;
    float top;
    float factor1;
    float factor2;
}ClimbOrigin;

typedef struct {
    float x_left;
    float y_top;
    float x_right;
    float y_bottom;
}ClimbRect;

typedef struct{
    int id;
    float x_left;
    float y_left;
    float x_right;
    float y_right;
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

template<typename T>
const T& clamp(const T& value, const T& low, const T& high) {
    return std::min(std::max(value, low), high);
}

#endif // STRUCT_H
