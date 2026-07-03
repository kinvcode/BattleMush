#ifndef GLOBALPLAYER_H
#define GLOBALPLAYER_H
#include <QObject>
#include "global/struct.h"
#include <windows.h>

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

    // 键鼠驱动句柄
    HANDLE MSDK_HANDLE = NULL;

    // 警报
    bool show_warning = false;

    // 出现滑鼠验证
    bool mousecheck_exists = false;

    // 法师使用瞬移
    bool use_skillteleport = false;

    // 上次使用瞬移时间
    quint64 last_useskillteleport = 0;

    // 游戏进程PID
    quint64 p_gamepid = 0;

    // assembly.dll基址
    quint64 p_assemblybase = 0;

    // 游戏入口
    quint64 p_gamecontext_entries = 0;

    // 游戏上线文
    quint64 p_gamecontext = 0;

    // 地图
    quint64 p_mapsentity = 0;

    quint64 p_curmapentity = 0;

    // 最后一次所在地图
    quint64 last_mapentity = 0;

    // 当前地图信息
    MapInfo curmapinfo;

    // 地图平台坐标映射
    std::vector<std::vector<PlatCoor>> plat_map;

    // 地图节点列表
    std::unordered_map<QString, RoomNode> room_nodes;

    // 地图是否加载完毕
    bool m_map_loaded = false;

    // UI
    quint64 p_uientity = 0;

    // allServices
    quint64 p_allServices = 0;

    // 用户服务
    quint64 p_userservice = 0;

    // 碰撞服务
    quint64 p_collisionservice = 0;

    quint64 p_localplayer = 0;

    quint64 p_statuspanel = 0;

    // 角色昵称
    QString p_nickname = 0;

    // 角色职业
    QString p_user_job = 0;

    // 角色等级
    quint64 p_user_level = 0;

    // 角色血量
    quint64 p_user_hp = 0;

    // 角色蓝量
    quint64 p_user_mp = 0;

    // 角色是否死亡
    bool DIED = false;

    // 属性
    int HP = 0;
    int MAX_HP = 0;

    int MP = 0;
    int MAX_MP = 0;

    // 快捷键物品数量
    int quick_slot_pageup_count = 999999;
    int quick_slot_pagedown_count = 999999;

    // 用户绑定的按键
    BindKey m_bindkey; // 建议放到controller里面

    std::unordered_map<quint64,int> m_black_item_pro;

    // 一些测试数据
    std::vector<std::pair<float, float>> move_pos_list;

    // 相机属性
    float camera_width = 1296.0f;
    float camera_height = 759.0f;

    // 药水数量
    bool need_hp_potion = false;
    bool need_mp_potion = false;

    GameWindow game_window = {0,0,1920,1080};
private:

};

// 内联全局引用
inline GlobalPlayer& Gplayer = *GlobalPlayer::instance();

#endif // GLOBALPLAYER_H
