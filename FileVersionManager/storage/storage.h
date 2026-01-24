#ifndef m_storageH
#define m_storageH


#pragma once
#include <QString>
#include <QByteArray>

class Storage {
public:
    virtual ~Storage() = default;

    virtual bool save(const QString& key, const QByteArray& data) = 0;
    virtual QByteArray load(const QString& key) = 0;
};

#endif // m_storageH
