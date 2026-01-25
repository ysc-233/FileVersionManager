#ifndef VERSIONINFO_H
#define VERSIONINFO_H


#pragma once
#include <QString>
#include <QDateTime>
enum class FileState {
    Normal,
    Deleted
};
struct VersionInfo {
    QString filePath;      // 原始文件路径
    QString versionId;     // hash
    QDateTime timestamp;   // 版本时间
    qint64 fileSize;   // 文件大小（字节）
    FileState state = FileState::Normal;
};

#endif // VERSIONINFO_H
