#ifndef FILEHASHER_H
#define FILEHASHER_H


#pragma once
#include <QString>

class FileHasher {
public:
    // 对内存内容算 hash（核心）
    static QString sha256(const QByteArray& data);

    // 对文件路径算 hash（工具函数，可选）
    static QString sha256File(const QString& absPath);
};
#endif // FILEHASHER_H
