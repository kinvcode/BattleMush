#include "hardware.h"
#include "vmp/VMProtectSDK.h"

namespace {

// wmic 默认第一行是列名，后面才是取值。输出为空、失败或只有表头时返回空串，避免越界。
__declspec(noinline) QString firstHardwareValue(const QString &output)
{
    VMProtectBegin(__FUNCSIG__);
    QString result;
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (int i = 1; i < lines.size(); ++i) {
        const QString value = lines.at(i).trimmed();
        if (!value.isEmpty()) {
            result = value;
            break;
        }
    }
    VMProtectEnd();
    return result;
}

} // namespace

__declspec(noinline) QString Hardware::getHardwareInfo(const QString &cmd)
{
    VMProtectBegin(__FUNCSIG__);
    QProcess process;
    process.start("cmd", QStringList() << "/c" << cmd);
    process.waitForFinished();

    QString result = process.readAllStandardOutput().trimmed();
    VMProtectEnd();
    return result;
}

__declspec(noinline) QString Hardware::motherboard()
{
    VMProtectBegin(__FUNCSIG__);
    const QString value = firstHardwareValue(getHardwareInfo("wmic baseboard get serialnumber"));
    VMProtectEnd();
    return value;
}

__declspec(noinline) QString Hardware::cpu()
{
    VMProtectBegin(__FUNCSIG__);
    const QString value = firstHardwareValue(getHardwareInfo("wmic cpu get processorid"));
    VMProtectEnd();
    return value;
}

__declspec(noinline) QString Hardware::disk()
{
    VMProtectBegin(__FUNCSIG__);
    const QString value = firstHardwareValue(getHardwareInfo("wmic diskdrive get serialnumber"));
    VMProtectEnd();
    return value;
}

__declspec(noinline) QString Hardware::sha256(const QString &input)
{
    VMProtectBegin(__FUNCSIG__);
    QByteArray byteArray = input.toUtf8();
    QByteArray hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha256);
    const QString hex = hash.toHex();
    VMProtectEnd();
    return hex;
}

namespace {

struct DeviceIdentity
{
    QString cpu;
    QString motherboard;
    QString disk;
    QString machineCode;
};

__declspec(noinline) DeviceIdentity readDeviceIdentity()
{
    VMProtectBegin(__FUNCSIG__);
    DeviceIdentity id;
    id.motherboard = Hardware::motherboard();
    id.cpu = Hardware::cpu();
    id.disk = Hardware::disk();

    QString uuid = id.motherboard + QStringLiteral("LOL") + id.cpu + QStringLiteral("LOL") + id.disk;
    if (uuid.isEmpty())
        uuid = QStringLiteral("123456");
    id.machineCode = Hardware::sha256(uuid);
    VMProtectEnd();
    return id;
}

} // namespace

__declspec(noinline) QString Hardware::generateMachineCode()
{
    VMProtectBegin(__FUNCSIG__);
    const QString code = readDeviceIdentity().machineCode;
    VMProtectEnd();
    return code;
}
