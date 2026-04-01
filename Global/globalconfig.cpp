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
