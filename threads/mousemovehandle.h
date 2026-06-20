#ifndef MOUSEMOVEHANDLE_H
#define MOUSEMOVEHANDLE_H

#include <qthread.h>
#include <QApplication>
#include <QObject>
#include "threads/iworker.h"

class MouseMoveHandle : public IWorker
{
    Q_OBJECT
public:
    explicit MouseMoveHandle(int id, QObject *parent = nullptr);
    virtual ~MouseMoveHandle();

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

    // 检测滑鼠
    void checkMouseMove();

    bool UIAntiMacroExists(quint64 list, quint64 head);

    void logMouseCheck();

    // 打印滑鼠信息
    void printMouseInfo();

    void printPlanInfo(quint64 plan_p);
};

#endif // MOUSEMOVEHANDLE_H
