#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QVariantMap>
#include <QString>
#include <QMutex>
#include <QDebug>

class ConfigManager {
public:
    static ConfigManager &instance();

    bool loadConfig(const QString &filePath);

    QVariant getValue(const QString &key) const;
    QVariantMap getGroup(const QString &group) const;

    bool updateValue(const QString &key, const QVariant &newValue);
    bool saveConfig(const QString &filePath);
    QString serializeYaml(const QVariantMap &data, int indentLevel) const;

private:
    ConfigManager();
    ~ConfigManager() = default;

    ConfigManager(const ConfigManager &) = delete;
    ConfigManager &operator=(const ConfigManager &) = delete;

    QVariantMap configData;
    QString currentFilePath;

    QVariantMap parseYaml(const QString &content);

    mutable QMutex mutex;
};

#endif // CONFIGMANAGER_H
