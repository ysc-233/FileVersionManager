#include "logger.h"
#include <QDir>
#include <QDateTime>
#include <QTextStream>

QString Logger::s_rootPath;
QFile Logger::s_file;

void Logger::init(const QString& rootPath)
{
    s_rootPath = rootPath;

    const QString logDir = s_rootPath + "/.fvm/logs";
    QDir dir(logDir);
    if (!dir.exists())
        dir.mkpath(".");

    s_file.setFileName(logDir + "/fvm.log");
    s_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
}

void Logger::info(const QString& msg)
{
    write("INFO", msg);
}

void Logger::warn(const QString& msg)
{
    write("WARN", msg);
}

void Logger::error(const QString& msg)
{
    write("ERROR", msg);
}

void Logger::shutdown()
{
    if (s_file.isOpen())
        s_file.close();
    s_rootPath.clear();
}

void Logger::write(const QString& level, const QString& msg)
{
    if (!s_file.isOpen())
        return;

    QTextStream out(&s_file);
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
        << " [" << level << "] "
        << msg << "\n";
    out.flush();
}
