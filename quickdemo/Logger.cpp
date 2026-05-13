#include "Logger.h"

#include <QDebug>

Logger::Logger(QObject *parent) : QObject(parent) {}

QStringList Logger::history() const
{
    return m_history;
}

void Logger::log(const QString &message)
{
    qDebug() << "[Logger]" << message;   // printed to the application output
    m_history.append(message);
    emit historyChanged();
}

void Logger::clearHistory()
{
    m_history.clear();
    emit historyChanged();
}
