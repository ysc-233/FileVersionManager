#include "metadatamanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QDebug>

MetadataManager::MetadataManager(const QString& rootPath)
    : m_rootPath(rootPath)
{
    m_metadataFile = metadataFilePath();
    ensureMetadata();

    const QJsonDocument doc = load();
    const QJsonObject root = doc.object();
    const QJsonObject filesObj = root["files"].toObject();

    for (auto it = filesObj.begin(); it != filesObj.end(); ++it) {
        const QString filePath = it.key();
        const QJsonArray arr = it.value().toArray();

        QList<VersionInfo> list;
        for (const auto& v : arr) {
            QJsonObject obj = v.toObject();

            VersionInfo info;
            info.filePath = filePath;
            info.versionId = obj["versionId"].toString();
            info.timestamp = QDateTime::fromString(
                obj["timestamp"].toString(),
                "yyyy-MM-dd HH:mm:ss");

            list.append(info);
        }
        m_data.insert(filePath, list);
    }
}

QString MetadataManager::metadataFilePath() const
{
    return m_rootPath + "/.fvm/metadata.json";
}

void MetadataManager::ensureMetadata()
{
    QDir dir(m_rootPath + "/.fvm/objects");
    if (!dir.exists())
        dir.mkpath(".");

    QFile file(m_metadataFile);
    if (file.exists())
        return;

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonObject root;
        root["files"] = QJsonObject();
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        file.close();
    }
}

QJsonDocument MetadataManager::load() const
{
    QFile file(m_metadataFile);
    if (!file.open(QIODevice::ReadOnly))
        return QJsonDocument(QJsonObject());

    return QJsonDocument::fromJson(file.readAll());
}

bool MetadataManager::hasVersion(const QString& filePath,
                                 const QString& versionId) const
{
    const auto it = m_data.find(filePath);
    if (it == m_data.end())
        return false;

    for (const auto& v : it.value()) {
        if (v.versionId == versionId)
            return true;
    }
    return false;
}

void MetadataManager::addVersion(const VersionInfo& info)
{
    m_data[info.filePath].append(info);
}

bool MetadataManager::save()
{
    QJsonObject root;
    QJsonObject filesObj;

    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        QJsonArray arr;
        for (const auto& v : it.value()) {
            QJsonObject obj;
            obj["versionId"] = v.versionId;
            obj["timestamp"] =
                v.timestamp.toString("yyyy-MM-dd HH:mm:ss");
            arr.append(obj);
        }
        filesObj[it.key()] = arr;
    }

    root["files"] = filesObj;

    QFile file(m_metadataFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

VersionInfo MetadataManager::find(const QString& filePath,
                                  const QString& versionId) const
{
    const auto list = m_data.value(filePath);
    for (const auto& v : list) {
        if (v.versionId == versionId)
            return v;
    }
    return VersionInfo{};
}

QList<VersionInfo> MetadataManager::versions(
    const QString& filePath) const
{
    return m_data.value(filePath);
}
