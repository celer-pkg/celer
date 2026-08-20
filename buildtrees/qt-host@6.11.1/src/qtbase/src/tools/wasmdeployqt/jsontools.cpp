// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QDir>
#include <QJsonArray>
#include <QJsonObject>

#include "jsontools.h"
#include "common.h"

#include <iostream>
#include <optional>

namespace JsonTools {

bool savePreloadFile(QSet<PreloadEntry> preload, QString destFile)
{

    QJsonArray jsonArray;
    for (const PreloadEntry &entry : preload) {
        QJsonObject obj;
        obj["source"] = entry.source;
        obj["destination"] = entry.destination;
        jsonArray.append(obj);
    }
    QJsonDocument doc(jsonArray);

    QFile outFile(destFile);
    if (outFile.exists()) {
        if (!outFile.remove()) {
            std::cout << "ERROR: Failed to delete old file: " << outFile.fileName().toStdString()
                      << std::endl;
            return false;
        }
    }
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cout << "ERROR: Failed to open file for writing:" << outFile.fileName().toStdString()
                  << std::endl;
        return false;
    }
    if (outFile.write(doc.toJson(QJsonDocument::Indented)) == -1) {
        std::cout << "ERROR: Failed writing into file :" << outFile.fileName().toStdString()
                  << std::endl;
        return false;
    }
    if (!outFile.flush()) {
        std::cout << "ERROR: Failed flushing the file :" << outFile.fileName().toStdString()
                  << std::endl;
        return false;
    }
    outFile.close();
    return true;
}

std::optional<QSet<PreloadEntry>> getPreloadsFromQmlImportScannerOutput(QString output)
{
    QString qtLibPath = "$QTDIR/lib";
    QString qtQmlPath = "$QTDIR/qml";
    QString qtDeployQmlPath = "/qt/qml";
    QSet<PreloadEntry> res;
    auto addImport = [&res](const PreloadEntry &entry) {
        // qDebug() << "adding " << entry.source << "" << entry.destination;
        res.insert(entry);
    };

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        std::cout << "ERROR: QmlImport JSON parse error: " << parseError.errorString().toStdString()
                  << std::endl;
        return std::nullopt;
    }
    if (!doc.isArray()) {
        std::cout << "ERROR: QmlImport JSON is not an array." << std::endl;
        return std::nullopt;
    }

    QJsonArray jsonArray = doc.array();
    for (const QJsonValue &value : jsonArray) {
        if (value.isObject()) {
            QJsonObject obj = value.toObject();
            auto relativePath = obj["relativePath"].toString();
            auto plugin = obj["plugin"].toString();
            if (plugin.isEmpty() || relativePath.isEmpty()) {
                continue;
            }
            auto pluginFilename = "lib" + plugin + ".so";
            addImport(PreloadEntry{
                    QDir::cleanPath(qtQmlPath + "/" + relativePath + "/" + pluginFilename),
                    QDir::cleanPath(qtDeployQmlPath + "/" + relativePath + "/" + pluginFilename) });
            addImport(PreloadEntry{
                    QDir::cleanPath(qtQmlPath + "/" + relativePath + "/" + "qmldir"),
                    QDir::cleanPath(qtDeployQmlPath + "/" + relativePath + "/" + "qmldir") });
        }
    }

    return res;
}

}; // namespace JsonTools
