#ifndef RAND_H
#define RAND_H

#include <QRandomGenerator>

class Rand
{
public:
    // 带有符号的名字
    static QString nameWithSymbol();

    // 不带符号的名字
    static QString nameWithoutSymbol();

    // 随机返回bool值
    static bool getBool();
};

#endif // RAND_H
