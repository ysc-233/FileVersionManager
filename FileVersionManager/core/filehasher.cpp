#include "filehasher.h"
#include <QCryptographicHash>
#include <QFile>

QString FileHasher::sha256(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    QCryptographicHash hash(QCryptographicHash::Sha256);

    while (!file.atEnd()) {
        hash.addData(file.read(8192));
    }

    return hash.result().toHex();
}
