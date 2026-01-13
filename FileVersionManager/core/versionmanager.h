#ifndef VERSIONMANAGER_H
#define VERSIONMANAGER_H

#pragma once
#include "metadatamanager.h"
#include <QObject>
#include <QString>

class VersionManager : public QObject {
    Q_OBJECT
public:
    explicit VersionManager(QObject* parent = nullptr);

public slots:
    void onFileChanged(const QString& path);
private:
    MetadataManager metadata_;
};

#endif // VERSIONMANAGER_H
