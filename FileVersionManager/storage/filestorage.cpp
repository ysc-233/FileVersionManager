#include "filestorage.h"
#include <QDir>
#include <QFile>
FileStorage::FileStorage(const QString &root)
{
    m_objectsDir = root + "/.fvm/objects";
    QDir().mkpath(root);
}

bool FileStorage::save(const QString &key, const QByteArray &data)
{
    const QString path = m_objectsDir + "/" + key;

    if (QFile::exists(path))
        return true; // 已存在直接跳过

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(data);
    return true;
}

QByteArray FileStorage::load(const QString &key)
{
    QFile file(m_objectsDir + "/" + key);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    return file.readAll();
}
