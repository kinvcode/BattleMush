#include "hardware.h"

QString Hardware::getHardwareInfo(const QString &cmd)
{
    QProcess process;
    process.start("cmd", QStringList() << "/c" << cmd);
    process.waitForFinished();

    QString result = process.readAllStandardOutput().trimmed();
    return result;
}

QString Hardware::motherboard()
{
    // return getHardwareInfo("wmic baseboard get serialnumber").split("\n").at(1).trimmed();
    QString default_motherboard = "88888888";
    QString output = getHardwareInfo("wmic baseboard get serialnumber");

    // 1. 检查是否为空
    if (output.isEmpty()) {
        return default_motherboard;
    }

    QStringList lines = output.split("\n", Qt::SkipEmptyParts); // Qt 5.14+ 可用，跳过空行

    // 2. 检查行数是否足够 (通常第一行是标题"SerialNumber"，第二行是数据)
    if (lines.size() < 2) {
        return default_motherboard;
    }

    // 3. 取第二行（索引1）并去除前后空白
    QString serial = lines.at(1).trimmed();

    // 4. 如果序列号为空，记录日志并返回默认值
    if (serial.isEmpty()) {
        return default_motherboard;
    }

    return serial;
}

QString Hardware::cpu()
{
    QString default_cpu = "666666";
    // return getHardwareInfo("wmic cpu get processorid").split("\n").at(1).trimmed();
    QString output = getHardwareInfo("wmic cpu get processorid");

    // 1. 检查是否为空
    if (output.isEmpty()) {
        return default_cpu;
    }

    QStringList lines = output.split("\n", Qt::SkipEmptyParts); // Qt 5.14+ 可用，跳过空行

    // 2. 检查行数是否足够 (通常第一行是标题"SerialNumber"，第二行是数据)
    if (lines.size() < 2) {
        return default_cpu;
    }

    // 3. 取第二行（索引1）并去除前后空白
    QString serial = lines.at(1).trimmed();

    // 4. 如果序列号为空，记录日志并返回默认值
    if (serial.isEmpty()) {
        return default_cpu;
    }

    return serial;
}

QString Hardware::sha256(const QString &input)
{
    QByteArray byteArray = input.toUtf8();
    QByteArray hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha256);
    return hash.toHex();
}

QString Hardware::generateMachineCode()
{
    // 拼接序列号
    QString combined = cpu() + motherboard();

    // 计算哈希值
    return sha256(combined);
}
