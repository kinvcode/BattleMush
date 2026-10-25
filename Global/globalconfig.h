#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "utils/config.h"
// #include "driver/mskeycode.h"
#include "driver/msdk/usbhidkeycode.h"

class GlobalConfig : public QObject
{
    Q_OBJECT
public:
    QJsonObject data =
{
    {"atk_swtich",QJsonObject{
        {"general", true},
        {"skill1", true},
        {"buff1", true},
        {"buff2", false},
        {"buff3", false},
        {"buff4", false},
        {"buff5", false},
        {"skillteleport",false},
        }
    },
    {"auto_add_health",QJsonObject{
        {"add_hp", true},
        {"add_mp", true},
        {"hp_rate", 50},
        {"mp_rate", 50},
        {"hp_key", Keyboard_PageUp},
        {"mp_key", Keyboard_PageDown},
        {"potion_num_switch",true},
        {"potion_num",1}
        }
    },
    {"bindkey",QJsonObject{
        {"general_atk",Keyboard_x},
        {"skill_atk1",Keyboard_LeftControl},
        {"buff1",Keyboard_1},
        {"buff2",Keyboard_2},
        {"buff3",Keyboard_3},
        {"buff4",Keyboard_4},
        {"buff5",Keyboard_5},
        {"jump",Keyboard_c},
        {"gohome",Keyboard_Home},
        {"skillteleport",Keyboard_f}
        }
    },
    {"buff_bind",QJsonObject{
        {"buff1",""},
        {"buff2",""},
        {"buff3",""},
        {"buff4",""},
        {"buff5",""},
       }
    },
    {"atk_range",QJsonObject{
        {"general_x",50},
        {"general_y",20},
        {"skill1_x",150},
        {"skill1_y",40},
        }
    },
    {"atk_recast",QJsonObject{
        {"buff1",200},
        {"buff2",200},
        {"buff3",200},
        {"buff4",200},
        {"buff5",200},
        {"buff1_max",300},
        {"buff2_max",300},
        {"buff3_max",300},
        {"buff4_max",300},
        {"buff5_max",300},
        }
    },
    {"potion_buy",QJsonObject{
        {"hp_buy",true},
        {"mp_buy",true},
        {"hp_num",100},
        {"mp_num",100},
        {"hp_type",0},
        {"mp_type",0},
        }
    },
    {"pickup",QJsonObject{
        {"onlymyplat",false},
        {"forceatk",false},
        }
    },
    {"model","uplevel"}
};

    GlobalConfig();

    static GlobalConfig* instance() {
        static GlobalConfig instance;
        return &instance;
    }
public:
    void loadFromJson();
    QVariant getValue(const QString& keyPath, const QVariant& defaultValue = QVariant()) const; // 读取值
    void setValue(const QString& keyPath, const QVariant& value);  // 修改或新增值
public slots:
    void updateConfig();
};

// 内联全局引用
inline GlobalConfig& Gconfig = *GlobalConfig::instance();

#endif // GLOBALCONFIG_H
