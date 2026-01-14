#ifndef FILEm_storageH
#define FILEm_storageH

#pragma once
#include "storage/storage.h"

class FileStorage : public Storage {
public:
    explicit FileStorage(const QString& root);

    bool save(const QString& key, const QByteArray& data) override;
    QByteArray load(const QString& key) override;
private:
    QString m_objectsDir;
};

#endif // FILEm_storageH
