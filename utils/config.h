#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QVariant>

class Config : public QObject
{
    Q_OBJECT
public:
    static Config* instance(); // 获取单例实例

    void setFilePath(const QString& filePath); // 设置 JSON 文件路径
    QString getFilePath() const;               // 获取 JSON 文件路径

    QVariant getValue(const QString& keyPath, const QVariant& defaultValue = QVariant()) const; // 读取值
    void setValue(const QString& keyPath, const QVariant& value);  // 修改或新增值
    void removeValue(const QString& keyPath);  // 删除键值对
    bool contains(const QString& keyPath) const; // 检查是否包含键

    void save();  // 保存修改到文件
    void load();  // 从文件加载数据

    QJsonObject m_jsonObject;

    QJsonValue getValueHelper(const QStringList& keys, const QJsonObject& jsonObj) const;
private:
    explicit Config(QObject* parent = nullptr);
    ~Config() = default;

    QString m_filePath;
    static Config* m_instance;


    void setValueHelper(const QStringList& keys, QJsonObject& jsonObj, const QVariant& value);
    void removeValueHelper(const QStringList& keys, QJsonObject& jsonObj);
};

#endif // CONFIG_H
