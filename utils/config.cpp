#include "Config.h"

Config* Config::m_instance = nullptr;

Config::Config(QObject* parent)
    : QObject(parent)
{
}

Config* Config::instance()
{
    if (!m_instance) {
        m_instance = new Config();
    }
    return m_instance;
}

void Config::setFilePath(const QString& filePath)
{
    m_filePath = filePath;
    load();
}

QString Config::getFilePath() const
{
    return m_filePath;
}

QVariant Config::getValue(const QString& keyPath, const QVariant& defaultValue) const
{
    QStringList keys = keyPath.split('/');
    QJsonValue value = getValueHelper(keys, m_jsonObject);
    return value.isUndefined() ? defaultValue : value.toVariant();
}

QJsonValue Config::getValueHelper(const QStringList& keys, const QJsonObject& jsonObj) const
{
    if (keys.isEmpty())
        return QJsonValue();

    QString currentKey = keys.first();
    if (!jsonObj.contains(currentKey))
        return QJsonValue();

    QJsonValue value = jsonObj.value(currentKey);
    if (keys.size() == 1)
        return value;

    if (value.isObject())
        return getValueHelper(keys.mid(1), value.toObject());

    return QJsonValue();
}

void Config::setValue(const QString& keyPath, const QVariant& value)
{
    QStringList keys = keyPath.split('/');
    setValueHelper(keys, m_jsonObject, value);
}

void Config::setValueHelper(const QStringList& keys, QJsonObject& jsonObj, const QVariant& value)
{
    if (keys.isEmpty())
        return;

    QString currentKey = keys.first();
    if (keys.size() == 1) {
        jsonObj.insert(currentKey, QJsonValue::fromVariant(value));
        return;
    }

    QJsonObject subObj = jsonObj[currentKey].toObject();
    setValueHelper(keys.mid(1), subObj, value);
    jsonObj.insert(currentKey, subObj);
}

void Config::removeValue(const QString& keyPath)
{
    QStringList keys = keyPath.split('/');
    removeValueHelper(keys, m_jsonObject);
}

void Config::removeValueHelper(const QStringList& keys, QJsonObject& jsonObj)
{
    if (keys.isEmpty())
        return;

    QString currentKey = keys.first();
    if (keys.size() == 1) {
        jsonObj.remove(currentKey);
        return;
    }

    if (jsonObj.contains(currentKey) && jsonObj[currentKey].isObject()) {
        QJsonObject subObj = jsonObj[currentKey].toObject();
        removeValueHelper(keys.mid(1), subObj);
        jsonObj.insert(currentKey, subObj);
    }
}

bool Config::contains(const QString& keyPath) const
{
    QStringList keys = keyPath.split('/');
    QJsonValue value = getValueHelper(keys, m_jsonObject);
    if(value.isNull())
    {
        return false;
    }
    return !value.isUndefined();
}

void Config::save()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Couldn't open file for writing:" << m_filePath;
        return;
    }

    QJsonDocument doc(m_jsonObject);
    file.write(doc.toJson());
    file.close();
}

void Config::saveArray()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Couldn't open file for writing:" << m_filePath;
        return;
    }

    QJsonDocument doc;
    doc.setArray(m_jsonArray.toArray());
    file.write(doc.toJson());
    file.close();
}

void Config::load()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Couldn't open file:" << m_filePath;
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    m_jsonObject = doc.object();
}
