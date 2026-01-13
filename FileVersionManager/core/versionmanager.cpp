#include "versionmanager.h"
#include "utils/logger.h"
#include "filehasher.h"

VersionManager::VersionManager(QObject* parent)
    : QObject(parent),
      metadata_(".fvm/metadata.json")
{

}

void VersionManager::onFileChanged(const QString &path)
{
    const QString hash = FileHasher::sha256(path);
    if (hash.isEmpty())
        return;

    if (metadata_.hasVersion(path, hash)) {
        Logger::info("No content change, skip version");
        return;
    }

    VersionInfo info;
    info.filePath = path;
    info.versionId = hash;
    info.timestamp = QDateTime::currentDateTime();

    metadata_.addVersion(info);
    if (metadata_.save())
    {
        Logger::info(QString("New version created: %1").arg(hash.left(8)));
    }
    else
    {
        Logger::info("Version created in memory, but failed to persist");
    }
}
