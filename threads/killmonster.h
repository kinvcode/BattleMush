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
    std::unordered_map<quint64,int> m_black_item_pro;

    // 计划任务
    int m_task_id = 0; // 当前执行的任务ID
    int m_task_type = 0; // 当前执行的任务类型
    int m_task_excute_time = 0; // 当前任务执行的时间
    QElapsedTimer m_task_timer; // 当前任务计时器

    // 定时释放技能
    QElapsedTimer m_timer; // 当前线程计时器
    bool m_recast_buff1_enable = false;
    bool m_recast_buff2_enable = false;
    bool m_recast_buff3_enable = false;
    bool m_recast_buff4_enable = false;
    bool m_recast_buff5_enable = false;
    int m_recast_buff1_time;
    int m_recast_buff2_time;
    int m_recast_buff3_time;
    int m_recast_buff4_time;
    int m_recast_buff5_time;
    int m_recast_buff1_lasttime = 0;
    int m_recast_buff2_lasttime = 0;
    int m_recast_buff3_lasttime = 0;
    int m_recast_buff4_lasttime = 0;
    int m_recast_buff5_lasttime = 0;
    int m_recast_buff1_keycode;
    int m_recast_buff2_keycode;
    int m_recast_buff3_keycode;
    int m_recast_buff4_keycode;
    int m_recast_buff5_keycode;

    // 符文信息
    struct{
        int platform;
        cv::Point area;
    }m_rune_info;

// 角色信息
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

// 图内对象信息
private:
    // 寻找掉落物品
    Item findDropItem();

    // 更新全图怪物
    void updateAllMonsters();

    // 获取当前线路怪物
    std::vector<Monster> getLineMonsters(int user_line);

    // 是否正在释放技能
    bool isAttaking();

    // 是否存在符文
    bool isExistsRune();

// 计划任务
private:
    struct PlatformSequence{
        int index;
        std::vector<int> platform_set;
    };

    // 当前刷图路线
    PlatformSequence m_platform_sequence;

    // 当前计划信息
    TaskInfo m_task_info;

    // 更新计划任务ID
    void updateTaskID();

    // 更新任务信息到UI
    void sendTaskInfo(const TaskInfo &task);

// 具体操作
private:
    // 击杀怪物
    void killNearMonster(const std::vector<Monster> &handle_monsters);

    // 拾取物品
    void reciveItem(const Item &target_item);

    // 定时释放的技能。目前支持的有：个人BUFF
    void skillRecast();

    // 解除符文
    void solveRune();

    // 解除封印按键
    void runneKeyboard();

    // 自动清怪路线
    void autoCleanLine();

    // 执行平台路线
    void excutePlatformSequence();

    // 执行平台休息
    void excutePlatformRest(int rest_platform);

    // 执行NPC任务
    void excuteQuest();

    // 前往指定地图
    void moveToRoomByUUID(const QString &uuid);

    // 购买药水
    void buyPotion();

    // 药水商店购买
    void potionShopBuyItem(QString npc);

private:
    // 刷图计划
    void taskCleanRoom();

    // 休息计划
    void taskRest();

    // NPC任务计划
    void taskQuest();
};

#endif // KILLMONSTER_H

