#include "widgets/hotkeyedit.h"
#include <QKeySequence>
#include "global/globalconfig.h"
#include "driver/msdk/usbhidkeycode.h"
#include "Windows.h"

HotkeyEdit::HotkeyEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setPlaceholderText("点击后按快捷键");
    setAlignment(Qt::AlignCenter);

    // 设置样式
    setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #ccc;"
        "   border-radius: 3px;"
        "   padding: 5px;"
        "   background-color: white;"
        "}"
        "QLineEdit:focus {"
        "   border: 2px solid #4CAF50;"
        "   background-color: #f0f8ff;"
        "}"
        );
}

void HotkeyEdit::setHotkey(int key)
{
    if (m_currentKey != key) {
        m_currentKey = key;
        Gconfig.setValue(m_config_path, m_currentKey);
        updateDisplay();
    }
}

void HotkeyEdit::updateDisplay()
{
    if (m_currentKey != Qt::Key_unknown) {
        QString keystr = usbCodeToString(m_currentKey);
        setText(keystr);
        setPlaceholderText("");
    } else {
        qDebug() << "清理";
        clear();
    }
}

void HotkeyEdit::keyPressEvent(QKeyEvent *event)
{
    int sc = event->nativeScanCode();
    // int res = VK_HOME;
    // 去掉扩展键前缀

    qDebug() << "当前sc按键：" << sc;
    int usb_code = scanCodeToUSBCode(sc);
    setHotkey(usb_code);
}

// 扫描码 → 字符串（支持扩展键）
QString HotkeyEdit::scanCodeToString(int sc)
{
    // 小键盘和普通键
    switch (sc)
    {
    // 数字行
    case 0x02: return "1";
    case 0x03: return "2";
    case 0x04: return "3";
    case 0x05: return "4";
    case 0x06: return "5";
    case 0x07: return "6";
    case 0x08: return "7";
    case 0x09: return "8";
    case 0x0A: return "9";
    case 0x0B: return "0";

        // 字母
    case 0x1E: return "A"; case 0x30: return "B"; case 0x2E: return "C"; case 0x20: return "D";
    case 0x12: return "E"; case 0x21: return "F"; case 0x22: return "G"; case 0x23: return "H";
    case 0x17: return "I"; case 0x24: return "J"; case 0x25: return "K"; case 0x26: return "L";
    case 0x32: return "M"; case 0x31: return "N"; case 0x18: return "O"; case 0x19: return "P";
    case 0x10: return "Q"; case 0x13: return "R"; case 0x1F: return "S"; case 0x14: return "T";
    case 0x16: return "U"; case 0x2F: return "V"; case 0x11: return "W"; case 0x2D: return "X";
    case 0x15: return "Y"; case 0x2C: return "Z";

        // 功能键
    case 0x01: return "Esc"; case 0x3B: return "F1"; case 0x3C: return "F2"; case 0x3D: return "F3";
    case 0x3E: return "F4"; case 0x3F: return "F5"; case 0x40: return "F6"; case 0x41: return "F7";
    case 0x42: return "F8"; case 0x43: return "F9"; case 0x44: return "F10"; case 0x57: return "F11";
    case 0x58: return "F12";

        // 控制键
    case 0x2A: return "Left Shift"; case 0x36: return "Right Shift"; case 0x1D: return "Left Ctrl";
    case 0x38: return "Left Alt"; case 0x39: return "Space"; case 0x0F: return "Tab"; case 0x0E: return "Backspace";

        // 小键盘
    case 0x52: return "Num 0"; case 0x4F: return "Num 1"; case 0x50: return "Num 2"; case 0x51: return "Num 3";
    case 0x4B: return "Num 4"; case 0x4C: return "Num 5"; case 0x4D: return "Num 6"; case 0x47: return "Num 7";
    case 0x48: return "Num 8"; case 0x49: return "Num 9"; case 0x53: return "Num ."; case 0x4A: return "Num -";
    case 0x4E: return "Num +"; case 0x37: return "Num *"; case 0x35: return "Num /";

        // 扩展键 E0 前缀的扫描码
    case 0x149: return "PageUp";    // E0 49
    case 0x151: return "PageDown";  // E0 51
    case 0x147: return "Home";      // E0 47
    case 0x14F: return "End";       // E0 4F
    case 0x152: return "Insert";    // E0 52
    case 0x153: return "Delete";    // E0 53
    case 0x14B: return "Left";      // E0 4B
    case 0x14D: return "Right";     // E0 4D
    case 0x148: return "Up";        // E0 48
    case 0x150: return "Down";      // E0 50
    case 0x11D: return "Right Ctrl"; // E0 1D
    case 0x138: return "Right Alt";  // E0 38
    case 0x15B: return "Left Win";   // E0 5B
    case 0x15C: return "Right Win";  // E0 5C
    case 0x11C: return "Num Enter";  // E0 1C
    default:
        return QString("SC_%1").arg(sc, 3, 16, QChar('0')).toUpper();
    }
}

int HotkeyEdit::scanCodeToUSBCode(int sc)
{
    // 检查是否是扩展键 (扫描码 & 0x100 表示 E0 前缀)
    bool isExtended = (sc & 0x100) != 0;
    int baseCode = sc & 0xFF;  // 去掉高位

    // 如果是扩展键，使用扩展键映射
    if (isExtended)
    {
        switch (baseCode)
        {
        case 0x49: return Keyboard_PageUp;      // E0 49
        case 0x51: return Keyboard_PageDown;    // E0 51
        case 0x47: return Keyboard_Home;        // E0 47
        case 0x4F: return Keyboard_End;         // E0 4F
        case 0x52: return Keyboard_Insert;      // E0 52
        case 0x53: return Keyboard_Delete;      // E0 53
        case 0x4B: return Keyboard_LeftArrow;   // E0 4B
        case 0x4D: return Keyboard_RightArrow;  // E0 4D
        case 0x48: return Keyboard_UpArrow;     // E0 48
        case 0x50: return Keyboard_DownArrow;   // E0 50
        case 0x5B: return Keyboard_LeftWindows; // E0 5B
        case 0x5C: return Keyboard_RightWindows;// E0 5C
        case 0x5D: return Keyboard_Application; // E0 5D
        case 0x1D: return Keyboard_RightControl;// E0 1D
        case 0x38: return Keyboard_RightAlt;    // E0 38
        case 0x1C: return Keypad_ENTER;         // E0 1C (Num Enter)
        case 0x35: return Keyboard_XieGang_WenHao; // E0 35 (Keypad /)
        case 0x37: return Keyboard_PrintScreen; // E0 37 (Print Screen - 实际上需要组合键)
        default:
            return -1;
        }
    }

    // 字母键映射 (扫描码 -> USB HID)
    switch (sc)
    {
    case 0x1E: return Keyboard_a;      // A
    case 0x30: return Keyboard_b;      // B
    case 0x2E: return Keyboard_c;      // C
    case 0x20: return Keyboard_d;      // D
    case 0x12: return Keyboard_e;      // E
    case 0x21: return Keyboard_f;      // F
    case 0x22: return Keyboard_g;      // G
    case 0x23: return Keyboard_h;      // H
    case 0x17: return Keyboard_i;      // I
    case 0x24: return Keyboard_j;      // J
    case 0x25: return Keyboard_k;      // K
    case 0x26: return Keyboard_l;      // L
    case 0x32: return Keyboard_m;      // M
    case 0x31: return Keyboard_n;      // N
    case 0x18: return Keyboard_o;      // O
    case 0x19: return Keyboard_p;      // P
    case 0x10: return Keyboard_q;      // Q
    case 0x13: return Keyboard_r;      // R
    case 0x1F: return Keyboard_s;      // S
    case 0x14: return Keyboard_t;      // T
    case 0x16: return Keyboard_u;      // U
    case 0x2F: return Keyboard_v;      // V
    case 0x11: return Keyboard_w;      // W
    case 0x2D: return Keyboard_x;      // X
    case 0x15: return Keyboard_y;      // Y
    case 0x2C: return Keyboard_z;      // Z

        // 数字行
    case 0x02: return Keyboard_1;      // 1
    case 0x03: return Keyboard_2;      // 2
    case 0x04: return Keyboard_3;      // 3
    case 0x05: return Keyboard_4;      // 4
    case 0x06: return Keyboard_5;      // 5
    case 0x07: return Keyboard_6;      // 6
    case 0x08: return Keyboard_7;      // 7
    case 0x09: return Keyboard_8;      // 8
    case 0x0A: return Keyboard_9;      // 9
    case 0x0B: return Keyboard_0;      // 0

        // 功能键
    case 0x3B: return Keyboard_F1;
    case 0x3C: return Keyboard_F2;
    case 0x3D: return Keyboard_F3;
    case 0x3E: return Keyboard_F4;
    case 0x3F: return Keyboard_F5;
    case 0x40: return Keyboard_F6;
    case 0x41: return Keyboard_F7;
    case 0x42: return Keyboard_F8;
    case 0x43: return Keyboard_F9;
    case 0x44: return Keyboard_F10;
    case 0x57: return Keyboard_F11;
    case 0x58: return Keyboard_F12;

        // 控制键
    case 0x01: return Keyboard_ESCAPE;          // Esc
    case 0x0E: return Keyboard_Backspace;       // Backspace
    case 0x0F: return Keyboard_Tab;             // Tab
    case 0x39: return Keyboard_KongGe;          // Space
    case 0x1C: return Keyboard_ENTER;           // Enter
    case 0x2A: return Keyboard_LeftShift;       // Left Shift
    case 0x36: return Keyboard_RightShift;      // Right Shift
    case 0x1D: return Keyboard_LeftControl;     // Left Ctrl
    case 0x9D: return Keyboard_RightControl;    // Right Ctrl (E0 1D)
    case 0x38: return Keyboard_LeftAlt;         // Left Alt
    case 0xB8: return Keyboard_RightAlt;        // Right Alt (E0 38)
    case 0x3A: return Keyboard_CapsLock;        // Caps Lock

        // 符号键
    case 0x0C: return Keyboard_JianHao;         // -
    case 0x0D: return Keyboard_DengHao;         // =
    case 0x1A: return Keyboard_ZuoZhongKuoHao;  // [
    case 0x1B: return Keyboard_YouZhongKuoHao;  // ]
    case 0x2B: return Keyboard_FanXieGang;      // "\"
    case 0x27: return Keyboard_FenHao;          // ;
    case 0x28: return Keyboard_DanYinHao;       // '
    case 0x29: return Keyboard_BoLangXian;      // `
    case 0x33: return Keyboard_Douhao;          // ,
    case 0x34: return Keyboard_JuHao;           // "."
    // case 0x35: return Keyboard_XieGang_WenHao;  // "/"

        // 小键盘（非扩展）
    case 0x52: return Keypad_0_and_Insert;      // Num 0
    case 0x4F: return Keypad_1_and_End;         // Num 1
    case 0x50: return Keypad_2_and_DownArrow;   // Num 2
    case 0x51: return Keypad_3_and_PageDn;      // Num 3
    case 0x4B: return Keypad_4_and_LeftArrow;   // Num 4
    case 0x4C: return Keypad_5;                 // Num 5
    case 0x4D: return Keypad_6_and_RightArrow;  // Num 6
    case 0x47: return Keypad_7_and_Home;        // Num 7
    case 0x48: return Keypad_8_and_UpArrow;     // Num 8
    case 0x49: return Keypad_9_and_PageUp;      // Num 9
    case 0x53: return Keypad_Dian_and_Delete;   // Num .
    case 0x4A: return Keypad_JianHao;           // Num -
    case 0x4E: return Keypad_JiaHao;            // Num +
    case 0x37: return Keypad_ChengHao;          // Num *
    case 0x35: return Keypad_ChuHao;            // Num /
    case 0x9C: return Keypad_ENTER;             // Num Enter (E0 1C)

        // 扩展键 (E0 前缀)
    case 0xE049: return Keyboard_PageUp;        // E0 49
    case 0xE051: return Keyboard_PageDown;      // E0 51
    case 0xE047: return Keyboard_Home;          // E0 47
    case 0xE04F: return Keyboard_End;           // E0 4F
    case 0xE052: return Keyboard_Insert;        // E0 52
    case 0xE053: return Keyboard_Delete;        // E0 53
    case 0xE04B: return Keyboard_LeftArrow;     // E0 4B
    case 0xE04D: return Keyboard_RightArrow;    // E0 4D
    case 0xE048: return Keyboard_UpArrow;       // E0 48
    case 0xE050: return Keyboard_DownArrow;     // E0 50
    case 0xE05B: return Keyboard_LeftWindows;   // E0 5B
    case 0xE05C: return Keyboard_RightWindows;  // E0 5C
    case 0xE05D: return Keyboard_Application;   // E0 5D
    case 0xE01D: return Keyboard_RightControl;  // E0 1D
    case 0xE038: return Keyboard_RightAlt;      // E0 38
    case 0xE01C: return Keypad_ENTER;           // E0 1C (Num Enter)
    case 0xE035: return Keyboard_XieGang_WenHao;// E0 35 (Keypad /)

        // 其他键
    case 0x46: return Keyboard_ScrollLock;      // Scroll Lock
    case 0x45: return Keyboard_Pause;           // Pause
    case 0xB7: return Keyboard_PrintScreen;     // Print Screen (E0 2A E0 37)

    default:
        return -1;  // 未映射
    }
}

QString HotkeyEdit::usbCodeToString(int usbCode)
{
    // 字母键 - 返回大写
    if (usbCode >= Keyboard_a && usbCode <= Keyboard_z)
    {
        char ch = 'A' + (usbCode - Keyboard_a);
        return QString(ch);
    }

    // 数字行
    switch (usbCode)
    {
    case Keyboard_1: return "1";
    case Keyboard_2: return "2";
    case Keyboard_3: return "3";
    case Keyboard_4: return "4";
    case Keyboard_5: return "5";
    case Keyboard_6: return "6";
    case Keyboard_7: return "7";
    case Keyboard_8: return "8";
    case Keyboard_9: return "9";
    case Keyboard_0: return "0";
    }

    // 符号键（无 Shift 版本）
    switch (usbCode)
    {
    case Keyboard_JianHao: return "-";
    case Keyboard_DengHao: return "=";
    case Keyboard_ZuoZhongKuoHao: return "[";
    case Keyboard_YouZhongKuoHao: return "]";
    case Keyboard_FanXieGang: return "\\";
    case Keyboard_FenHao: return ";";
    case Keyboard_DanYinHao: return "'";
    case Keyboard_BoLangXian: return "`";
    case Keyboard_Douhao: return ",";
    case Keyboard_JuHao: return ".";
    case Keyboard_XieGang_WenHao: return "/";
    }

    // 特殊键
    switch (usbCode)
    {
    case Keyboard_KongGe: return "Space";
    case Keyboard_ENTER: return "Enter";
    case Keyboard_Tab: return "Tab";
    case Keyboard_Backspace: return "Backspace";
    case Keyboard_ESCAPE: return "Esc";
    case Keyboard_CapsLock: return "Caps Lock";
    }

    // 功能键
    switch (usbCode)
    {
    case Keyboard_F1: return "F1";
    case Keyboard_F2: return "F2";
    case Keyboard_F3: return "F3";
    case Keyboard_F4: return "F4";
    case Keyboard_F5: return "F5";
    case Keyboard_F6: return "F6";
    case Keyboard_F7: return "F7";
    case Keyboard_F8: return "F8";
    case Keyboard_F9: return "F9";
    case Keyboard_F10: return "F10";
    case Keyboard_F11: return "F11";
    case Keyboard_F12: return "F12";
    }

    // 控制键
    switch (usbCode)
    {
    case Keyboard_LeftShift: return "Left Shift";
    case Keyboard_RightShift: return "Right Shift";
    case Keyboard_LeftControl: return "Left Ctrl";
    case Keyboard_RightControl: return "Right Ctrl";
    case Keyboard_LeftAlt: return "Left Alt";
    case Keyboard_RightAlt: return "Right Alt";
    case Keyboard_LeftWindows: return "Left Win";
    case Keyboard_RightWindows: return "Right Win";
    case Keyboard_Application: return "Application";
    }

    // 导航键
    switch (usbCode)
    {
    case Keyboard_PrintScreen: return "Print Screen";
    case Keyboard_ScrollLock: return "Scroll Lock";
    case Keyboard_Pause: return "Pause";
    case Keyboard_Insert: return "Insert";
    case Keyboard_Home: return "Home";
    case Keyboard_PageUp: return "PageUp";
    case Keyboard_Delete: return "Delete";
    case Keyboard_End: return "End";
    case Keyboard_PageDown: return "PageDown";
    case Keyboard_RightArrow: return "Right";
    case Keyboard_LeftArrow: return "Left";
    case Keyboard_DownArrow: return "Down";
    case Keyboard_UpArrow: return "Up";
    }

    // 小键盘
    switch (usbCode)
    {
    case Keypad_NumLock: return "Num Lock";
    case Keypad_ChuHao: return "Num /";
    case Keypad_ChengHao: return "Num *";
    case Keypad_JianHao: return "Num -";
    case Keypad_JiaHao: return "Num +";
    case Keypad_ENTER: return "Num Enter";
    case Keypad_1_and_End: return "Num 1";
    case Keypad_2_and_DownArrow: return "Num 2";
    case Keypad_3_and_PageDn: return "Num 3";
    case Keypad_4_and_LeftArrow: return "Num 4";
    case Keypad_5: return "Num 5";
    case Keypad_6_and_RightArrow: return "Num 6";
    case Keypad_7_and_Home: return "Num 7";
    case Keypad_8_and_UpArrow: return "Num 8";
    case Keypad_9_and_PageUp: return "Num 9";
    case Keypad_0_and_Insert: return "Num 0";
    case Keypad_Dian_and_Delete: return "Num .";
    }

    // 未知键
    return QString("USB_%1").arg(usbCode);
}

