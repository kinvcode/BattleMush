#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include "global/struct.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

class Controller : public QObject
{
    Q_OBJECT
private:
    Controller() = default;
public:
    static Controller* instance() {
        static Controller instance;
        return &instance;
    }

// ====== 成员属性 ======
public:
    /* HTTP开始 */
    static QString m_http_token;
    static qint64 m_http_token_timestamp;
    // 到期日期
    static QString m_vip_expired_date;
    // 到期时间戳
    static qint64 m_vip_expired_timestamp;
    /* HTTP结束 */

    static AttackInfo m_attack_info;

    static cv::Point m_attack_range_general;

    static cv::Point m_attack_range_skill1;

    // 刷图地图列表
    static std::vector<MapRoomPath> m_clean_room_list;

    // 休息地图列表
    static std::vector<MapRoomPath> m_rest_point_list;

    // NPC任务列表
    static std::vector<NPCQuest> m_quest_list;

    // 黑名单怪物列表
    static std::vector<QString> m_black_monster;

    // 计划列表
    static std::vector<Task> m_tasks;

    static bool m_disable_attack;

    static bool m_disable_recive;

    // 易键鼠句柄。未安装时键鼠走 PeaceDriver
    static HANDLE m_msdk_handle;
    static bool m_msdk_installed;
    static double m_system_gdi;

// ====== 便捷函数 ======
public:
    static void updateAttackInfo();

    static void initTaskList();

    static void initRoomNodes();

    static void saveTaskConfig();

    static void loadTaskConfig();

    static void reSortTask();
};

// 内联全局引用
inline Controller& Gcontroller = *Controller::instance();

#endif // CONTROLLER_H
