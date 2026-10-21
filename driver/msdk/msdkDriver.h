#pragma once
#ifndef MSDK_H
#define MSDK_H
#include <QRandomGenerator>
#include <QThread>
#include "windows.h"
#include "msdk.h"
#include "global/controller.h"
#include "global/globalplayer.h"
#include "driver/msdk/usbhidkeycode.h"
#include "process/process.h"
#include "helper/rand.h"

class MSDK
{
public:
    // 打开驱动端口
    static HANDLE Open(int pid);

    static HANDLE Open2(int vid,int pid);

    // 设置分辨率 0正确 -10不支持绝对移动
    static int ResolutionUsed(int x, int y);

    // 获取鼠标按键状态：-1失败，0弹起，1按下
    static int GetMouseState(int code);

    // 鼠标左键点击
    static void LeftClick();

    // 鼠标左键双击
    static void LeftDoubleClick();

    // 鼠标左键按下
    static void LeftDown();

    // 鼠标左键弹起
    static void LeftUp();

    // 鼠标右键点击
    static void RightClick();

    // 鼠标右键按键
    static void RightDown();

    // 鼠标右键弹起
    static void RightUp();

    // 弹起所有鼠标按键
    static void ReleaseAllMouse();

    // 鼠标滚轮 负数（向下）和正数（向上）
    static void MouseWheel(int num);

    // 获取鼠标位置
    static cv::Point2i GetPos();

    // 重置鼠标位置（屏幕左上角），0,0
    static void ResetMousePos();

    // 相对当前位置移动
    static void MoveR(int x,int y);

    // 移动到指定位置
    static void MoveTo(int x,int y);

    // 移动到RECT
    static void MoveToRect(cv::Rect range);

    // 移动到空白区域
    static void MoveToFree();

    // 仿真移动
    static void humanMove(int targetX, int targetY);

    // 键盘按键状态
    static int KeyState(int code);

    // 键盘按下
    static void KeyDown(int code);

    // 键盘弹起
    static void KeyUp(int code);

    static void KeyUp2(int code);

    // 键盘所有键弹起
    static void ReleaseAllKey();

    // 弹起方向键
    static void cleanRunKey();

    // 按键
    static void keyPress(int code, int repeats);

    // 输入指定字符
    static void KeyInputWord(char word);

    // 输入 Unicode
    static void KeyInputUnicode(QString str);

    // 随机延迟
    static void DelayRandom(int min, int max);

    // 弹起上下左右方向键
    static void closeRunKey();

    // 设置延迟
    static void setParam(int type,int param1,int param2);
};

#endif // MSDK_H
