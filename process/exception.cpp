#include "exception.h"

#include "driver/msdk/msdkDriver.h"
#include <QDebug>
#include <cstdio>

namespace {

void ensureDumpDir(const wchar_t* dir)
{
    wchar_t buf[MAX_PATH] = {};
    wcsncpy_s(buf, dir, _TRUNCATE);
    for (wchar_t* p = buf; *p; ++p) {
        if (*p != L'\\' && *p != L'/')
            continue;
        if (p <= buf + 2)
            continue;
        const wchar_t saved = *p;
        *p = L'\0';
        CreateDirectoryW(buf, nullptr);
        *p = saved;
    }
    CreateDirectoryW(buf, nullptr);
}

} // namespace

LONG WINAPI BattleCrashHandler(EXCEPTION_POINTERS* pException)
{
    static volatile LONG entered = 0;
    if (InterlockedCompareExchange(&entered, 1, 0) != 0)
        return EXCEPTION_EXECUTE_HANDLER;

    // 处理函数内部如果再次崩溃，不再重入，避免第二次把转储写坏
    SetUnhandledExceptionFilter(nullptr);

    MSDK::cleanRunKey();
    MSDK::ReleaseAllKey();

    const wchar_t* dumpDir = L"D:\\data\\dump\\output\\dump";
    ensureDumpDir(dumpDir);

    SYSTEMTIME st = {};
    GetLocalTime(&st);
    wchar_t dumpSavePath[MAX_PATH] = {};
    swprintf_s(dumpSavePath,
               L"%s\\thread_crash_%04u-%02u-%02u-%02u-%02u-%02u.dmp",
               dumpDir,
               st.wYear, st.wMonth, st.wDay,
               st.wHour, st.wMinute, st.wSecond);

    HANDLE hFile = CreateFileW(dumpSavePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        const DWORD err = GetLastError();
        qDebug() << "dump路径：" << QString::fromWCharArray(dumpSavePath);
        qDebug() << "CreateFile failed, GetLastError:" << err;
    } else {
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo = {};
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ExceptionPointers = pException;
        dumpInfo.ClientPointers = FALSE;

        const BOOL success = MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            static_cast<MINIDUMP_TYPE>(
                MiniDumpWithFullMemory |
                MiniDumpWithHandleData |
                MiniDumpWithThreadInfo |
                MiniDumpWithUnloadedModules |
                MiniDumpWithFullMemoryInfo |
                MiniDumpWithIndirectlyReferencedMemory |
                MiniDumpWithDataSegs),
            pException ? &dumpInfo : nullptr,
            nullptr,
            nullptr);

        if (!success) {
            const DWORD err = GetLastError();
            qDebug() << "dump路径：" << QString::fromWCharArray(dumpSavePath);
            qDebug() << "MiniDumpWriteDump failed, GetLastError:" << err;
        } else {
            qDebug() << "dump已写入：" << QString::fromWCharArray(dumpSavePath);
        }

        FlushFileBuffers(hFile);
        CloseHandle(hFile);
    }

    if (pException && pException->ExceptionRecord) {
        qDebug() << "exception code:" << Qt::hex << pException->ExceptionRecord->ExceptionCode;
    }

    MessageBoxA(nullptr, "Error!", "Error message!", MB_OK);
    return EXCEPTION_EXECUTE_HANDLER;
}
