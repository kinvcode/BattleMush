#ifndef DRIVERLOAD_H
#define DRIVERLOAD_H

#include <windows.h>
#include <tlhelp32.h>
#include <QString>

class DriverLoad
{
public:
    DriverLoad();

    bool loadCommonDriver();
    //
    //得到加载驱动权限的令牌
    //
    BOOL enableDrvLoadPrivilege();

    // 安装驱动

    //
    //加载驱动
    //
    BOOL installDriver(const QString& ServiceName, const QString& DriverPath);

    //
    //启动驱动
    //
    BOOL startDriver(const QString& ServiceName);

    //
    //停止驱动
    //
    BOOL stopDriver(const QString& ServiceName);

    //
    //卸载驱动
    //
    BOOL unInstallDriver(const QString& ServiceName);
private:
    DWORD m_errorCode;
};

#endif // DRIVERLOAD_H
