#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtQml/qqml.h>

// Exposed to QML as Logger { }
// Demonstrates passing a string from QML → C++, logging it, and reflecting
// the full log history back to QML as a property.
class Logger : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    // Read-only list of all logged messages — QML can bind to this
    Q_PROPERTY(QStringList history READ history NOTIFY historyChanged)

public:
    explicit Logger(QObject *parent = nullptr);

    QStringList history() const;

    // Called from QML: logs the message via qDebug and appends to history
    Q_INVOKABLE void log(const QString &message);

    // Clears the history list
    Q_INVOKABLE void clearHistory();

signals:
    void historyChanged();

private:
    QStringList m_history;
};
