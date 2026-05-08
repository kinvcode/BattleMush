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

    // 技能开关
    QJsonObject atkSwitchConfig = data["atk_swtich"].toObject();
    atkSwitchConfig["general"] = config->getValue("atk_swtich/general", atkSwitchConfig["general"].toVariant()).toBool();
    atkSwitchConfig["skill1"] = config->getValue("atk_swtich/skill1", atkSwitchConfig["skill1"].toVariant()).toBool();
    atkSwitchConfig["buff1"] = config->getValue("atk_swtich/buff1", atkSwitchConfig["buff1"].toVariant()).toBool();
    atkSwitchConfig["buff2"] = config->getValue("atk_swtich/buff2", atkSwitchConfig["buff2"].toVariant()).toBool();
    atkSwitchConfig["buff3"] = config->getValue("atk_swtich/buff3", atkSwitchConfig["buff3"].toVariant()).toBool();
    atkSwitchConfig["buff4"] = config->getValue("atk_swtich/buff4", atkSwitchConfig["buff4"].toVariant()).toBool();
    atkSwitchConfig["buff5"] = config->getValue("atk_swtich/buff5", atkSwitchConfig["buff5"].toVariant()).toBool();
    data["atk_swtich"] = atkSwitchConfig;

    // 监控血蓝
    QJsonObject healthConfig = data["auto_add_health"].toObject();
    healthConfig["add_hp"] = config->getValue("auto_add_health/add_hp", healthConfig["add_hp"].toVariant()).toBool();
    healthConfig["add_mp"] = config->getValue("auto_add_health/add_mp", healthConfig["add_mp"].toVariant()).toBool();
    healthConfig["hp_rate"] = config->getValue("auto_add_health/hp_rate", healthConfig["hp_rate"].toVariant()).toInt();
    healthConfig["mp_rate"] = config->getValue("auto_add_health/mp_rate", healthConfig["mp_rate"].toVariant()).toInt();
    healthConfig["hp_key"] = config->getValue("auto_add_health/hp_key", healthConfig["hp_key"].toVariant()).toInt();
    healthConfig["mp_key"] = config->getValue("auto_add_health/mp_key", healthConfig["mp_key"].toVariant()).toInt();
    healthConfig["potion_num_switch"] = config->getValue("auto_add_health/potion_num_switch", healthConfig["potion_num_switch"].toVariant()).toBool();
    healthConfig["potion_num"] = config->getValue("auto_add_health/potion_num", healthConfig["potion_num"].toVariant()).toInt();
    data["auto_add_health"] = healthConfig;

    // 血蓝购买
    QJsonObject potionBuyConfig = data["potion_buy"].toObject();
    potionBuyConfig["hp_buy"] = config->getValue("potion_buy/hp_buy", potionBuyConfig["hp_buy"].toVariant()).toBool();
    potionBuyConfig["hp_num"] = config->getValue("potion_buy/hp_num", potionBuyConfig["hp_num"].toVariant()).toInt();
    potionBuyConfig["hp_type"] = config->getValue("potion_buy/hp_type", potionBuyConfig["hp_type"].toVariant()).toInt();
    potionBuyConfig["mp_buy"] = config->getValue("potion_buy/mp_buy", potionBuyConfig["mp_buy"].toVariant()).toBool();
    potionBuyConfig["mp_num"] = config->getValue("potion_buy/mp_num", potionBuyConfig["mp_num"].toVariant()).toInt();
    potionBuyConfig["mp_type"] = config->getValue("potion_buy/mp_type", potionBuyConfig["mp_type"].toVariant()).toInt();
    data["potion_buy"] = potionBuyConfig;

    // 快捷键绑定
    QJsonObject bindkeyConfig = data["bindkey"].toObject();
    bindkeyConfig["general_atk"] = config->getValue("bindkey/general_atk", bindkeyConfig["general_atk"].toVariant()).toInt();
    bindkeyConfig["skill_atk1"] = config->getValue("bindkey/skill_atk1", bindkeyConfig["skill_atk1"].toVariant()).toInt();
    bindkeyConfig["buff1"] = config->getValue("bindkey/buff1", bindkeyConfig["buff1"].toVariant()).toInt();
    bindkeyConfig["buff2"] = config->getValue("bindkey/buff2", bindkeyConfig["buff2"].toVariant()).toInt();
    bindkeyConfig["buff3"] = config->getValue("bindkey/buff3", bindkeyConfig["buff3"].toVariant()).toInt();
    bindkeyConfig["buff4"] = config->getValue("bindkey/buff4", bindkeyConfig["buff4"].toVariant()).toInt();
    bindkeyConfig["buff5"] = config->getValue("bindkey/buff5", bindkeyConfig["buff5"].toVariant()).toInt();
    bindkeyConfig["jump"] = config->getValue("bindkey/jump", bindkeyConfig["jump"].toVariant()).toInt();
    bindkeyConfig["gohome"] = config->getValue("bindkey/gohome", bindkeyConfig["gohome"].toVariant()).toInt();
    data["bindkey"] = bindkeyConfig;

    // 技能范围
    QJsonObject atkrangeConfig = data["atk_range"].toObject();
    atkrangeConfig["general_x"] = config->getValue("atk_range/general_x", atkrangeConfig["general_x"].toVariant()).toInt();
    atkrangeConfig["general_y"] = config->getValue("atk_range/general_y", atkrangeConfig["general_y"].toVariant()).toInt();
    atkrangeConfig["skill1_x"] = config->getValue("atk_range/skill1_x", atkrangeConfig["skill1_x"].toVariant()).toInt();
    atkrangeConfig["skill1_y"] = config->getValue("atk_range/skill1_y", atkrangeConfig["skill1_y"].toVariant()).toInt();
    data["atk_range"] = atkrangeConfig;

    // 技能频率
    QJsonObject atkRecast = data["atk_recast"].toObject();
    atkRecast["buff1"] = config->getValue("atk_recast/buff1", atkRecast["buff1"].toVariant()).toInt();
    atkRecast["buff2"] = config->getValue("atk_recast/buff2", atkRecast["buff2"].toVariant()).toInt();
    atkRecast["buff3"] = config->getValue("atk_recast/buff3", atkRecast["buff3"].toVariant()).toInt();
    atkRecast["buff4"] = config->getValue("atk_recast/buff4", atkRecast["buff4"].toVariant()).toInt();
    atkRecast["buff5"] = config->getValue("atk_recast/buff5", atkRecast["buff5"].toVariant()).toInt();
    atkRecast["buff1_max"] = config->getValue("atk_recast/buff1_max", atkRecast["buff1_max"].toVariant()).toInt();
    atkRecast["buff2_max"] = config->getValue("atk_recast/buff2_max", atkRecast["buff2_max"].toVariant()).toInt();
    atkRecast["buff3_max"] = config->getValue("atk_recast/buff3_max", atkRecast["buff3_max"].toVariant()).toInt();
    atkRecast["buff4_max"] = config->getValue("atk_recast/buff4_max", atkRecast["buff4_max"].toVariant()).toInt();
    atkRecast["buff5_max"] = config->getValue("atk_recast/buff5_max", atkRecast["buff5_max"].toVariant()).toInt();
    data["atk_recast"] = atkRecast;
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
