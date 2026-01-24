#include "filehasher.h"
#include <QCryptographicHash>
#include <QFile>

QString FileHasher::sha256(const QByteArray& data)
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(data);
    return hash.result().toHex();
}

QString FileHasher::sha256File(const QString& absPath)
{
    QFile file(absPath);
    if (!file.open(QIODevice::ReadOnly))
        return QString();

    QCryptographicHash hash(QCryptographicHash::Sha256);

    constexpr qint64 BufSize = 64 * 1024;
    char buffer[BufSize];

    while (!file.atEnd()) {
        qint64 n = file.read(buffer, BufSize);
        if (n > 0)
            hash.addData(buffer, n);
    }

    return hash.result().toHex();
}
