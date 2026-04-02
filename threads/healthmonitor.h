#ifndef HEALTHMONITOR_H
#define HEALTHMONITOR_H

#include <qthread.h>
#include <QApplication>
#include "threads/iworker.h"

class HealthMonitor : public IWorker
{
    Q_OBJECT
public:
    explicit HealthMonitor(int id, QObject *parent = nullptr);
    virtual ~HealthMonitor();
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
};

#endif // HEALTHMONITOR_H
