#ifndef STRUCT_H
#define STRUCT_H

#include <QCoreApplication>

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

#endif // STRUCT_H
