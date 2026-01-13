#ifndef METADATAMANAGER_H
#define METADATAMANAGER_H

#pragma once
#include <QString>
#include <QMap>
#include <QList>
#include "versioninfo.h"

class MetadataManager {
public:
    explicit MetadataManager(const QString& file);

    QList<VersionInfo> versions(const QString& filePath);
    bool hasVersion(const QString& filePath, const QString& hash);

    void addVersion(const VersionInfo& info);
    bool save();

private:
    QString metadataFile_;
    QMap<QString, QList<VersionInfo>> data_;

    void load();
};
#endif // METADATAMANAGER_H
