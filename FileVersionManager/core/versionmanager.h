#ifndef m_versionManagerH
#define m_versionManagerH

#pragma once
#include "metadatamanager.h"
#include "storage/filestorage.h"
#include <QObject>
#include <QString>

class VersionManager : public QObject
{
    Q_OBJECT
public:
    explicit VersionManager(const QString& rootPath, QObject* parent = nullptr);

    QList<VersionInfo> versions(const QString& filePath) const;
    QMap<QString, QList<VersionInfo>> allVersions() const;

    bool rollback(const QString& filePath, const QString& versionId);

public slots:
    void onFileChanged(const QString& filePath);

private:
    MetadataManager m_metadata;
    FileStorage m_storage;
};

#endif // m_versionManagerH
