#ifndef STRUCT_H
#define STRUCT_H

#include <QCoreApplication>

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
    float x_left;
    float y_left;
    float x_right;
    float y_right;
}FootholdRect;

#endif // STRUCT_H
