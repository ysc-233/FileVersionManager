#include "filestorage.h"
#include <QDir>
#include <QFile>
FileStorage::FileStorage(const QString &root)
    : rootDir_(root)
{
    QDir().mkpath(rootDir_);
}

void FileStorage::save(const QString &key, const QByteArray &data)
{
    const QString path = pathForKey(key);
    QFileInfo info(path);
    QDir().mkpath(info.path());

    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        file.write(data);
    }
}

QByteArray FileStorage::load(const QString &key)
{
    QFile file(pathForKey(key));
    if (!file.open(QIODevice::ReadOnly))
        return {};

    return file.readAll();
}

QString FileStorage::pathForKey(const QString &key) const
{
    // bb3daa... -> bb/3d/bb3daa....
    return rootDir_ + "/" +
           key.mid(0, 2) + "/" +
           key.mid(2, 2) + "/" +
           key + ".bin";
}
