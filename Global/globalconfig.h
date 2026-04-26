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
        {"buff1", true}
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
        {"buff1",MSKeyboard_LeftShift},
        {"jump",MSKeyboard_c},
        {"gohome",MSKeyboard_LeftControl}
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
        {"buff1",60}
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
