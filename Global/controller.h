#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include "global/struct.h"

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
    static AttackInfo m_attack_info;

    static cv::Point m_attack_range_general;

    static cv::Point m_attack_range_skill1;

    // 刷图地图列表
    static std::vector<MapRoomPath> m_clean_room_list;

    // 休息地图列表
    static std::vector<MapRoomPath> m_rest_point_list;

    // NPC任务列表
    static std::vector<NPCQuest> m_quest_list;

    // 计划列表
    static std::vector<Task> m_tasks;

    static bool m_disable_attack;

    static bool m_disable_recive;

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
