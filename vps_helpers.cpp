#include "vps_helpers.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>

//How to use:
//QJsonDocument doc = QJsonDocument::fromJson(jsonBytes);
//QStringList path = {"company", "department", "engineering"};
//QJsonObject engineeringObj = getNestedObject(doc, path);

//Turn QJsonObj to string:
//QString jsonStr = QJsonDocument(QJsonObject).toJson(QJsonDocument::Compact);

QJsonObject getNestedObject(const QJsonDocument &doc, const QStringList &path)
{
    // Return empty if the document is not an object or path is empty
    if (!doc.isObject() || path.isEmpty()) {
        return QJsonObject();
    }

    // Start from the root object
    QJsonObject currentObj = doc.object();

    // Traverse the path
    for (const QString &key : path) {
        // Check if the key exists and holds an object
        if (currentObj.contains(key) && currentObj.value(key).isObject()) {
            currentObj = currentObj.value(key).toObject(); //Recurse into.
        } else {
            // Return empty object if path is broken
            return QJsonObject();
        }
    }

    return currentObj; //Return the object.
}

QJsonValue getJsonValueByPath(const QJsonDocument &doc, const QStringList &path)
{
    if (path.isEmpty() || doc.isNull())
        return QJsonValue();

    QJsonValue currentValue = doc.isObject()  ? QJsonValue(doc.object())
                              : doc.isArray() ? QJsonValue(doc.array())
                                              : QJsonValue();

    for (const QString &key : path) {
        if (currentValue.isObject()) {
            currentValue = currentValue.toObject().value(key);
        } else if (currentValue.isArray()) {
            bool ok = false;
            int index = key.toInt(&ok);
            if (ok && index >= 0 && index < currentValue.toArray().size()) {
                currentValue = currentValue.toArray().at(index);
            } else {
                return QJsonValue(); // Invalid array index
            }
        } else {
            return QJsonValue(); // Path goes deeper than JSON structure
        }

        if (currentValue.isUndefined())
            break;
    }

    return currentValue;
}