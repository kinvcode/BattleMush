#ifndef MONITORTOOLTEST_H
#define MONITORTOOLTEST_H

#include <qthread.h>
#include <QApplication>
#include "threads/iworker.h"

class MonitorToolTest : public IWorker
{
    Q_OBJECT
public:
    explicit MonitorToolTest(int id, QObject *parent = nullptr);
    virtual ~MonitorToolTest();

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

#endif // MONITORTOOLTEST_H
