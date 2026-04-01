#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "utils/config.h"

class GlobalConfig : public QObject
{
    Q_OBJECT
public:
    QJsonObject data =
{
    {"auto_add_health",QJsonObject{
        {"add_hp",true},
        {"add_mp",true},
        {"hp_rate",50},
        {"mp_rate",50},
        {"hp_key","1"},
        {"mp_key","2"},
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
