#include "msdkDriver.h"
#include "global/globalplayer.h"
#include <QDebug>

HANDLE MSDK::Open(int pid)
{
    return Ap5isNXMcaWr(pid);
}

HANDLE MSDK::Open2(int vid,int pid)
{
    return i4dUMV3JPnm1(vid,pid);
}

int MSDK::ResolutionUsed(int x, int y)
{
    return qEPoFZf2uldH(Gplayer.MSDK_HANDLE, x, y);
}

int MSDK::GetMouseState(int code)
{
    return kLNkJH89D(Gplayer.MSDK_HANDLE, code);
}

void MSDK::LeftClick()
{
    eEXShIvePH(Gplayer.MSDK_HANDLE, 1);
}

void MSDK::LeftDoubleClick()
{
    e802WadGw(Gplayer.MSDK_HANDLE, 1);
}

void MSDK::LeftDown()
{
    SndzmqrfS6YH(Gplayer.MSDK_HANDLE);
}

void MSDK::LeftUp()
{
    EuWssRuD2(Gplayer.MSDK_HANDLE);
}

void MSDK::RightClick()
{
    bz4u95VFqV(Gplayer.MSDK_HANDLE, 1);
}

void MSDK::RightDown()
{
    Z5edmlUJUUn(Gplayer.MSDK_HANDLE);
}

void MSDK::RightUp()
{
    Q4q22sHu8g(Gplayer.MSDK_HANDLE);
}

void MSDK::ReleaseAllMouse()
{
    Rb9Lzwm1DQ(Gplayer.MSDK_HANDLE);
}

void MSDK::MouseWheel(int num)
{
    ghBhQngjohp(Gplayer.MSDK_HANDLE, num);
}

cv::Point2i MSDK::GetPos()
{
    int x, y;
    HrsNUDjFBO(Gplayer.MSDK_HANDLE, &x, &y);
    cv::Point2i coor = { x,y };
    return coor;
}

void MSDK::ResetMousePos()
{
    sYgPZrEaaNs(Gplayer.MSDK_HANDLE);
}

void MSDK::MoveR(int x, int y)
{
    QPoint currentPos = QCursor::pos();
    int currentX = currentPos.x();
    int currentY = currentPos.y();

    x += currentX;
    y += currentY;

    MoveTo(x, y);
}

void MSDK::MoveTo(int x, int y)
{
    // humanMove(x, y);
    Jo5KxStTbxF(Gplayer.MSDK_HANDLE,x, y);
}

void MSDK::MoveToRect(cv::Rect range)
{
    if(range.width == 0 || range.height == 0){
        return;
    }
    int x = range.x + rand() % range.width;
    int y = range.y + rand() % range.height;
    MoveTo(Gplayer.game_window.x + x, Gplayer.game_window.y + y);
}

void MSDK::humanMove(int targetX, int targetY)
{
    targetX = Gplayer.game_window.x + targetX;
    targetY = Gplayer.game_window.y + targetY;
    Jo5KxStTbxF(Gplayer.MSDK_HANDLE,targetX, targetY);
    return;

    // qDebug() << "移动位置:" << targetX << targetY;

    QPoint currentPos = QCursor::pos();
    int currentX = currentPos.x();
    int currentY = currentPos.y();

    // 计算直径
    int length = qSqrt(qPow((currentX-targetX),2)+qPow((currentY-targetY),2));
    int controllValue = 5;
    int rand_value = 15;
    if(length < 300)
    {
        // 控制弧度
        controllValue = 5;
        // 控制抖动
        rand_value = 3;
    }

    // 随机化控制点1的偏移量
    int control1OffsetX = QRandomGenerator::global()->bounded(-controllValue, controllValue);
    int control1OffsetY = QRandomGenerator::global()->bounded(-controllValue, controllValue);

    // 随机化控制点2的偏移量
    int control2OffsetX = QRandomGenerator::global()->bounded(-controllValue, controllValue);
    int control2OffsetY = QRandomGenerator::global()->bounded(-controllValue, controllValue);

    // 控制点1坐标
    int control1X = currentX + (targetX - currentX) / 3 + control1OffsetX;
    int control1Y = currentY - qAbs(targetY - currentY) / 2 - controllValue + control1OffsetY; // 调整控制点1的Y坐标偏移量
    control1X = targetX;
    control1Y = targetY;

    // 控制点2坐标
    int control2X = currentX + 2 * (targetX - currentX) / 3 + control2OffsetX;
    int control2Y = targetY + qAbs(targetY - currentY) / 2 + controllValue + control2OffsetY; // 调整控制点2的Y坐标偏移量
    control2Y = targetY;

    // 贝塞尔曲线轨迹
    for (double t = 0; t <= 1; t += 0.03) {
        // 计算当前点的坐标
        qreal x = qPow(1 - t, 3) * currentX + 3 * qPow(1 - t, 2) * t * control1X + 3 * (1 - t) * qPow(t, 2) * control2X + qPow(t, 3) * targetX;
        qreal y = qPow(1 - t, 3) * currentY + 3 * qPow(1 - t, 2) * t * control1Y + 3 * (1 - t) * qPow(t, 2) * control2Y + qPow(t, 3) * targetY;

        // 添加较低频率的随机抖动效果
        if (QRandomGenerator::global()->bounded(100) < rand_value) {
            int jitterX = QRandomGenerator::global()->bounded(-5, 5);
            int jitterY = QRandomGenerator::global()->bounded(-5, 5);
            x += jitterX;
            y += jitterY;
        }

        // 移动鼠标
        Jo5KxStTbxF(Gplayer.MSDK_HANDLE, qRound(x), qRound(y));

        // 等待一段时间
        QThread::msleep(QRandomGenerator::global()->bounded(5, 15));
    }

    Jo5KxStTbxF(Gplayer.MSDK_HANDLE,targetX, targetY);
}

// 获取按键状态: -1失败 0弹起状态 1按下状态
int MSDK::KeyState(int code)
{
    return rFpti6KthJ(Gplayer.MSDK_HANDLE, code);
}

// 按下某个按键
void MSDK::KeyDown(int code)
{
    if (rFpti6KthJ(Gplayer.MSDK_HANDLE, code) == 0)
    {
        rdpWSY0nFv(Gplayer.MSDK_HANDLE, code);
    }
}

// 弹起某个按键
void MSDK::KeyUp(int code)
{
    if (rFpti6KthJ(Gplayer.MSDK_HANDLE, code) == 1)
    {
        oihUU7DIfVcX(Gplayer.MSDK_HANDLE, code);
    }
}

void MSDK::KeyUp2(int code)
{
    if (rFpti6KthJ(Gplayer.MSDK_HANDLE, code) == 1)
    {
        oihUU7DIfVcX(Gplayer.MSDK_HANDLE, code);
    }
}

// 弹起所有按键
void MSDK::ReleaseAllKey()
{
    Na8FeYnvveiZ(Gplayer.MSDK_HANDLE);
}

void MSDK::cleanRunKey()
{
    KeyUp2(Keyboard_RightArrow);
    KeyUp2(Keyboard_LeftArrow);
    KeyUp2(Keyboard_UpArrow);
    KeyUp2(Keyboard_DownArrow);
}

// 按键以及次数
void MSDK::keyPress(int code, int repeats)
{
    nnN3A2qfth(Gplayer.MSDK_HANDLE, code, repeats);
}

void MSDK::KeyInputWord(char word)
{
    pcmgzzXYA(Gplayer.MSDK_HANDLE,&word,1);
}

void MSDK::KeyInputUnicode(QString str)
{
    if(str.isEmpty())
        return;

    // 使用 QVector 并包含结尾空字符
    QVector<ushort> utf16Vector;
    utf16Vector.reserve(str.length() + 1);

    const ushort* src = str.utf16();
    for(int i = 0; i <= str.length(); i++)  // <= 包含结尾空字符
    {
        utf16Vector.append(src[i]);
    }

    int byteLen = utf16Vector.size() * 2;

    // 检查长度限制
    if(byteLen > 512)
    {
        qDebug() << "字符串太长，最大支持512字节";
        int maxChars = 512 / 2;
        utf16Vector.resize(maxChars);
        utf16Vector[maxChars - 1] = 0;  // 确保结尾有空字符
        byteLen = utf16Vector.size() * 2;
    }

    // 调用接口
    char* word = reinterpret_cast<char*>(utf16Vector.data());

    PQn5pR2iK(Gplayer.MSDK_HANDLE,word,byteLen);
}


// 随机延迟
void MSDK::DelayRandom(int min, int max)
{
    YYyWL2SSFlkB(min, max);
}

void MSDK::closeRunKey()
{
    // 释放上
    if (rFpti6KthJ(Gplayer.MSDK_HANDLE, Keyboard_UpArrow) == 1) {
        oihUU7DIfVcX(Gplayer.MSDK_HANDLE, Keyboard_UpArrow);
        ProcessHelper::programDelay(50);
    }

    // 释放右
    if (rFpti6KthJ(Gplayer.MSDK_HANDLE, Keyboard_RightArrow) == 1) {
        oihUU7DIfVcX(Gplayer.MSDK_HANDLE, Keyboard_RightArrow);
        ProcessHelper::programDelay(50);
    }

    // 释放下
    if (rFpti6KthJ(Gplayer.MSDK_HANDLE, Keyboard_DownArrow) == 1) {
        oihUU7DIfVcX(Gplayer.MSDK_HANDLE, Keyboard_DownArrow);
        ProcessHelper::programDelay(50);
    }

    // 释放左
    if (rFpti6KthJ(Gplayer.MSDK_HANDLE, Keyboard_LeftArrow) == 1) {
        oihUU7DIfVcX(Gplayer.MSDK_HANDLE, Keyboard_LeftArrow);
        ProcessHelper::programDelay(50);
    }
}

void MSDK::setParam(int type,int param1,int param2)
{
    whcg0cGgT(Gplayer.MSDK_HANDLE, type,param1,param2);
}
