#ifndef HELPER_H
#define HELPER_H

#include <windows.h>
#include <QString>
#include <QDateTime>
#include <QDebug>

class Helper
{
public:
    // 搜索Entity的子组件
    static quint64 findChild(quint64 parent, QString name);

    // 获取Entity的文本组件字符串
    static QString getEntityTextComponentString(quint64 entity);

    // 分割字符串
    static bool parseHPString(const QString& Text, int& left, int& right);

};

class DebugOutput
{
public:
    // 格式化输出
    static void out(const QString& message);
};

#endif // HELPER_H
