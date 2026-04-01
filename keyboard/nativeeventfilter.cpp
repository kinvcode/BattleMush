#include "nativeeventfilter.h"
#include <windows.h>
#include <iostream>

NativeEventFilter::NativeEventFilter(MainWindow* window)
{
    main_win = window;
}

bool NativeEventFilter::nativeEventFilter(const QByteArray& eventType, void* message, long long* result)
{
    MSG* msg = static_cast<MSG*>(message);
    if (msg->message == WM_HOTKEY)
    {
        const quint32 keycode = HIWORD(msg->lParam);
        qDebug() << "当前按键：" + QString::number(keycode);
        sendMsg("当前按键:" + QString::number(keycode));
            switch(keycode){
        case VK_HOME:
                qDebug() << "HOME";
            break;
        case VK_END:
                qDebug() << "END";
            break;
        default:
            sendMsg("未知快捷键触发:" + QString::number(keycode));
            break;
        }
    }
    return false;
}



