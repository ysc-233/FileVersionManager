#ifndef STORAGE_H
#define STORAGE_H


#pragma once
#include <QString>
#include <QByteArray>

class Storage {
public:
    virtual ~Storage() = default;

    virtual void save(const QString& key, const QByteArray& data) = 0;
    virtual QByteArray load(const QString& key) = 0;
};

#endif // STORAGE_H
