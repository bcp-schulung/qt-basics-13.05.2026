#pragma once

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QtQml/qqml.h>

// Exposed to QML as JsonSerializer { }
// Demonstrates Qt's JSON API: QJsonDocument, QJsonObject, QJsonArray
class JsonSerializer : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit JsonSerializer(QObject *parent = nullptr);

    // Serialize a JS/QML object (QVariantMap) → indented JSON string
    Q_INVOKABLE QString serialize(const QVariantMap &obj) const;

    // Deserialize a JSON string → JS/QML object (QVariantMap)
    // Returns an empty map if the input is invalid JSON or not an object
    Q_INVOKABLE QVariantMap deserialize(const QString &json) const;

    // Serialize a JS/QML array (QVariantList) → indented JSON string
    Q_INVOKABLE QString serializeList(const QVariantList &list) const;

    // Deserialize a JSON string → JS/QML array (QVariantList)
    // Returns an empty list if the input is invalid JSON or not an array
    Q_INVOKABLE QVariantList deserializeList(const QString &json) const;
};
