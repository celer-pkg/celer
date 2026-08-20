// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "lightmapimageprovider.h"

#include "lightmapviewerhelpers.h"

#include <QUrlQuery>
#include <QtQuick3DRuntimeRender/private/qssglightmapio_p.h>

LightmapImageProvider::LightmapImageProvider() : QQuickImageProvider(QQuickImageProvider::Image)
{
    // Generate error image
    constexpr int width = 100;
    m_errorImage = QImage(QSize(width, width), QImage::Format::Format_RGB888);
    m_errorImage.fill(QColorConstants::White);
    for (int i = 0; i < width; i++) {
        m_errorImage.setPixelColor(i, i, QColorConstants::Red);
        m_errorImage.setPixelColor(width - i - 1, i, QColorConstants::Red);
    }
}

enum class LightmapDataType { Unset, F32, U32 };

QImage LightmapImageProvider::requestImage(const QString &id, QSize *size, const QSize & /*requestedSize*/)
{
    if (size)
        *size = QSize(100, 100);
    const QUrl url = QUrl("lightmap://?" + id, QUrl::StrictMode);
    const QUrlQuery query(url);
    const QString key = query.queryItemValue("key");
    const QString tagStr = query.queryItemValue("tag");
    QSSGLightmapIODataTag tag = LightmapViewerHelpers::stringToLightmapTag(tagStr);

    const auto filePath = QUrl{query.queryItemValue("file")};
    const bool useAlpha = query.queryItemValue("alpha") == QStringLiteral("true");

    LightmapDataType dataType = LightmapDataType::Unset;
    switch (tag) {
    case QSSGLightmapIODataTag::Mask:
        dataType = LightmapDataType::U32;
        break;
    case QSSGLightmapIODataTag::Texture_Final:
    case QSSGLightmapIODataTag::Texture_Direct:
    case QSSGLightmapIODataTag::Texture_Indirect:
        dataType = LightmapDataType::F32;
        break;
    case QSSGLightmapIODataTag::Metadata:
    case QSSGLightmapIODataTag::Mesh:
    case QSSGLightmapIODataTag::Unset:
    default:
        return {};
    }

    QSharedPointer<QSSGLightmapLoader> loader = QSSGLightmapLoader::open(filePath.toLocalFile());

    if (!loader)
        return m_errorImage;

    QVariantMap metadata = loader->readMap(key, QSSGLightmapIODataTag::Metadata);
    if (metadata.isEmpty())
        return m_errorImage;
    bool ok = false;
    const int width = metadata[QStringLiteral("width")].toInt(&ok);
    if (!ok)
        return m_errorImage;
    const int height = metadata[QStringLiteral("height")].toInt(&ok);
    if (!ok)
        return m_errorImage;

    QImage::Format format = QImage::Format_Invalid;
    QByteArray array;
    if (dataType == LightmapDataType::U32) {
        array = loader->readU32Image(key, tag);
        if (array.size() != qsizetype(sizeof(quint32) * width * height))
            return m_errorImage;
        format = QImage::Format_RGBA8888;
        LightmapViewerHelpers::maskToBBGRColor(array, useAlpha);
    } else if (dataType == LightmapDataType::F32) {
        array = loader->readF32Image(key, tag);
        if (array.size() != qsizetype(4 * sizeof(float) * width * height))
            return m_errorImage;
        if (!useAlpha) {
            std::array<float, 4> *rgbas = reinterpret_cast<std::array<float, 4> *>(array.data());
            for (int i = 0, n = array.size() / (4 * sizeof(float)); i < n; ++i)
                rgbas[i][3] = 1.0f;
        }
        format = QImage::Format_RGBA32FPx4;
    } else {
        return m_errorImage;
    }

    // Everything should be OK here
    if (size)
        *size = QSize(width, height);
    QImage result(width, height, format);
    memcpy(result.bits(), array.data(), array.size());
    return result;
}
