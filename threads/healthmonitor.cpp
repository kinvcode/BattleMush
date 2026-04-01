#include "healthmonitor.h"
#include "process/process.h"
#include "global/globalconfig.h"
#include "global/globalplayer.h"

HealthMonitor::HealthMonitor(QObject* parent)
{

}

void HealthMonitor::run()
{
    while(true){

        qDebug() << "当前血量：" << Gplayer.HP;

        ProcessHelper::programDelay(200);
        QApplication::processEvents();
    }
}
