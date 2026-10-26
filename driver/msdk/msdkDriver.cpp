#include "msdkDriver.h"
#include <QCursor>
#include <QDebug>
#include <QVector>
#include <QtMath>
#include <string>
#include "driver/PeaceDriverAPI.h"

namespace {

// MSDK: 1左 2右 3中；PeaceDriver: 1左 2中 3右
int toPeaceMouseKey(int code)
{
    if (code == 2)
        return 3;
    if (code == 3)
        return 2;
    return code;
}

int letterKey(char lower)
{
    static const int keys[26] = {
        Keyboard_a, Keyboard_b, Keyboard_c, Keyboard_d, Keyboard_e,
        Keyboard_f, Keyboard_g, Keyboard_h, Keyboard_i, Keyboard_j,
        Keyboard_k, Keyboard_l, Keyboard_m, Keyboard_n, Keyboard_o,
        Keyboard_p, Keyboard_q, Keyboard_r, Keyboard_s, Keyboard_t,
        Keyboard_u, Keyboard_v, Keyboard_w, Keyboard_x, Keyboard_y,
        Keyboard_z
    };
    return keys[lower - 'a'];
}

int digitKey(char digit)
{
    static const int keys[10] = {
        Keyboard_0, Keyboard_1, Keyboard_2, Keyboard_3, Keyboard_4,
        Keyboard_5, Keyboard_6, Keyboard_7, Keyboard_8, Keyboard_9
    };
    return keys[digit - '0'];
}

int asciiToHidKey(char word, bool& shift)
{
    shift = false;
    if (word >= 'a' && word <= 'z')
        return letterKey(word);
    if (word >= 'A' && word <= 'Z') {
        shift = true;
        return letterKey(static_cast<char>(word - 'A' + 'a'));
    }
    if (word >= '0' && word <= '9')
        return digitKey(word);

    switch (word) {
    case ' ': return Keyboard_KongGe;
    case '\n':
    case '\r': return Keyboard_ENTER;
    case '\t': return Keyboard_Tab;
    case '-': return Keyboard_JianHao;
    case '=': return Keyboard_DengHao;
    case '[': return Keyboard_ZuoZhongKuoHao;
    case ']': return Keyboard_YouZhongKuoHao;
    case '\\': return Keyboard_FanXieGang;
    case ';': return Keyboard_FenHao;
    case '\'': return Keyboard_DanYinHao;
    case '`': return Keyboard_BoLangXian;
    case ',': return Keyboard_Douhao;
    case '.': return Keyboard_JuHao;
    case '/': return Keyboard_XieGang_WenHao;
    case '!': shift = true; return Keyboard_1;
    case '@': shift = true; return Keyboard_2;
    case '#': shift = true; return Keyboard_3;
    case '$': shift = true; return Keyboard_4;
    case '%': shift = true; return Keyboard_5;
    case '^': shift = true; return Keyboard_6;
    case '&': shift = true; return Keyboard_7;
    case '*': shift = true; return Keyboard_8;
    case '(': shift = true; return Keyboard_9;
    case ')': shift = true; return Keyboard_0;
    case '_': shift = true; return Keyboard_JianHao;
    case '+': shift = true; return Keyboard_DengHao;
    case '{': shift = true; return Keyboard_ZuoZhongKuoHao;
    case '}': shift = true; return Keyboard_YouZhongKuoHao;
    case '|': shift = true; return Keyboard_FanXieGang;
    case ':': shift = true; return Keyboard_FenHao;
    case '"': shift = true; return Keyboard_DanYinHao;
    case '~': shift = true; return Keyboard_BoLangXian;
    case '<': shift = true; return Keyboard_Douhao;
    case '>': shift = true; return Keyboard_JuHao;
    case '?': shift = true; return Keyboard_XieGang_WenHao;
    default: return 0;
    }
}

// 调用方统一传入 HID 码。PeaceDriver 使用 PS/2 Set 1 扫描码，扩展键把 0xE0 放在高字节。
const int kHidKeys[] = {
    4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 51, 52, 53, 54, 55, 56, 57,
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
    70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82,
    83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
    101, 224, 225, 226, 227, 228, 229, 230, 231
};
const int kPs2Keys[] = {
    0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18, 0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C,
    0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x1C, 0x01, 0x0E, 0x0F, 0x39, 0x0C, 0x0D, 0x1A, 0x1B, 0x2B, 0x27, 0x28, 0x29, 0x33, 0x34, 0x35, 0x3A,
    0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x57, 0x58,
    0xE037, 0x46, 0xE045, 0xE052, 0xE047, 0xE049, 0xE053, 0xE04F, 0xE051, 0xE04D, 0xE04B, 0xE050, 0xE048,
    0x45, 0xE035, 0x37, 0x4A, 0x4E, 0xE01C, 0x4F, 0x50, 0x51, 0x4B, 0x4C, 0x4D, 0x47, 0x48, 0x49, 0x52, 0x53,
    0xE05D, 0x1D, 0x2A, 0x38, 0xE05B, 0xE01D, 0x36, 0xE038, 0xE05C
};
static_assert(sizeof(kHidKeys) == sizeof(kPs2Keys), "hid/ps2 key map length mismatch");

int toPeaceKey(int code)
{
    const int count = int(sizeof(kHidKeys) / sizeof(kHidKeys[0]));
    for (int i = 0; i < count; ++i) {
        if (kHidKeys[i] == code)
            return kPs2Keys[i];
    }
    return code;
}

bool usePeaceDriver()
{
    return !Controller::m_msdk_installed;
}

// 易键鼠静态库在 x64 下会把设备路径指针截成 32 位再交给 strstr。
// 堆地址一旦超过 4GB（部分机器 ASLR 下很常见），启动就会 AV。
// 这两个包装函数里不能有 C++ 对象，否则不能使用 __try。
HANDLE openMsdkPortSEH(int pid)
{
    __try {
        return Ap5isNXMcaWr(pid);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return INVALID_HANDLE_VALUE;
    }
}

HANDLE openMsdkPortByVidPidSEH(int vid, int pid)
{
    __try {
        return i4dUMV3JPnm1(vid, pid);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return INVALID_HANDLE_VALUE;
    }
}

}

HANDLE MSDK::Open(int pid)
{
    return openMsdkPortSEH(pid);
}

HANDLE MSDK::Open2(int vid,int pid)
{
    return openMsdkPortByVidPidSEH(vid, pid);
}

int MSDK::ResolutionUsed(int x, int y)
{
    if (usePeaceDriver()) {
        PeaceDriver::setResolutionUsed(x, y);
        return 0;
    }
    return qEPoFZf2uldH(Controller::m_msdk_handle, x, y);
}

int MSDK::GetMouseState(int code)
{
    if (usePeaceDriver())
        return PeaceDriver::mouseKeyState(toPeaceMouseKey(code));
    return kLNkJH89D(Controller::m_msdk_handle, code);
}

void MSDK::LeftClick()
{
    if (usePeaceDriver()) {
        PeaceDriver::mouseLeftClick();
        return;
    }
    eEXShIvePH(Controller::m_msdk_handle, 1);
}

void MSDK::LeftDoubleClick()
{
    if (usePeaceDriver()) {
        PeaceDriver::mouseLeftDoubleClick();
        return;
    }
    e802WadGw(Controller::m_msdk_handle, 1);
}

void MSDK::LeftDown()
{
    if (usePeaceDriver()) {
        PeaceDriver::mouseLeftDown();
        return;
    }
    SndzmqrfS6YH(Controller::m_msdk_handle);
}

void MSDK::LeftUp()
{
    if (usePeaceDriver()) {
        PeaceDriver::mouseLeftUp();
        return;
    }
    EuWssRuD2(Controller::m_msdk_handle);
}

void MSDK::RightClick()
{
    if (usePeaceDriver()) {
        PeaceDriver::mouseRightClick();
        return;
    }
    bz4u95VFqV(Controller::m_msdk_handle, 1);
}

void MSDK::RightDown()
{
    if (usePeaceDriver()) {
        PeaceDriver::mouseRightDown();
        return;
    }
    Z5edmlUJUUn(Controller::m_msdk_handle);
}

void MSDK::RightUp()
{
    if (usePeaceDriver()) {
        PeaceDriver::mouseRightUp();
        return;
    }
    Q4q22sHu8g(Controller::m_msdk_handle);
}

void MSDK::ReleaseAllMouse()
{
    if (usePeaceDriver()) {
        PeaceDriver::mouseReleaseAll();
        return;
    }
    Rb9Lzwm1DQ(Controller::m_msdk_handle);
}

void MSDK::MouseWheel(int num)
{
    if (usePeaceDriver()) {
        // MSDK 正数向上，PeaceDriver 正数向下
        PeaceDriver::mouseWheel(-num);
        return;
    }
    ghBhQngjohp(Controller::m_msdk_handle, num);
}

cv::Point2i MSDK::GetPos()
{
    if (usePeaceDriver())
        return {0, 0};
    int x, y;
    HrsNUDjFBO(Controller::m_msdk_handle, &x, &y);
    return { x, y };
}

void MSDK::ResetMousePos()
{
    if (usePeaceDriver()) {
        PeaceDriver::mouseMoveTo(0, 0);
        return;
    }
    sYgPZrEaaNs(Controller::m_msdk_handle);
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
    humanMove(x, y);
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

void MSDK::MoveToFree()
{
    cv::Rect range = {Gplayer.game_window.x + 800, Gplayer.game_window.y, 332, 600};

    int x = range.x + rand() % range.width;
    int y = range.y + rand() % range.height;

    MoveTo(x, y);
}

void MSDK::humanMove(int targetX, int targetY)
{
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
        if (usePeaceDriver())
            PeaceDriver::mouseMoveTo(qRound(x), qRound(y));
        else
            Jo5KxStTbxF(Controller::m_msdk_handle, qRound(x), qRound(y));

        // 等待一段时间
        QThread::msleep(QRandomGenerator::global()->bounded(5, 15));
    }

    if (usePeaceDriver())
        PeaceDriver::mouseMoveTo(targetX, targetY);
    else
        Jo5KxStTbxF(Controller::m_msdk_handle,targetX, targetY);
}

// 获取按键状态: -1失败 0弹起状态 1按下状态
int MSDK::KeyState(int code)
{
    if (usePeaceDriver())
        return PeaceDriver::keyboardState(toPeaceKey(code));
    return rFpti6KthJ(Controller::m_msdk_handle, code);
}

// 按下某个按键
void MSDK::KeyDown(int code)
{
    if (usePeaceDriver()) {
        PeaceDriver::keyboardDown(toPeaceKey(code));
        return;
    }
    if (rFpti6KthJ(Controller::m_msdk_handle, code) == 0)
    {
        rdpWSY0nFv(Controller::m_msdk_handle, code);
    }
}

// 弹起某个按键
void MSDK::KeyUp(int code)
{
    if (usePeaceDriver()) {
        PeaceDriver::keyboardUp(toPeaceKey(code));
        return;
    }
    if (rFpti6KthJ(Controller::m_msdk_handle, code) == 1)
    {
        oihUU7DIfVcX(Controller::m_msdk_handle, code);
    }
}

void MSDK::KeyUp2(int code)
{
    if (usePeaceDriver()) {
        PeaceDriver::keyboardUp(toPeaceKey(code));
        return;
    }
    if (rFpti6KthJ(Controller::m_msdk_handle, code) == 1)
    {
        oihUU7DIfVcX(Controller::m_msdk_handle, code);
    }
}

// 弹起所有按键
void MSDK::ReleaseAllKey()
{
    if (usePeaceDriver()) {
        PeaceDriver::keyboardReleaseAll();
        return;
    }
    Na8FeYnvveiZ(Controller::m_msdk_handle);
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
    if (usePeaceDriver()) {
        PeaceDriver::keyboardClick(toPeaceKey(code), repeats);
        return;
    }
    nnN3A2qfth(Controller::m_msdk_handle, code, repeats);
}

void MSDK::KeyInputWord(char word)
{
    if (usePeaceDriver()) {
        bool shift = false;
        int key = asciiToHidKey(word, shift);
        if (key == 0)
            return;
        if (shift)
            PeaceDriver::keyboardDown(toPeaceKey(Keyboard_LeftShift));
        PeaceDriver::keyboardClick(toPeaceKey(key), 1);
        if (shift)
            PeaceDriver::keyboardUp(toPeaceKey(Keyboard_LeftShift));
        return;
    }
    pcmgzzXYA(Controller::m_msdk_handle,&word,1);
}

void MSDK::KeyInputUnicode(QString str)
{
    if (str.isEmpty())
        return;

    const std::wstring text = str.toStdWString();
    PeaceDriver::sendUnicodeStr(text.c_str());

}


// 随机延迟
void MSDK::DelayRandom(int min, int max)
{
    YYyWL2SSFlkB(min, max);
}

void MSDK::closeRunKey()
{
    if (usePeaceDriver()) {
        KeyUp2(Keyboard_UpArrow);
        KeyUp2(Keyboard_RightArrow);
        KeyUp2(Keyboard_DownArrow);
        KeyUp2(Keyboard_LeftArrow);
        return;
    }

    // 释放上
    if (KeyState(Keyboard_UpArrow) == 1) {
        KeyUp2(Keyboard_UpArrow);
        ProcessHelper::programDelay(50);
    }

    // 释放右
    if (KeyState(Keyboard_RightArrow) == 1) {
        KeyUp2(Keyboard_RightArrow);
        ProcessHelper::programDelay(50);
    }

    // 释放下
    if (KeyState(Keyboard_DownArrow) == 1) {
        KeyUp2(Keyboard_DownArrow);
        ProcessHelper::programDelay(50);
    }

    // 释放左
    if (KeyState(Keyboard_LeftArrow) == 1) {
        KeyUp2(Keyboard_LeftArrow);
        ProcessHelper::programDelay(50);
    }
}

void MSDK::setParam(int type,int param1,int param2)
{
    if (usePeaceDriver())
        return;
    whcg0cGgT(Controller::m_msdk_handle, type,param1,param2);
}
