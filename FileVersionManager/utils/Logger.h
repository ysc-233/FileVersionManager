#ifndef LOGGER_H
#define LOGGER_H

#pragma once
#include <QString>
#include <QFile>

class Logger
{
public:
    static void init(const QString& rootPath);
    static void info(const QString& msg);
    static void warn(const QString& msg);
    static void error(const QString& msg);
    static void shutdown();
private:
    static void write(const QString& level, const QString& msg);
    static QString logFilePath();

private:
    static QString s_rootPath;
    static QFile s_file;
};


#endif // LOGGER_H
