#ifndef HARDWARE_H
#define HARDWARE_H

#include <QProcess>
#include <QCryptographicHash>

class Hardware
{
public:
    // 获取硬件信息
    static QString getHardwareInfo(const QString &cmd);

    // 获取主板信息
    static QString motherboard();

    // 获取CPU信息
    static QString cpu();

    // 哈希值
    static QString sha256(const QString &input);

    // 机器码
    static QString generateMachineCode();
};

#endif // HARDWARE_H
