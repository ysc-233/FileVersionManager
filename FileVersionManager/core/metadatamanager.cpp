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
    const QString objectsDir = m_rootPath + "/.fvm/objects";
    const QJsonDocument doc = load();
    const QJsonObject root = doc.object();
    const QJsonObject filesObj = root["files"].toObject();

    for (auto it = filesObj.begin(); it != filesObj.end(); ++it) {

        QString storedPath = it.key();
        QString relativePath;

        // 兼容旧数据：如果是绝对路径，转为相对路径
        if (QDir::isAbsolutePath(storedPath)) {
            relativePath = QDir(m_rootPath).relativeFilePath(storedPath);
        } else {
            relativePath = storedPath;
        }

        const QJsonArray arr = it.value().toArray();
        QList<VersionInfo> list;

        for (const auto& v : arr) {
            QJsonObject obj = v.toObject();

            VersionInfo info;
            info.filePath  = relativePath;
            info.versionId = obj["versionId"].toString();
            info.timestamp = QDateTime::fromString(
                obj["timestamp"].toString(),
                "yyyy-MM-dd HH:mm:ss"
            );
            info.fileSize = obj["fileSize"].toInt();
            QString s = obj.value("state").toString("normal");
            info.state = (s == "deleted") ? FileState::Deleted : FileState::Normal;

            if (info.fileSize == 0) {
                QFileInfo fi(objectsDir + "/" + info.versionId);
                if (fi.exists())
                    info.fileSize = fi.size();
            }

            list.append(info);
            Q_ASSERT(!QDir::isAbsolutePath(info.filePath));
        }

        m_versions.insert(relativePath, list);
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

bool MetadataManager::hasVersion(const QString& filePath,const QString& versionId) const
{
    const auto it = m_versions.find(filePath);
    if (it == m_versions.end())
        return false;

    for (const auto& v : it.value()) {
        if (v.versionId == versionId)
            return true;
    }
    return false;
}

void MetadataManager::addVersion(const VersionInfo& info)
{
    m_versions[info.filePath].append(info);
}

bool MetadataManager::save()
{
    QJsonObject root;
    QJsonObject filesObj;

    for (auto it = m_versions.begin(); it != m_versions.end(); ++it) {
        QJsonArray arr;
        for (const auto& v : it.value()) {
            QJsonObject obj;
            obj["versionId"] = v.versionId;
            obj["timestamp"] = v.timestamp.toString("yyyy-MM-dd HH:mm:ss");
            obj["fileSize"] = v.fileSize;
            obj["state"] = (v.state == FileState::Deleted) ? "deleted" : "normal";
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

VersionInfo MetadataManager::find(const QString& filePath,const QString& versionId) const
{
    const auto list = m_versions.value(filePath);
    for (const auto& v : list) {
        if (v.versionId == versionId)
            return v;
    }
    return VersionInfo{};
}

QList<VersionInfo> MetadataManager::versions(const QString& filePath) const
{
    return m_versions.value(filePath);
}

bool MetadataManager::hasFile(const QString &filePath) const
{
    return m_versions.contains(filePath);
}

bool MetadataManager::markDeleted(const QString &relPath)
{
    auto it = m_versions.find(relPath);
    if (it == m_versions.end() || it.value().isEmpty())
        return false;

    const VersionInfo& last = it.value().last();

    // 已经是 deleted，不重复标记
    if (last.state == FileState::Deleted)
        return true;

    VersionInfo deleted;
    deleted.filePath  = relPath;
    deleted.versionId = last.versionId;   // 继承最后一个版本
    deleted.timestamp = QDateTime::currentDateTime();
    deleted.fileSize  = last.fileSize;
    deleted.state     = FileState::Deleted;

    it.value().append(deleted);
    save();
    return true;
}

QString MetadataManager::latestVersionHash(const QString &filePath) const
{
    auto it = m_versions.find(filePath);
    if (it == m_versions.end())
        return QString();
    const auto& list = it.value();
    for (auto rit = list.rbegin(); rit != list.rend(); ++rit) {
        if (rit->state == FileState::Normal)
            return rit->versionId;
    }

    return QString();
}

void MetadataManager::renameFile(const QString &oldRelPath, const QString &newRelPath)
{
    if (!m_versions.contains(oldRelPath))
        return;

    if (m_versions.contains(newRelPath))
        return; // 防御：避免覆盖已有记录

    auto list = m_versions.take(oldRelPath);

    for (auto& v : list) {
        v.filePath = newRelPath;
        v.state = FileState::Normal;
    }

    m_versions.insert(newRelPath, list);
    save();
}

bool MetadataManager::isDeleted(const QString &filePath) const
{
    auto it = m_versions.find(filePath);
     if (it ==m_versions.end() || it.value().isEmpty())
         return false;
     return it.value().last().state == FileState::Deleted;
}

