// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "lightmapviewerhelpers.h"

#include <QtQuick3DRuntimeRender/private/qssglightmapio_p.h>

#include <QRgb>
#include <QVector>
#include <QImage>
#include <QFile>
#include <QJsonObject>
#include <QDir>

static QRgb numberToBBGRColor(quint32 i, quint32 N, bool useAlpha)
{
    if (i < 1 || i > N) {
        return qRgba(0, 0, 0, useAlpha ? 0 : 0xff);
    }

    int range = N - 1; // exclude 0
    double t = static_cast<double>(i - 1) / range;

    quint8 r = 0, g = 0, b = 0;

    if (t < 0.5) {
        // Blue -> Green
        double t2 = t / 0.5; // normalize 0..1
        g = 255 * t2;
        b = 255 * (1.0 - t2);
    } else {
        // Green -> Red
        double t2 = (t - 0.5) / 0.5;
        r = 255 * t2;
        g = 255 * (1.0 - t2);
    }

    return qRgba(r, g, b, 0xff);
}

QString LightmapViewerHelpers::lightmapTagToString(QSSGLightmapIODataTag tag)
{
    switch (tag) {
    case QSSGLightmapIODataTag::Unset:
        return QStringLiteral("Unset");
        break;
    case QSSGLightmapIODataTag::Mask:
        return QStringLiteral("Mask");
        break;
    case QSSGLightmapIODataTag::Texture_Final:
        return QStringLiteral("Texture_Final");
        break;
    case QSSGLightmapIODataTag::Texture_Direct:
        return QStringLiteral("Texture_Direct");
        break;
    case QSSGLightmapIODataTag::Texture_Indirect:
        return QStringLiteral("Texture_Indirect");
        break;
    case QSSGLightmapIODataTag::Metadata:
        return QStringLiteral("Metadata");
        break;
    case QSSGLightmapIODataTag::SceneMetadata:
        return QStringLiteral("SceneMetadata");
        break;
    case QSSGLightmapIODataTag::Mesh:
        return QStringLiteral("Mesh");
        break;
    case QSSGLightmapIODataTag::Count:
        break;
    }
    return QStringLiteral("Invalid");
}

QSSGLightmapIODataTag LightmapViewerHelpers::stringToLightmapTag(const QString &tag)
{
    if (tag == QStringLiteral("Unset"))
        return QSSGLightmapIODataTag::Unset;
    if (tag == QStringLiteral("Mask"))
        return QSSGLightmapIODataTag::Mask;
    if (tag == QStringLiteral("Texture_Final"))
        return QSSGLightmapIODataTag::Texture_Final;
    if (tag == QStringLiteral("Texture_Direct"))
        return QSSGLightmapIODataTag::Texture_Direct;
    if (tag == QStringLiteral("Texture_Indirect"))
        return QSSGLightmapIODataTag::Texture_Indirect;
    if (tag == QStringLiteral("Metadata"))
        return QSSGLightmapIODataTag::Metadata;
    if (tag == QStringLiteral("SceneMetadata"))
        return QSSGLightmapIODataTag::SceneMetadata;
    if (tag == QStringLiteral("Mesh"))
        return QSSGLightmapIODataTag::Mesh;

    qWarning() << "Could not match tag for: " << tag;
    return QSSGLightmapIODataTag::Unset;
}

LightmapViewerHelpers::SceneMetadata LightmapViewerHelpers::processSceneMetadata(const QVariantMap &map)
{
    SceneMetadata sceneMetadata;
    if (map.isEmpty())
        return sceneMetadata;

    const auto doc = QJsonDocument::fromVariant(map);
    const auto obj = doc.object();

    sceneMetadata.qtVersion = obj.value("qt_version").toString();
    const auto bakeStartTs = obj.value("bake_start_time").toInteger();
    if (bakeStartTs > 0) {
        sceneMetadata.bakeStartTime = QDateTime::fromMSecsSinceEpoch(bakeStartTs)
                                          .toString(Qt::DateFormat::TextDate);

        const qint64 bakeEndTs = obj.value("bake_end_time").toInteger();
        const qint64 denoiseStartTs = obj.value("denoise_start_time").toInteger();
        const qint64 denoiseEndTs = obj.value("denoise_start_time").toInteger();

        qint64 seconds = ((bakeEndTs - bakeStartTs) + (denoiseEndTs - denoiseStartTs)) / 1000;
        qint64 minutes = seconds / 60;
        const qint64 hours = minutes / 60;
        seconds %= 60;
        minutes %= 60;

        if (hours)
            sceneMetadata.bakeDuration = QString("%1h %2m %3s")
                                             .arg(hours)
                                             .arg(minutes)
                                             .arg(seconds);
        else if (minutes)
            sceneMetadata.bakeDuration = QString("%1m %2s").arg(minutes).arg(seconds);
        else
            sceneMetadata.bakeDuration = QString("%1s").arg(seconds);
    }

    if (obj.contains("options") && obj.value("options").isObject()) {
        const auto options = obj.value("options").toObject();
        sceneMetadata.options.reserve(options.size());
        for (auto it = options.begin(); it != options.end(); ++it) {
            QVariantMap row;
            row.insert(QStringLiteral("key"), it.key());

            const QJsonValue &val = it.value();
            if (val.isDouble()) {
                // Max 6 decimals
                row.insert(QStringLiteral("value"), QString::number(val.toDouble(), 'g', 6));
            } else {
                row.insert(QStringLiteral("value"), val.toVariant());
            }

            sceneMetadata.options.push_back(row);
        }
    }

    return sceneMetadata;
}

QVariantList LightmapViewerHelpers::processMetadata(const QString &key, const QVariantMap &map)
{
    if (map.isEmpty())
        return {};


    auto appendRow = [](QVariantList &map, const QString &key, const QString &value) {
        QVariantMap row;
        row.insert("key", key);
        row.insert("value", value);
        map.push_back(row);
    };

    QVariantList metadata;

    appendRow(metadata, "key", key);

    if (map.contains("height") && map.contains("width")) {

        appendRow(metadata, "Lightmap pixel size", QStringLiteral("%1x%2").
                                                         arg(map["width"].toString()).arg(
                                                             map["height"].toString()));
    }

    if (map.contains("mesh_key")) {
        appendRow(metadata, "meshKey", map["mesh_key"].toString());
    }

    return metadata;
}

void LightmapViewerHelpers::maskToBBGRColor(QByteArray &array, bool useAlpha)
{
    QVector<quint32> uints;
    uints.resize(array.size() / sizeof(quint32));
    memcpy(uints.data(), array.data(), array.size());

    quint32 maxN = 0;
    for (quint32 v : uints) {
        maxN = qMax(maxN, v);
    }
    for (quint32 &vRef : uints) {
        vRef = numberToBBGRColor(vRef, maxN, useAlpha);
    }
    memcpy(array.data(), uints.data(), array.size());
}

bool LightmapViewerHelpers::processLightmap(const QString &filename, bool print, bool extract)
{
    bool success = true;
    QSharedPointer<QSSGLightmapLoader> loader = QSSGLightmapLoader::open(filename);

    if (!loader) {
        return false;
    }

    if (extract) {
        QDir dir;
        for (const char *path : { "meshes", "images", "images/masks", "images/direct", "images/indirect", "images/final" }) {
            if (!dir.mkpath(path)) {
                qInfo() << "Failed to create folders";
                return false;
            }
        }
    }

    int numImagesSaved = 0;
    int numMeshesSaved = 0;

    QList<std::pair<QString, QSSGLightmapIODataTag>> keys = loader->getKeys();

    if (print)
        qInfo() << "-- Keys --";

    QVector<QString> meshKeys;

    QVariantMap sceneMetadata;
    for (const auto &[key, tag] : std::as_const(keys)) {
        if (tag == QSSGLightmapIODataTag::SceneMetadata) {
            sceneMetadata = loader->readMap(key, tag);
            continue;
        }
        QString tagString = LightmapViewerHelpers::lightmapTagToString(tag);

        if (print)
            qInfo() << key << ":" << tagString;

        if (tag == QSSGLightmapIODataTag::Mesh)
            meshKeys.push_back(key);
    }

    if (print)
        qInfo() << "-- Values --";

    // Extract meshes
    if (extract) {
        for (const QString &key : meshKeys) {
            const QByteArray meshData = loader->readData(key, QSSGLightmapIODataTag::Mesh);
            QFile meshFile(QString("meshes/" + key + ".mesh"));
            if (meshFile.open(QFile::WriteOnly)) {
                meshFile.write(meshData);
                meshFile.close();
                ++numMeshesSaved;
            } else {
                success = false;
                qInfo() << key << "->" << "FAILED TO WRITE";
            }
        }
    }

    for (const auto &[key, tag] : std::as_const(keys)) {
        if (tag != QSSGLightmapIODataTag::Metadata)
            continue;

        QVariantMap map = loader->readMap(key, tag);
        if (print) {
            qInfo() << key << ":";
            qInfo().noquote() << QJsonDocument(QJsonObject::fromVariantMap(map)).toJson(QJsonDocument::Indented).trimmed();
        }

        int width = map[QStringLiteral("width")].toInt();
        int height = map[QStringLiteral("height")].toInt();

        if (extract) {
            if (keys.contains(std::make_pair(key, QSSGLightmapIODataTag::Mask))) {
                QByteArray data = loader->readU32Image(key, QSSGLightmapIODataTag::Mask);
                maskToBBGRColor(data);
                QImage img = QImage(reinterpret_cast<uchar *>(data.data()), width, height, QImage::Format_RGBA8888);
                img.save(QString("images/masks/" + key + ".png"));
                ++numImagesSaved;
            }
            for (const auto &[texTag, dir] : std::array {
                         std::pair { QSSGLightmapIODataTag::Texture_Direct, QStringLiteral("direct") },
                         std::pair { QSSGLightmapIODataTag::Texture_Indirect, QStringLiteral("indirect") },
                         std::pair { QSSGLightmapIODataTag::Texture_Final, QStringLiteral("final") },
                 }) {
                if (!keys.contains(std::make_pair(key, texTag)))
                    continue;
                QByteArray data = loader->readF32Image(key, texTag);
                QImage img = QImage(reinterpret_cast<uchar *>(data.data()), width, height, QImage::Format_RGBA32FPx4);
                img.save(QString("images/" + dir + "/" + key + ".png"));
                ++numImagesSaved;
            }
        }
    }

    if (print && !sceneMetadata.isEmpty()) {
        const auto processed = processSceneMetadata(sceneMetadata);

        qInfo() << "-- Scene metadata --";
        qInfo() << QStringLiteral("Baked with Qt version: %1")
                       .arg(!processed.qtVersion.isEmpty() ? processed.qtVersion : "-");
        qInfo() << QStringLiteral("Bake initiated at: %1")
                       .arg(!processed.bakeStartTime.isEmpty() ? processed.bakeStartTime : "-");
        qInfo() << QStringLiteral("Bake duration: %1")
                       .arg(!processed.bakeDuration.isEmpty() ? processed.bakeDuration : "-");
        qInfo() << "Options used:";
        for (const auto &option : processed.options) {
            const auto map = option.toMap();
            qInfo() << map.value("key").toString() << ": " << map.value("value").toString();
        }
    }

    if (extract) {
        qInfo() << "Saved" << numImagesSaved << "images to 'images' and " << numMeshesSaved << "meshes to 'meshes'";
    }

    return success;
}
