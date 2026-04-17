#ifndef KILLMONSTER_H
#define KILLMONSTER_H

#include "threads/iworker.h"
#include <QThread>
#include "global/struct.h"
#include <unordered_set>

class KillMonster : public IWorker
{
    Q_OBJECT
public:
    explicit KillMonster(int id, QObject *parent = nullptr);
    virtual ~KillMonster();
public slots:
    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;

private:
    void process();
    bool m_running;
    bool m_paused;
    int m_counter;

    // 当前地图
    quint64 m_currentmap = 0;

    // 地图开始索引
    int m_map_start = 0;

    // 全图怪物
    std::vector<Monster> m_allmonster;

    // 全图物品
    std::vector<Item> m_allItem;

    // 黑名单物品（已经找过的物品，用于过滤掉其他玩家的物品）
    std::unordered_set<quint64> m_black_item;

    // 计划任务
    bool m_task_running = false;
    int m_task_id = 0; // 当前执行的任务ID
    int m_task_type = 0; // 当前执行的任务类型
    int m_task_excute_time = 0; // 当前任务执行的时间
    std::vector<int> m_task_room_path; //当前执行的路径
    QElapsedTimer m_task_timer; // 当前任务计时器

private:
    int userX();

    int userY();

    // 角色当前位置
    cv::Point userPos();

    // 角色当前地板
    quint64 userFoothold();

    // 角色当前攀爬物
    quint64 userClimb();

    // 角色当前平台
    quint64 userPlatform();

private:
    // 寻找掉落物品
    Item findDropItem();

    // 更新全图对象：0全部对象 1人物 2怪物 3物品

    // 更新全图怪物
    void updateAllMonsters();

    // 获取当前线路怪物
    std::vector<Monster> getLineMonsters(int user_line);

    // 击杀怪物
    void killNearMonster(const std::vector<Monster> &handle_monsters);

    // 拾取物品
    void reciveItem(const Item &target_item);


// 刷图线路
private:
    struct PlatformSequence{
        int index;
        std::vector<int> platform_set;
    };

    PlatformSequence m_platform_sequence;

    // 自动清怪路线
    void autoCleanLine();

    // 执行平台路线
    void excutePlatformSequence();

    // 更新计划任务ID
    void updateTaskID();

// signals:
    void sendTaskInfo(const TaskInfo &task);
};

#endif // KILLMONSTER_H

