#ifndef FILESTORAGE_H
#define FILESTORAGE_H

#pragma once
#include "storage/Storage.h"

class FileStorage : public Storage {
public:
    void save(const QString& key, const QByteArray& data) override;
    QByteArray load(const QString& key) override;
};

#endif // FILESTORAGE_H
