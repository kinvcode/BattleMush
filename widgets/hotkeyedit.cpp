#include "widgets/hotkeyedit.h"
#include <QKeySequence>
#include "global/globalconfig.h"
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
        QString keystr = scanCodeToString(m_currentKey);
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
    setHotkey(sc);
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

