#ifndef HEALTHMONITOR_H
#define HEALTHMONITOR_H

#include <qthread.h>
#include <QApplication>

class HealthMonitor : public QThread
{
    Q_OBJECT
public:
    HealthMonitor(QObject* parent);
    void run();
};

#endif // HEALTHMONITOR_H
