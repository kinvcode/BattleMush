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

    // 攻击范围
    cv::Point m_attack_range = {150, 40};

    // 地图开始索引
    int m_map_start = 0;

    // 全图怪物
    std::vector<Monster> m_allmonster;

    // 全图物品
    std::vector<Item> m_allItem;

    // 黑名单物品（已经找过的物品，用于过滤掉其他玩家的物品）
    std::unordered_set<quint64> m_black_item;

private:
    int userX();

    int userY();

    cv::Point userPos();

    quint64 userFoothold();

    quint64 userClimb();

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

    void reciveItem(const Item &target_item);














};

#endif // KILLMONSTER_H

