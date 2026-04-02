#ifndef HELPER_H
#define HELPER_H

#include <windows.h>
#include <QString>
#include <QDateTime>
#include <QDebug>

class Helper
{
public:
    Helper();
};

class DebugOutput
{
public:
    // 格式化输出
    static void out(const QString& message);
};

#endif // HELPER_H
