#include "versionmanager.h"
#include "utils/logger.h"

VersionManager::VersionManager(QObject* parent)
{

}

void VersionManager::onFileChanged(const QString &path)
{
    Logger::info(QString("File changed: %1").arg(path));
}
