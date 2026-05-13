#include "JsonSerializer.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

JsonSerializer::JsonSerializer(QObject *parent) : QObject(parent) {}

// ── Object (QVariantMap ↔ QJsonObject) ────────────────────────────────────────

QString JsonSerializer::serialize(const QVariantMap &obj) const
{
    const QJsonObject jsonObj = QJsonObject::fromVariantMap(obj);
    const QJsonDocument doc(jsonObj);
    return doc.toJson(QJsonDocument::Indented);
}

QVariantMap JsonSerializer::deserialize(const QString &json) const
{
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isNull() || !doc.isObject())
        return {};
    return doc.object().toVariantMap();
}

// ── Array (QVariantList ↔ QJsonArray) ─────────────────────────────────────────

QString JsonSerializer::serializeList(const QVariantList &list) const
{
    const QJsonArray arr = QJsonArray::fromVariantList(list);
    const QJsonDocument doc(arr);
    return doc.toJson(QJsonDocument::Indented);
}

QVariantList JsonSerializer::deserializeList(const QString &json) const
{
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isNull() || !doc.isArray())
        return {};
    return doc.array().toVariantList();
}
