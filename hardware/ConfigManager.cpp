#include "ConfigManager.h"
#include <QFile>

ConfigManager &ConfigManager::instance()
{
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager()
{
    QString defaultFilePath = "config.yaml";
    if (!loadConfig(defaultFilePath))
    {
        qWarning() << "Failed to load default config file:" << defaultFilePath;
    }
}
bool ConfigManager::saveConfig(const QString &filePath)
{
    QMutexLocker locker(&mutex);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open config file for writing:" << filePath;
        return false;
    }

    QTextStream out(&file);
    QString yamlContent = serializeYaml(configData, 0);
    out << yamlContent;
    file.close();

    qDebug() << "Config saved successfully to" << filePath;
    return true;
}

bool ConfigManager::loadConfig(const QString &filePath)
{
    QMutexLocker locker(&mutex);
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open config file:" << filePath;
        return false;
    }

    QString content = file.readAll();
    file.close();

    configData = parseYaml(content);
    currentFilePath = filePath;

    return true;
}

QVariantMap ConfigManager::parseYaml(const QString &content)
{
    QVariantMap result;
    QStringList lines = content.split("\n", QString::SkipEmptyParts);

    QString currentGroup;
    for (const QString &line : lines)
    {
        QString trimmedLine = line.trimmed();

        if (trimmedLine.startsWith('#') || trimmedLine.isEmpty())
        {
            continue;
        }

        if (trimmedLine.endsWith(':') && !trimmedLine.contains(": "))
        {
            currentGroup = trimmedLine.left(trimmedLine.length() - 1).trimmed();
            result[currentGroup] = QVariantMap();
        }
        else if (currentGroup.isEmpty())
        {
            QStringList keyValue = trimmedLine.split(":", QString::KeepEmptyParts);
            if (keyValue.size() == 2)
            {
                result[keyValue[0].trimmed()] = keyValue[1].trimmed();
            }
        }
        else
        {
            QStringList keyValue = trimmedLine.split(":", QString::KeepEmptyParts);
            if (keyValue.size() == 2)
            {
                QVariantMap &group = result[currentGroup].toMap();
                group[keyValue[0].trimmed()] = keyValue[1].trimmed();
                result[currentGroup] = group;
            }
        }
    }

    return result;
}

QVariant ConfigManager::getValue(const QString &key) const
{
    QMutexLocker locker(&mutex);
    return configData.value(key);
}

QVariantMap ConfigManager::getGroup(const QString &group) const
{
    QMutexLocker locker(&mutex);
    return configData.value(group).toMap();
}

bool ConfigManager::updateValue(const QString &key, const QVariant &newValue)
{
    QMutexLocker locker(&mutex);

    QStringList keyParts = key.split("/");

    QVariantMap *currentMap = &configData;

    for (int i = 0; i < keyParts.size() - 1; ++i)
    {
        const QString &part = keyParts[i];

        if (currentMap->contains(part) && currentMap->value(part).canConvert<QVariantMap>())
        {
            currentMap = &currentMap->value(part).toMap();
        }
        else
        {
            qWarning() << "Path not found or invalid group: " << part;
            return false;
        }
    }

    const QString &lastKey = keyParts.last();
    if (currentMap->contains(lastKey))
    {
        qDebug() << "Updating key:" << lastKey;
        (*currentMap)[lastKey] = newValue;
        locker.unlock();
        return saveConfig(currentFilePath);
    }
    else
    {
        qWarning() << "Key not found in config: " << lastKey;
        return false;
    }
}
QString ConfigManager::serializeYaml(const QVariantMap &data, int indentLevel) const
{
    QStringList yamlLines;

    for (auto it = data.begin(); it != data.end(); ++it)
    {
        QString key = it.key();
        QVariant value = it.value();

        QString indent(indentLevel, ' ');

        qDebug() << "Serializing key:" << key;

        if (value.type() == QVariant::Map)
        {
            // Handle nested group
            yamlLines.append(indent + key + ":");
            yamlLines.append(serializeYaml(value.toMap(), indentLevel + 2));
        }
        else
        {
            // Handle key-value pairs
            yamlLines.append(indent + key + ": " + value.toString());
        }
    }

    return yamlLines.join("\n");
}
