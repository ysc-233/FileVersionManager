#include "logger.h"
#include <QDebug>
#include <QDateTime>

void Logger::info(const QString& msg)
{
    const auto time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    qDebug().noquote() << "[" << time << "]" << msg;
}
