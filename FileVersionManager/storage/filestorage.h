#ifndef FILESTORAGE_H
#define FILESTORAGE_H

#pragma once
#include "storage/storage.h"

class FileStorage : public Storage {
public:
    explicit FileStorage(const QString& root);

    void save(const QString& key, const QByteArray& data) override;
    QByteArray load(const QString& key) override;
private:
    QString rootDir_;
    QString pathForKey(const QString& key) const;
};

#endif // FILESTORAGE_H
