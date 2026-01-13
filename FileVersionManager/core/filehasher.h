#ifndef FILEHASHER_H
#define FILEHASHER_H


#pragma once
#include <QString>

class FileHasher {
public:
    static QString sha256(const QString& filePath);
};
#endif // FILEHASHER_H
