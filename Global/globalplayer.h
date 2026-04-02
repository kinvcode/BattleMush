#ifndef GLOBALPLAYER_H
#define GLOBALPLAYER_H
#include <QObject>

class GlobalPlayer : public QObject
{
    Q_OBJECT
private:
    GlobalPlayer() = default;
public:
    static GlobalPlayer* instance() {
        static GlobalPlayer instance;
        return &instance;
    }

    // 游戏进程PID
    quint64 p_gamepid = 0;

    // assembly.dll基址
    quint64 p_assemblybase = 0;

    // 游戏入口指针
    quint64 p_entry = 0;

    // 游戏上线文
    quint64 p_gamecontext = 0;

    // UI->LocalUserGroup->StatusPanel
    quint64 p_statuspanel = 0;

    // 角色昵称
    quint64 p_nickname = 0;

    // 角色职业
    quint64 p_user_job = 0;

    // 角色等级
    quint64 p_user_level = 0;

    // 角色血量
    quint64 p_user_hp = 0;

    // 角色蓝量
    quint64 p_user_mp = 0;

    // 属性
    int HP = 999999;
    int MAX_HP = 999999;

    int MP = 999999;
    int MAX_MP = 999999;

private:

};

// 内联全局引用
inline GlobalPlayer& Gplayer = *GlobalPlayer::instance();

#endif // GLOBALPLAYER_H
