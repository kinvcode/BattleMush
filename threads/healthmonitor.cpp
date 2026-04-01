#include "healthmonitor.h"
#include "process/process.h"
#include "global/globalconfig.h"
#include "global/globalplayer.h"
#include "driver/msdriverpro.h"

HealthMonitor::HealthMonitor(QObject* parent)
{

}

void HealthMonitor::run()
{
    while(true){

        qDebug() << "当前血量：" << Gplayer.HP;
        qDebug() << "当前蓝量：" << Gplayer.MP;

        // 检测血量
        int hp_rate = Gconfig.getValue("auto_add_health/hp_rate").toInt();
        if(Gplayer.MAX_HP != 0 && Gplayer.HP < Gplayer.MAX_HP){
            int cur_hp_rate = ( Gplayer.HP / Gplayer.MAX_HP) * 100;
            if(cur_hp_rate <= hp_rate){
                qDebug() << "检测到血量低于安全值，自动加血";
                int hp_key_code = Gconfig.getValue("auto_add_health/hp_key").toInt();
                qDebug() << "驱动按键码:" << hp_key_code;

                // MSDriverPRO::keyboardClick(0x49);
                MSDriverPRO::keyboardClick(hp_key_code, 1);
            }
        }

        int mp_rate = Gconfig.getValue("auto_add_health/mp_rate").toInt();
        if(Gplayer.MAX_MP != 0 && Gplayer.MP < Gplayer.MAX_MP){
            int cur_mp_rate = ( Gplayer.MP / Gplayer.MAX_MP) * 100;
            if(cur_mp_rate <= mp_rate){
                qDebug() << "检测到蓝量低于安全值，自动加蓝";
                int mp_key_code = Gconfig.getValue("auto_add_health/mp_key").toInt();
                qDebug() << "驱动按键码:" << mp_key_code;

                // MSDriverPRO::keyboardClick(0x49);
                MSDriverPRO::keyboardClick(mp_key_code, 1);
            }
        }

        ProcessHelper::programDelay(500);
        QApplication::processEvents();
    }
}
