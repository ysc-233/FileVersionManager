#ifndef VERSIONINFO_H
#define VERSIONINFO_H


#pragma once
#include <QString>
#include <QDateTime>

struct VersionInfo {
    QString filePath;      // 原始文件路径
    QString versionId;     // hash
    QDateTime timestamp;   // 版本时间
};

#endif // VERSIONINFO_H
