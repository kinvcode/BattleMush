#ifndef KILLMONSTER_H
#define KILLMONSTER_H

#include "threads/iworker.h"
#include <QThread>
#include "global/struct.h"

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

    // 按键绑定
    int m_bindkey_general_atk;
    int m_bindkey_skill_atk1;
    int m_bindkey_jump;
    int m_bindkey_gohome;

    // 当前地图
    quint64 m_currentmap = 0;

    // 地图开始索引
    int m_map_start;

    // 全图怪物
    std::vector<Monster> m_allmonster;

    // 全图物品
    // std::vector<Item> m_allItem;

private:
    // 寻找掉落物品
    void findDropItem(int start_index);

    // 更新全图对象：0全部对象 1人物 2怪物 3物品

    // 更新全图怪物
    void updateAllMonsters();

    // 获取当前线路怪物
    std::vector<Monster> getLineMonsters(int user_line);

    // 击杀怪物

    void killNearMonster(std::vector<Monster> handle_monsters);

















};

#endif // KILLMONSTER_H

