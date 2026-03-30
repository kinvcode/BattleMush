#include "driverload.h"
#include <QDebug>
#include "vmp/VMProtectSDK.h"
#include <QFile>
#include <QApplication>
#include <QMessageBox>
#include "msdriverpro.h"
#include "process/process.h"

DriverLoad::DriverLoad() {}

bool DriverLoad::loadCommonDriver()
{
    VMProtectBegin("__FUNCTION__");
    DriverLoad drvLoad;

    QString service_name = "MicrosoftGuard";
    // service_name = Rand::nameWithoutSymbol();

    // QString currentDir = QCoreApplication::applicationDirPath();
    // QString drvPath = currentDir + "/MicrosoftGuard.sys"; // 驱动文件路径
    QString drvPath = "C:/Windows/System32/drivers/MicrosoftGuard.sys"; // 驱动文件路径

    QFile local_driver_file(drvPath);
    if(local_driver_file.exists()){

        // 判断是否可写
        if (!local_driver_file.isWritable()) {
            qDebug() << "驱动文件不可写";

            QFile::Permissions currentPerms = local_driver_file.permissions();
            qDebug() << "修改前权限：" << currentPerms;

            // 添加写权限（移除非写权限）
            if (local_driver_file.setPermissions(currentPerms | QFile::WriteOwner)) {
                qDebug() << "使用Qt成功修改文件权限";
            } else {
                qDebug() << "使用Qt修改权限失败：" << local_driver_file.errorString();
            }
        }

        // 判断是否停止
        // 停止驱动
        stopDriver(service_name);
        unInstallDriver(service_name);

        qDebug() << "驱动文件已存在，准备删除";
        if(!local_driver_file.remove())
        {
            QMessageBox::information(NULL,"Vega","驱动缓存文件删除失败！");


            QFile::FileError error = local_driver_file.error();
            QString errorStr = local_driver_file.errorString();

            QString message = QString("驱动缓存文件删除失败！\n"
                                      "文件: %1\n"
                                      "错误代码: %2\n"
                                      "错误描述: %3")
                                  .arg(drvPath)
                                  .arg(static_cast<int>(error))
                                  .arg(errorStr);

            // 添加更具体的错误分析
            switch (error) {
            case QFile::PermissionsError:
                message += "\n原因: 权限不足，请以管理员身份运行程序";
                break;
            // case QFile::AccessError:
            //     message += "\n原因: 文件可能正在被其他程序使用";
            //     break;
            case QFile::ResizeError:
                message += "\n原因: 文件系统错误";
                break;
            case QFile::RemoveError:
                message += "\n原因: 无法删除文件，检查文件属性和安全设置";
                break;
            default:
                break;
            }

            QMessageBox::critical(nullptr, "Vega", message);

            ProcessHelper::deleteViaCommandLine(drvPath);

            return false;
        }
    }

    // 释放文件
    QString driver_file_path = ":assets/MicrosoftGuard.sys";
    QFile driver_sys(driver_file_path);
    if(!driver_sys.exists())
    {
        qDebug() << "驱动文件丢失！";
        QMessageBox::information(NULL,"Vega","驱动文件丢失！");
        qApp->quit();
        return false;
    }

    qDebug() << "驱动文件已获取，准备下发";

    if (!QFile::copy(driver_file_path, drvPath))
    {
        qDebug() << "驱动文件下发失败！";
        QMessageBox::information(NULL,"Vega","驱动文件下发失败！");
        qApp->quit();
        return false;
    }

    qDebug() << "驱动文件已下发";


    if(!installDriver(service_name, drvPath)){
        qDebug() << "驱动安装失败";
        stopDriver(service_name);
        unInstallDriver(service_name);
        QMessageBox::information(NULL,"Vega","驱动安装失败");
        qApp->quit();
        return false;
    }

    if(!startDriver(service_name))
    {
        qDebug() << "驱动启动失败";
        stopDriver(service_name);
        unInstallDriver(service_name);
        QMessageBox::information(NULL,"Vega","驱动启动失败");
        qApp->quit();
        return false;
    }

    if (!MSDriverPRO::isLoadDriver())
    {
        qDebug() << "驱动加载失败！";
        QMessageBox::information(NULL,"Vega","驱动加载失败");
        qApp->quit();
        return false;
    }

    return true;
    VMProtectEnd();
}

BOOL DriverLoad::enableDrvLoadPrivilege()
{
    HANDLE hToken;
    LUID sedebugnameValue;
    TOKEN_PRIVILEGES tkp;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        return  FALSE;
    }
    if (!LookupPrivilegeValue(NULL, SE_LOAD_DRIVER_NAME, &sedebugnameValue))
    {
        CloseHandle(hToken);
        return FALSE;
    }
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = sedebugnameValue;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
    {
        CloseHandle(hToken);
        return FALSE;
    }
    return TRUE;
}

BOOL DriverLoad::installDriver(const QString& ServiceName, const QString& DriverPath)
{
    LPCWSTR lpServiceName = reinterpret_cast<LPCWSTR>(ServiceName.utf16());
    LPCWSTR lpDriverPath = reinterpret_cast<LPCWSTR>(DriverPath.utf16());

    SC_HANDLE sc_manage = NULL;
    SC_HANDLE sc_service = NULL;
    BOOL bRet = FALSE;

    do
    {
        sc_manage = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        if (NULL == sc_manage)
        {
            m_errorCode = GetLastError();
            break;
        }

        sc_service = CreateService(
            sc_manage,
            lpServiceName,
            lpServiceName,
            SERVICE_ALL_ACCESS,
            SERVICE_KERNEL_DRIVER,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_IGNORE,
            lpDriverPath,
            NULL, NULL, NULL, NULL, NULL);

        if (NULL == sc_service)
        {
            m_errorCode = GetLastError();
            break;
        }

        bRet = TRUE;
    } while (0);

    if (sc_service != NULL)
    {
        CloseServiceHandle(sc_service);
        sc_service = NULL;
    }

    if (sc_manage != NULL)
    {
        CloseServiceHandle(sc_manage);
        sc_manage = NULL;
    }

    return bRet;
}

BOOL DriverLoad::startDriver(const QString& ServiceName)
{
    LPCWSTR lpServiceName = reinterpret_cast<LPCWSTR>(ServiceName.utf16());

    SC_HANDLE sc_manage = NULL;
    SC_HANDLE sc_service = NULL;
    BOOL bRet = FALSE;

    do
    {
        sc_manage = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (NULL == sc_manage)
        {
            m_errorCode = GetLastError();
            break;
        }

        sc_service = OpenService(sc_manage, lpServiceName, SERVICE_ALL_ACCESS);
        if (NULL == sc_service)
        {
            m_errorCode = GetLastError();
            break;
        }

        if (StartService(sc_service, NULL, NULL) == FALSE)
        {
            m_errorCode = GetLastError();
            break;
        }

        bRet = TRUE;
    } while (0);

    if (sc_service != NULL)
    {
        CloseServiceHandle(sc_service);
        sc_service = NULL;
    }

    if (sc_manage != NULL)
    {
        CloseServiceHandle(sc_manage);
        sc_manage = NULL;
    }

    return bRet;
}

BOOL DriverLoad::stopDriver(const QString& ServiceName)
{
    LPCWSTR lpServiceName = reinterpret_cast<LPCWSTR>(ServiceName.utf16());

    SC_HANDLE sc_manage = NULL;
    SC_HANDLE sc_service = NULL;
    SERVICE_STATUS ss = { 0 };
    BOOL bRet = FALSE;

    do
    {
        sc_manage = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (NULL == sc_manage)
        {
            m_errorCode = GetLastError();
            break;
        }

        sc_service = OpenService(sc_manage, lpServiceName, SERVICE_ALL_ACCESS);
        if (NULL == sc_service)
        {
            m_errorCode = GetLastError();
            break;
        }

        if (ControlService(sc_service, SERVICE_CONTROL_STOP, &ss) == FALSE)
        {
            m_errorCode = GetLastError();
            break;
        }

        bRet = TRUE;
    } while (0);

    if (sc_service != NULL)
    {
        CloseServiceHandle(sc_service);
        sc_service = NULL;
    }

    if (sc_manage != NULL)
    {
        CloseServiceHandle(sc_manage);
        sc_manage = NULL;
    }

    return bRet;
}

BOOL DriverLoad::unInstallDriver(const QString& ServiceName)
{
    LPCWSTR lpServiceName = reinterpret_cast<LPCWSTR>(ServiceName.utf16());

    SC_HANDLE sc_manage = NULL;
    SC_HANDLE sc_service = NULL;
    BOOL bRet = FALSE;

    do
    {
        sc_manage = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (sc_manage == NULL)
        {
            m_errorCode = GetLastError();
            break;
        }

        sc_service = OpenService(sc_manage, lpServiceName, SERVICE_ALL_ACCESS);
        if (sc_service == NULL)
        {
            m_errorCode = GetLastError();
            break;
        }

        if (DeleteService(sc_service) == FALSE)
        {
            m_errorCode = GetLastError();
            break;
        }

        bRet = TRUE;
    } while (0);

    if (sc_service != NULL)
    {
        CloseServiceHandle(sc_service);
        sc_service = NULL;
    }

    if (sc_manage != NULL)
    {
        CloseServiceHandle(sc_manage);
        sc_manage = NULL;
    }

    return bRet;
}

