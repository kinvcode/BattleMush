#pragma once
#ifndef MSDRIVERPRO_H
#define MSDRIVERPRO_H

#include <QString>
#include <QMutex>
#include <QRandomGenerator>
#include <windows.h>
#include "driver/msstruct.h"
#include "driver/mskeycode.h"

class MSDriverPRO
{
private:
    static int resolutionWidth;

    static int resolutionHeight;

    static int keypressDelayMin;

    static int keypressDelayMax;

public:
    MSDriverPRO();

    //
    //是否已加载驱动
    //
    static BOOL isLoadDriver();


    //-------------------------------配置--------------------------------------------------

    static void setResolutionUsed(int width, int height);
    static int getResolutionWidth();

    //-------------------------------进程--------------------------------------------------

    //
    //保护目标进程
    //
    static BOOL protect(ULONG pid);

    // 隐藏目标进程
    static BOOL hide(ULONG pid);

    // 恢复目标进程
    void restore();

    // 获取进程ID
    static int getPid(QString name);

    //
    //获取模块基址
    //
    static ULONG64 getModuleAddr(ULONG pid, const QString& moduleName);

    // 申请内存
    static INT64 alloc(ULONG pid, ULONG64 addr, ULONG size);

    // 释放内存
    static void free(ULONG pid, ULONG64 addr);

    // 查询内存
    static void queryMemory(ULONG pid, ULONG64 addr, MEMORY_BASIC_INFORMATION &Mbi);

    //
    //提交物理页,用于刚使用VirtualAlloc申请内存后，需要立刻写入
    //
    static BOOL probeRead(ULONG pid, ULONG64 addr);

    // 创建远程线程
    static void remoteThread(ULONG pid, ULONG64 addr);


    //-------------------------------读内存--------------------------------------------------

    //
    //读内存 字节型(BYTE)
    //
    static BYTE readByte(ULONG pid, ULONG64 addr);

    //
    //读内存 短整数型(SHORT)
    //
    static SHORT readShort(ULONG pid, ULONG64 addr);

    //
    //读内存 整数型(INT)
    //
    static INT readInt(ULONG pid, ULONG64 addr);

    //
    //读内存 长整数型(INT64)
    //
    static INT64 readInt64(ULONG pid, ULONG64 addr);

    //
    //读内存 浮点型(FLOAT)
    //
    static FLOAT readFloat(ULONG pid, ULONG64 addr);

    //
    //读内存 字节集(Bytes)
    //
    static BOOL readBytes(ULONG pid, ULONG64 addr, _Out_ PBYTE pDataBuffer, ULONG size);

    //-------------------------------写内存--------------------------------------------------

    //
    //写内存 字节型(BYTE)
    //
    static BOOL writeByte(ULONG pid, ULONG64 addr, _In_ BYTE pDataBuffer);

    //
    //写内存 短整数型(SHORT)
    //
    static BOOL writeShort(ULONG pid, ULONG64 addr, _In_ SHORT pDataBuffer);

    //
    //写内存 整数型(INT)
    //
    static BOOL writeInt(ULONG pid, ULONG64 addr, _In_ INT pDataBuffer);

    //
    //写内存 长整数型(INT64)
    //
    static BOOL writeInt64(ULONG pid, ULONG64 addr, _In_ INT64 pDataBuffer);

    //
    //写内存 浮点型(FLOAT)
    //
    static BOOL writeFloat(ULONG pid, ULONG64 addr, _In_ FLOAT pDataBuffer);

    //
    //写内存 字节集(Bytes)
    //
    static BOOL writeBytes(ULONG pid, ULONG64 addr, _In_ PBYTE pDataBuffer, ULONG size);

    //-------------------------------键鼠--------------------------------------------------

    // 设置单击按键的间隔（默认50-80）


    //-------------------------------鼠标--------------------------------------------------

    // 左键单击
    static void mouseLeftClick();

    // 左键双击
    static void mouseLeftDoubleClick();

    // 左键按下
    static void mouseLeftDown();

    // 左键弹起
    static void mouseLeftUp();

    // 右键单击
    static void mouseRightClick();

    // 右键按下
    static void mouseRightDown();

    // 右键弹起
    static void mouseRightUp();

    // 中键单击
    static void mouseMiddleClick();

    // 中键按下
    static void mouseMiddleDown();

    // 中键弹起
    static void mouseMiddleUp();

    // 读取鼠标左右中按键状态（暂时无法通过驱动实现）, 1左键 2中键 3右键
    // 返回 0弹起 1按下
    static int mouseKeyState(int keycode);

    // 弹起所有鼠标按键
    static void mouseReleaseAll();

    // 滚动鼠标滚轮 正数向下，负数向上
    static void mouseWheel(int number);

    // 将鼠标移动到相对位置
    static void mouseMoveR(int x, int y);

    // 将鼠标移动到绝对位置
    static void mouseMoveTo(int x, int y);

    //-------------------------------键盘--------------------------------------------------

    // 敲击指定按键, 可以指定按键次数times
    static void keyboardClick(int keycode, int times = 1);

    // 按下指定按键
    static void keyboardDown(int keycode);

    // 弹起指定按键
    static void keyboardUp(int keycode);

    // 读取按键状态(暂时无法通过驱动实现)
    // 返回 0弹起 1按下
    static int keyboardState(int keycode);

    // 弹起所有键盘按键
    static void keyboardReleaseAll();


private:

    //-------------------------------进程--------------------------------------------------
    static BOOL FindNearestFreeRegion(DWORD pid, ULONG64 DesiredAddress, SIZE_T RegionSize, PVOID* OutFreeBaseAddress);

    static BOOL allocMemory(ULONG pid, _Out_ PBYTE pDataBuffer, ULONG64 addr, ULONG size);

    static BOOL freeMemory(ULONG pid, ULONG64 addr);


    //-------------------------------内存--------------------------------------------------

    static BOOL readMemory(ULONG64 pid, ULONG64 addr, _Out_ PBYTE pDataBuffer, ULONG size);

    static BOOL writeMemory(ULONG64 pid, ULONG64 addr, _In_ PBYTE pDataBuffer, ULONG size);


    //-------------------------------键鼠--------------------------------------------------

    static VOID mouse(PBYTE pDataBuffer);

    static VOID keyboard(PBYTE pDataBuffer);

};

#endif // MSDRIVERPRO_H










