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
    enum class RollbackError
    {
        None,
        VersionNotFound,
        LoadFailed,
        WriteFailed,
        RenameFailed
    };
    struct PendingDelete {
        QString relPath;
        QString hash;
        QDateTime time;
    };

    explicit VersionManager(const QString& rootPath, QObject* parent = nullptr);
    void initializeWorkspace();
    QList<VersionInfo> versions(const QString& filePath) const;
    QMap<QString, QList<VersionInfo>> allVersions() const;
    bool rollback(const QString& filePath, const QString& versionId,RollbackError* error = nullptr);
    QMap<QString, QString> currentVersions() const;
    VersionInfo currentVersionInfo(const QString& filePath) const;
    static QString buildDiffText(const VersionInfo& current,const VersionInfo& target);
    void markDeleted(const QString &relPath);
public slots:
    void onFileChanged(const QString& absPath);

private:
    QString m_rootPath;
    MetadataManager m_metadata;
    FileStorage m_storage;
    QList<PendingDelete> m_recentDeletes;
};

#endif // m_versionManagerH
