#ifndef VPS_HELPERS_H
#define VPS_HELPERS_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>

QJsonObject getNestedObject(const QJsonDocument &doc, const QStringList &path);
QJsonValue getJsonValueByPath(const QJsonDocument &doc, const QStringList &path);

#endif // VPS_HELPERS_H
