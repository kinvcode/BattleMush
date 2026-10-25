#include "nativeeventfilter.h"
#include <windows.h>
#include <iostream>
#include "utils/helper.h"
#include "global/globalplayer.h"
#include "utils/gamestruct.h"
#include "global/globalconfig.h"
#include "driver/msdk/msdkDriver.h"
#include "utils/data.h"

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

                // 更新游戏窗口
                Helper::updateGameWindow();
            }else{
                qDebug() << "自动模式已关闭~";
                Helper::console("自动模式已关闭~");

                // 清理状态
                Gplayer.need_hp_potion = false;
                Gplayer.need_mp_potion = false;

                emit sendControleStatus(false);

                Gplayer.mousecheck_exists = false;

                // 关闭警报
                if(main_win->m_alert){
                    Gplayer.show_warning = false;
                    main_win->m_alert->close();
                }

                MSDK::KeyUp(Keyboard_RightArrow);
                MSDK::KeyUp(Keyboard_LeftArrow);
                MSDK::KeyUp(Keyboard_UpArrow);
                MSDK::KeyUp(Keyboard_DownArrow);

                main_win->m_threadManager->pauseAll();
                m_is_stop = true;
            }
            break;
        case VK_F2:
            qDebug() << "F2";
            ImageTool::screenshot();
            break;
        case VK_F3:
            qDebug() << "F3";
            test();
            break;
        default:
            qDebug() << "未知快捷键触发:" + QString::number(keycode);
            break;
        }
    }
    return false;
}

void NativeEventFilter::test()
{
    qDebug() << "测试函数开始";

    qDebug() << "判断当前BUFF技能1绑定代码是否存在"; //SB2001002

    QString buff1_code = Gconfig.getValue("buff_bind/buff1").toString();

    if(buff1_code.isEmpty()){
        qDebug() << "BUFF技能1没有绑定技能";
        return;
    }

    qDebug() << "BUFF技能1绑定的技能是：" << buff1_code;

    // 遍历UI组件
    quint64 p_uientity = Data::Game::uiEntity();

    quint64 main_group = Helper::findChild(p_uientity, "MainGroup");
    quint64 buff_list = Helper::findChild(main_group,"BuffScroll");

    quint64 buff1_entity = Helper::findChild(buff_list, buff1_code);
    qDebug() << "buff1实体：" << QString::number(buff1_entity,16).toUpper();

    if(buff1_entity == 0){
        qDebug() << "BUFF技能1绑定的代码不存在";
        return;
    }

    qDebug() << "BUFF技能1绑定的代码已存在于BUFF栏中";

    qDebug() << "测试函数结束";
}



