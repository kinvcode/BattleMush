#ifndef STRUCT_H
#define STRUCT_H

#include <QCoreApplication>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include <set>
#include <utility>

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

template<typename T>
const T& clamp(const T& value, const T& low, const T& high) {
    return std::min(std::max(value, low), high);
}

#endif // STRUCT_H
