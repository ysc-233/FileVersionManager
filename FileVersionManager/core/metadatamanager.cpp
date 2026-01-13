#include "metadatamanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QDebug>

MetadataManager::MetadataManager(const QString& file)
    : metadataFile_(file)
{
    // 确保 .fvm 目录存在
    QFileInfo info(metadataFile_);
    QDir dir;
    dir.mkpath(info.path());

    load();
}

bool MetadataManager::hasVersion(const QString &filePath, const QString &hash)
{
    const auto& list = data_[filePath];
    for (const auto& v : list)
    {
        if (v.versionId == hash)
            return true;
    }
    return false;
}

void MetadataManager::addVersion(const VersionInfo &info)
{
    data_[info.filePath].push_back(info);
}

bool MetadataManager::save()
{
    QJsonObject root;

    for (auto it = data_.begin(); it != data_.end(); ++it)
    {
        QJsonArray arr;
        for (const auto& v : it.value())
        {
            QJsonObject obj;
            obj["versionId"] = v.versionId;
            obj["timestamp"] = v.timestamp.toString(Qt::ISODate);
            arr.append(obj);
        }
        root[it.key()] = arr;
    }

    QFile file(metadataFile_);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qWarning() << "Failed to open metadata file:" << metadataFile_;
        return false;
    }

    file.write(QJsonDocument(root).toJson());
    return true;
}

VersionInfo MetadataManager::find(const QString &filePath, const QString &versionId) const
{
    for (const auto& v : data_.value(filePath))
    {
        if (v.versionId == versionId)
            return v;
    }
    return {};
}

QList<VersionInfo> MetadataManager::versions(const QString &filePath) const
{
    return data_.value(filePath);
}

void MetadataManager::load()
{
    QFile file(metadataFile_);
    if (!file.open(QIODevice::ReadOnly))
        return;

    const auto doc = QJsonDocument::fromJson(file.readAll());
    const auto root = doc.object();

    for (auto it = root.begin(); it != root.end(); ++it)
    {
        QList<VersionInfo> list;
        for (const auto& v : it.value().toArray())
        {
            QJsonObject obj = v.toObject();
            VersionInfo info;
            info.filePath = it.key();
            info.versionId = obj["versionId"].toString();
            info.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
            list.push_back(info);
        }
        data_[it.key()] = list;
    }
}
