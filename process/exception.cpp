#include "exception.h"

#include "driver/msdk/msdkDriver.h"
#include <QDebug>


LONG WINAPI BattleCrashHandler(EXCEPTION_POINTERS* pException)
{
    MSDK::cleanRunKey();
    MSDK::ReleaseAllKey();

    // 可选：生成转储文件

    QString dumpPath = "D:\\data\\dump\\output\\dump";
    // QString dumpPath = QCoreApplication::applicationDirPath() + "/output/dump/";
    qDebug() << "dump路径：" << dumpPath;
    QDateTime logDateTime = QDateTime::currentDateTime();
    QString logTimeString = logDateTime.toString("yyyy-MM-dd-hh-mm-ss");
    QString dumpSavePath = QString("%1/thread_crash_%2.dmp").arg(dumpPath).arg(logTimeString);
    HANDLE hFile = CreateFile((LPCWSTR)dumpSavePath.utf16(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ExceptionPointers = pException;
        dumpInfo.ClientPointers = FALSE;

        // BOOL success = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &dumpInfo, nullptr, nullptr);
        BOOL success = MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            (MINIDUMP_TYPE)(
                MiniDumpWithFullMemory |
                MiniDumpWithHandleData |
                MiniDumpWithThreadInfo |
                MiniDumpWithUnloadedModules |
                MiniDumpWithFullMemoryInfo |
                MiniDumpWithIndirectlyReferencedMemory |
                MiniDumpWithDataSegs
                ),
            &dumpInfo,  // 如果是异常转储
            nullptr,
            nullptr
            );
        if (!success) {
            DWORD err = GetLastError();
            qDebug() << "MiniDumpWriteDump failed, GetLastError:" << err;
        }

        CloseHandle(hFile);
    }

    MessageBoxA(NULL, "Error!", "Error message!", MB_OK);
    return EXCEPTION_EXECUTE_HANDLER;
}
