#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <windows.h>
#include <dbghelp.h>
#include <QtCore>

LONG WINAPI BattleCrashHandler(EXCEPTION_POINTERS* pException);

#endif // EXCEPTION_H
