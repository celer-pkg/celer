// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "lightmapfile.h"

#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QBuffer>
#include <QQmlEngine>

#include <QtQuick3DRuntimeRender/private/qssglightmapio_p.h>

#include "lightmapviewerhelpers.h"

LightmapFile::LightmapFile(QObject *parent) : QObject { parent } { }

QVariantList LightmapFile::dataList() const
{
    return m_dataList;
}

void LightmapFile::loadData()
{
    QSharedPointer<QSSGLightmapLoader> loader = QSSGLightmapLoader::open(m_source.toLocalFile());
    auto keys = loader ? loader->getKeys() : QList<std::pair<QString, QSSGLightmapIODataTag>>();
    m_dataList.clear();
    m_dataList.reserve(keys.size());

    QVariantMap sceneMetadata;
    std::unordered_map<QString, QStringList> keysReferencingMeshes;

    // First pass: find scene metadata and populate keysReferencingMesh

    for (const auto &[key, tag] : std::as_const(keys)) {
        if (tag == QSSGLightmapIODataTag::SceneMetadata) {
            sceneMetadata = loader->readMap(key, tag);
            continue;
        } else if (tag != QSSGLightmapIODataTag::Mesh) {
            continue;
        }

        keysReferencingMeshes[key] = keysReferencingMesh(key);
    }

    QSet<QString> meshRowAddedForGroup;

    // Second pass: populate datalist
    for (const auto &[key, tag] : std::as_const(keys)) {
        switch (tag) {
        case QSSGLightmapIODataTag::Mask:
        case QSSGLightmapIODataTag::Texture_Direct:
        case QSSGLightmapIODataTag::Texture_Indirect:
        case QSSGLightmapIODataTag::Texture_Final: {
            const QString tagString = LightmapViewerHelpers::lightmapTagToString(tag);

            QString meshKeyForThisImage;
            for (auto it = keysReferencingMeshes.cbegin(); it != keysReferencingMeshes.cend(); ++it) {
                if (it->second.contains(key)) {
                    meshKeyForThisImage = it->first;
                    break;
                }
            }

            // Meshes
            if (!meshRowAddedForGroup.contains(key)) {
                QVariantMap meshEntry;
                meshEntry["kind"]    = "mesh";
                meshEntry["key"]     = meshKeyForThisImage;
                meshEntry["display"] = meshKeyForThisImage;
                meshEntry["owner"] = key;
                m_dataList.push_back(meshEntry);
                meshRowAddedForGroup.insert(key);
            }

            // Images
            QVariantMap imgEntry;
            imgEntry["kind"]    = "image";
            imgEntry["key"]     = key;
            imgEntry["tag"]     = tagString;
            imgEntry["owner"] = key;
            imgEntry["display"] = tagString.split('_').last();

            m_dataList.push_back(imgEntry);
        } break;
        default:
            break;
        }
    }

    emit dataListChanged();

    const auto processed = LightmapViewerHelpers::processSceneMetadata(sceneMetadata);
    m_qtVersion = processed.qtVersion;
    m_bakeStart = processed.bakeStartTime;
    m_bakeDuration = processed.bakeDuration;
    m_options = processed.options;
}

QUrl LightmapFile::source() const
{
    return m_source;
}

void LightmapFile::setSource(const QUrl &newSource)
{
    if (m_source == newSource)
        return;
    m_source = newSource;
    emit sourceChanged();
}

QVariantList LightmapFile::metadataFor(const QVariant &selectedEntry)
{
    const QString key = selectedEntry.toMap()["owner"].toString();

    if (key.isEmpty())
        return {};

    auto loader = QSSGLightmapLoader::open(m_source.toLocalFile());
    if (!loader)
        return {};

    const auto metadata = loader->readMap(key, QSSGLightmapIODataTag::Metadata);
    if (metadata.isEmpty())
        return {};

    return LightmapViewerHelpers::processMetadata(key, metadata);
}

QStringList LightmapFile::keysReferencingMesh(const QString &meshKey) const
{
    QStringList out;
    auto loader = QSSGLightmapLoader::open(m_source.toLocalFile());
    if (!loader)
        return out;

    const auto keys = loader->getKeys();
    for (const auto &kp : keys) {
        if (kp.second != QSSGLightmapIODataTag::Metadata)
            continue;
        const QVariantMap md = loader->readMap(kp.first, QSSGLightmapIODataTag::Metadata);
        if (md.value(QStringLiteral("mesh_key")).toString() == meshKey)
            out << kp.first;
    }
    out.removeDuplicates();
    return out;
}

QQuick3DTextureData *LightmapFile::textureDataFor(const QString &key, quint32 tag)
{
    if (key.isEmpty())
        return nullptr;


    auto loader = QSSGLightmapLoader::open(m_source.toLocalFile());
    if (!loader)
        return nullptr;

    const QVariantMap md = loader->readMap(key, QSSGLightmapIODataTag::Metadata);
    const int w = md.value(QStringLiteral("width")).toInt();
    const int h = md.value(QStringLiteral("height")).toInt();
    if (w <= 0 || h <= 0)
        return nullptr;

    const QSSGLightmapIODataTag actualTag = static_cast<QSSGLightmapIODataTag>(tag);

    QByteArray src;

    if (actualTag == QSSGLightmapIODataTag::Mask) {
        QByteArray src = loader->readU32Image(key, actualTag);
        if (src.size() != w * h * int(sizeof(quint32)))
            return nullptr;

        LightmapViewerHelpers::maskToBBGRColor(src, false);
        QByteArray dst;
        dst.resize(src.size());
        const int stride = w * 4;
        const uchar* s = reinterpret_cast<const uchar*>(src.constData());
        uchar* d = reinterpret_cast<uchar*>(dst.data());
        for (int y = 0; y < h; ++y) {
            const int srcRow = y;
            const int dstRow = (h - 1) - y;
            memcpy(d + dstRow * stride, s + srcRow * stride, size_t(stride));
        }

        auto *tex = new QQuick3DTextureData;
        QQmlEngine::setObjectOwnership(tex, QQmlEngine::CppOwnership);
        tex->setSize(QSize(w, h));
        tex->setFormat(QQuick3DTextureData::RGBA8);
        tex->setHasTransparency(false);
        tex->setTextureData(dst);
        return tex;
    }

    if (actualTag == QSSGLightmapIODataTag::Texture_Final
        || actualTag == QSSGLightmapIODataTag::Texture_Direct
        || actualTag == QSSGLightmapIODataTag::Texture_Indirect) {

        src = loader->readF32Image(key, actualTag);

        if (src.size() != w * h * int(4 * sizeof(float)))
            return nullptr;

        QByteArray dst;
        dst.resize(src.size());
        const float *srcF = reinterpret_cast<const float *>(src.constData());
        float *dstF = reinterpret_cast<float *>(dst.data());
        const int strideFloats = w * 4;
        for (int y = 0; y < h; ++y) {
            const int srcRow = y;
            const int dstRow = (h - 1) - y;
            memcpy(dstF + dstRow * strideFloats,
                   srcF + srcRow * strideFloats,
                   size_t(strideFloats) * sizeof(float));
        }

        auto *tex = new QQuick3DTextureData;
        QQmlEngine::setObjectOwnership(tex, QQmlEngine::CppOwnership);
        tex->setSize(QSize(w, h));
        tex->setFormat(QQuick3DTextureData::RGBA32F);
        tex->setHasTransparency(false);
        tex->setTextureData(dst);
        return tex;
    }

    return nullptr;
}

QVariantList LightmapFile::texturesAvailableFor(const QString &key) const
{
    if (key.isEmpty())
        return {};

    auto loader = QSSGLightmapLoader::open(m_source.toLocalFile());
    if (!loader)
        return {};

    QVariantList out;

    const auto keys = loader->getKeys();
    for (const auto &kp : keys) {
        if (kp.first != key) continue;

        const QSSGLightmapIODataTag tag = kp.second;
        if (tag== QSSGLightmapIODataTag::Mask || tag== QSSGLightmapIODataTag::Texture_Direct
            || tag == QSSGLightmapIODataTag::Texture_Indirect || tag == QSSGLightmapIODataTag::Texture_Final) {
            out << QVariantMap { {"name", LightmapViewerHelpers::lightmapTagToString(tag).replace("Texture_", "")},
                                 {"value", static_cast<quint32>(tag)}
            };
        }
    }
    return out;
}
