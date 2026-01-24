#ifndef METADATAMANAGER_H
#define METADATAMANAGER_H

#pragma once
#include <QString>
#include <QMap>
#include <QList>
#include "versioninfo.h"

class MetadataManager
{
public:
    explicit MetadataManager(const QString& rootPath);

    bool hasVersion(const QString& filePath, const QString& versionId) const;
    void addVersion(const VersionInfo& info);
    bool save();

    VersionInfo find(const QString& filePath, const QString& versionId) const;
    QList<VersionInfo> versions(const QString& filePath) const;
    QMap<QString, QList<VersionInfo>> allVersions() const { return m_data; }

private:
    QString m_rootPath;
    QString m_metadataFile;
    QMap<QString, QList<VersionInfo>> m_data;

private:
    QJsonDocument load() const;
    void ensureMetadata();
    QString metadataFilePath() const;
};
#endif // METADATAMANAGER_H
