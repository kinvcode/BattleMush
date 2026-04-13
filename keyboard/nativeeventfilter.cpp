#include "nativeeventfilter.h"
#include <windows.h>
#include <iostream>
#include "utils/helper.h"

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
            switch(keycode){
        case VK_F1:
            qDebug() << "F1";
            if(m_is_stop){
                qDebug() << "恢复线程";
                Helper::console("已启动自动模式~");
                emit sendControleStatus(true);
                main_win->m_threadManager->resumeAll();
                m_is_stop = false;
            }else{
                qDebug() << "自动模式已关闭~";
                Helper::console("自动模式已关闭~");
                emit sendControleStatus(false);
                main_win->m_threadManager->pauseAll();
                m_is_stop = true;
            }
            break;
        case VK_F2:
            qDebug() << "F2";
                // main_win->m_threadManager->stopAll();
            break;
        default:
            qDebug() << "未知快捷键触发:" + QString::number(keycode);
            break;
        }
    }
    return false;
}



