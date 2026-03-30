#include "rand.h"
#include <QString>


QString Rand::nameWithSymbol()
{
    const QString characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+-=[]{};\\|:\",.<>/?`~";
    QString randomString;

    int length = QRandomGenerator::global()->bounded(8, 13);

    for (int i = 0; i < length; ++i) {
        // 生成随机索引
        int randomIndex = QRandomGenerator::global()->bounded(characters.length());

        // 将随机字符添加到结果字符串中
        randomString.append(characters.at(randomIndex));
    }

    return randomString;
}

QString Rand::nameWithoutSymbol()
{
    const QString characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    QString randomString;

    int length = QRandomGenerator::global()->bounded(5, 10);

    for (int i = 0; i < length; ++i) {
        // 生成随机索引
        int randomIndex = QRandomGenerator::global()->bounded(characters.length());

        // 将随机字符添加到结果字符串中
        randomString.append(characters.at(randomIndex));
    }

    return randomString;
}

bool Rand::getBool()
{
    return QRandomGenerator::global()->generate() % 8 == 0;
}
