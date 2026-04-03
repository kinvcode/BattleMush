#ifndef HELPER_H
#define HELPER_H

#include <windows.h>
#include <QString>
#include <QDateTime>
#include <QDebug>
#include "global/struct.h"
#include <unordered_set>

class Helper
{
public:
    // 搜索Entity的子组件
    static quint64 findChild(quint64 parent, QString name);

    // 搜索服务指针
    static quint64 findService(quint64 allservice, QString name);

    // 获取地图内所有的攀爬组件
    static std::unordered_set<quint64> findClimbableComponentSet(quint64 cur_map);

    // 获取Entity的文本组件字符串
    static QString getEntityTextComponentString(quint64 entity);

    // 获取Entity名称

    // 遍历攀爬物体
    static void dumpClimbList(quint64 cur_map, quint64 collisionservice);

    // 遍历地板
    static void dumpTilemapList(quint64 map);

    // 分割字符串
    static bool parseHPString(const QString& Text, int& left, int& right);

};

class DebugOutput
{
public:
    // 格式化输出
    static void out(const QString& message);
};

class Math{
public:
    // 计算攀爬物体RECT
    static void ComputeClimbRect(const ClimbOrigin* climb, ClimbRect& climb_rect);

    // 保留2位小数点
    static float roundHalfAwayFromZero_bit(float value, int decimalPlaces = 2);
};

#endif // HELPER_H
