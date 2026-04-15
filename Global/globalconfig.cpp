#include "globalconfig.h"

#include <QDebug>
#include <QApplication>

GlobalConfig::GlobalConfig() {

    qDebug() << "初始化";
    // loadFromJson();
}

void GlobalConfig::loadFromJson()
{
    QString currentDir = QCoreApplication::applicationDirPath();
    QString config_path = currentDir + "/config.json";
    Config* config = Config::instance();
    config->setFilePath(config_path);

    // 配置文件不存在则使用默认配置
    QFile config_file(config_path);
    if (!config_file.exists()) {
        qDebug() << "配置文件不存在，采用默认配置";
        return;
    }

    // 加载配置
    config->load();
    QJsonObject healthConfig = data["auto_add_health"].toObject();
    healthConfig["add_hp"] = config->getValue("auto_add_health/add_hp", healthConfig["add_hp"].toVariant()).toBool();
    healthConfig["add_mp"] = config->getValue("auto_add_health/add_mp", healthConfig["add_mp"].toVariant()).toBool();
    healthConfig["hp_rate"] = config->getValue("auto_add_health/hp_rate", healthConfig["hp_rate"].toVariant()).toInt();
    healthConfig["mp_rate"] = config->getValue("auto_add_health/mp_rate", healthConfig["mp_rate"].toVariant()).toInt();
    healthConfig["hp_key"] = config->getValue("auto_add_health/hp_key", healthConfig["hp_key"].toVariant()).toInt();
    healthConfig["mp_key"] = config->getValue("auto_add_health/mp_key", healthConfig["mp_key"].toVariant()).toInt();
    data["auto_add_health"] = healthConfig;

    QJsonObject bindkeyConfig = data["bindkey"].toObject();
    bindkeyConfig["general_atk"] = config->getValue("bindkey/general_atk", bindkeyConfig["general_atk"].toVariant()).toInt();
    bindkeyConfig["skill_atk1"] = config->getValue("bindkey/skill_atk1", bindkeyConfig["skill_atk1"].toVariant()).toInt();
    bindkeyConfig["jump"] = config->getValue("bindkey/jump", bindkeyConfig["jump"].toVariant()).toInt();
    bindkeyConfig["gohome"] = config->getValue("bindkey/gohome", bindkeyConfig["gohome"].toVariant()).toInt();
    data["bindkey"] = bindkeyConfig;

    QJsonObject atkrangeConfig = data["atk_range"].toObject();
    atkrangeConfig["general_x"] = config->getValue("atk_range/general_x", atkrangeConfig["general_x"].toVariant()).toInt();
    atkrangeConfig["general_y"] = config->getValue("atk_range/general_y", atkrangeConfig["general_y"].toVariant()).toInt();
    atkrangeConfig["skill1_x"] = config->getValue("atk_range/skill1_x", atkrangeConfig["skill1_x"].toVariant()).toInt();
    atkrangeConfig["skill1_y"] = config->getValue("atk_range/skill1_y", atkrangeConfig["skill1_y"].toVariant()).toInt();
    data["atk_range"] = atkrangeConfig;
}

QVariant GlobalConfig::getValue(const QString &keyPath, const QVariant& defaultValue) const
{
    Config* config = Config::instance();
    config->m_jsonObject = data;
    return config->getValue(keyPath, defaultValue);
}

void GlobalConfig::setValue(const QString &keyPath, const QVariant& value)
{
    Config* config = Config::instance();
    config->m_jsonObject = data;

    QStringList keys = keyPath.split('/');

    QJsonValue res_value = config->getValueHelper(keys, config->m_jsonObject);
    if(res_value.isUndefined() || res_value.isNull()){
        qDebug() << "该配置项未定义" << keyPath;
        return;
    }

    config->setValue(keyPath, value);
    data = config->m_jsonObject;
    updateConfig();
}

void GlobalConfig::updateConfig()
{
    QString currentDir = QCoreApplication::applicationDirPath();
    QString config_path = currentDir + "/config.json";
    Config* config = Config::instance();
    config->setFilePath(config_path);
    config->m_jsonObject = data;
    config->save();
    qDebug() << "更新配置文件";
}
