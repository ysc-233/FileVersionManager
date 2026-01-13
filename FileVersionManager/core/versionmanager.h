#ifndef VERSIONMANAGER_H
#define VERSIONMANAGER_H

#pragma once
#include "metadatamanager.h"
#include "storage/filestorage.h"
#include <QObject>
#include <QString>

class VersionManager : public QObject {
    Q_OBJECT
public:
    explicit VersionManager(QObject* parent = nullptr);
    bool rollback(const QString& filePath,const QString& versionId);

public slots:
    void onFileChanged(const QString& path);

private:
    MetadataManager metadata_;
    FileStorage storage_;
};

#endif // VERSIONMANAGER_H
