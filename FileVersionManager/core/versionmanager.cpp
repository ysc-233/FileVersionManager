#include "versionmanager.h"
#include "utils/logger.h"
#include "filehasher.h"
#include "storage/filestorage.h"

VersionManager::VersionManager(QObject* parent)
    : QObject(parent),
        metadata_(".fvm/metadata.json"),
        storage_(".fvm/objects")
{

}

bool VersionManager::rollback(const QString &filePath, const QString &versionId)
{
    const QByteArray data = storage_.load(versionId);
    if (data.isEmpty()) {
        Logger::info("Rollback failed: version not found");
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        Logger::info("Rollback failed: cannot write file");
        return false;
    }

    file.write(data);
    Logger::info("Rollback success: " + versionId.left(8));
    return true;
}

void VersionManager::onFileChanged(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;

    const QByteArray content = file.readAll();
    const QString hash = FileHasher::sha256(path);
    if (hash.isEmpty())
        return;

    if (metadata_.hasVersion(path, hash)) {
        Logger::info("No content change, skip version");
        return;
    }

    storage_.save(hash, content);

    VersionInfo info;
    info.filePath = path;
    info.versionId = hash;
    info.timestamp = QDateTime::currentDateTime();

    metadata_.addVersion(info);
    if (metadata_.save())
    {
        Logger::info("New version created: " + hash.left(8));
    }
    else
    {
        Logger::info("Version created in memory, but failed to persist");
    }
}
