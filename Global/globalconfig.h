#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "utils/config.h"
#include "driver/mskeycode.h"

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
        {"hp_key", MSKeyboard_1},
        {"mp_key", MSKeyboard_2},
        {"potion_num_switch",true},
        {"potion_num",10}
        }
    },
    {"bindkey",QJsonObject{
        {"general_atk",MSKeyboard_x},
        {"skill_atk1",MSKeyboard_LeftControl},
        {"buff1",MSKeyboard_1},
        {"buff2",MSKeyboard_2},
        {"buff3",MSKeyboard_3},
        {"buff4",MSKeyboard_4},
        {"buff5",MSKeyboard_5},
        {"jump",MSKeyboard_c},
        {"gohome",MSKeyboard_LeftControl},
        {"skillteleport",MSKeyboard_f}
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
        {"buff1",60},
        {"buff2",60},
        {"buff3",60},
        {"buff4",60},
        {"buff5",60},
        {"buff1_max",60},
        {"buff2_max",60},
        {"buff3_max",60},
        {"buff4_max",60},
        {"buff5_max",60},
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
