#ifndef UPDATEGAMEDATA_H
#define UPDATEGAMEDATA_H
#include "threads/iworker.h"

#include <QThread>

class UpdateGameData : public IWorker
{
    Q_OBJECT
public:
    explicit UpdateGameData(int id, QObject *parent = nullptr);
    virtual ~UpdateGameData();
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

private:
    // 处理登录
    void HandleLogin();
    // 更新角色信息
    void UpdateUserStatus(quint64 statuspanel);
    // 更新房间地图
    void UpdateRoomMap();
    // 更新怪物
    void UpdateMonsters();
    // 更新快捷栏信息
    void updateQuickSlot(quint64 ui_entity);
};

#endif // UPDATEGAMEDATA_H
