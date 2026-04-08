#ifndef KILLMONSTER_H
#define KILLMONSTER_H

#include "threads/iworker.h"
#include <QThread>

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

    int m_bindkey_general_atk;
    int m_bindkey_skill_atk1;
    int m_bindkey_jump;
    int m_bindkey_gohome;

    void findDropItem(int start_index);
};

#endif // KILLMONSTER_H
