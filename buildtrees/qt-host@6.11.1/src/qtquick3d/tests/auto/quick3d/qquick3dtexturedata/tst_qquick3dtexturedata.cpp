// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QtQuick3D/QQuick3DTextureData>
#include <QtQuick3D/private/qquick3dobject_p.h>
#include <QtQuick3D/private/qquick3dtexture_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendertexturedata_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderimage_p.h>

class tst_QQuick3DTextureData : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testProperties();
};

namespace {
    QSSGRenderTextureFormat::Format convertToBackendFormat(QQuick3DTextureData::Format format)
    {
        switch (format) {
        case QQuick3DTextureData::None:
        case QQuick3DTextureData::RGBA8:
            return QSSGRenderTextureFormat::RGBA8;
        case QQuick3DTextureData::RGBA16F:
            return QSSGRenderTextureFormat::RGBA16F;
        case QQuick3DTextureData::RGBA32F:
            return QSSGRenderTextureFormat::RGBA32F;
        case QQuick3DTextureData::RGBE8:
            return QSSGRenderTextureFormat::RGBE8;
        case QQuick3DTextureData::R8:
            return QSSGRenderTextureFormat::R8;
        case QQuick3DTextureData::R16:
            return QSSGRenderTextureFormat::R16;
        case QQuick3DTextureData::R16F:
            return QSSGRenderTextureFormat::R16F;
        case QQuick3DTextureData::R32F:
            return QSSGRenderTextureFormat::R32F;
        case QQuick3DTextureData::BC1:
            return QSSGRenderTextureFormat::BC1;
        case QQuick3DTextureData::BC2:
            return QSSGRenderTextureFormat::BC2;
        case QQuick3DTextureData::BC3:
            return QSSGRenderTextureFormat::BC3;
        case QQuick3DTextureData::BC4:
            return QSSGRenderTextureFormat::BC4;
        case QQuick3DTextureData::BC5:
            return QSSGRenderTextureFormat::BC5;
        case QQuick3DTextureData::BC6H:
            return QSSGRenderTextureFormat::BC6H;
        case QQuick3DTextureData::BC7:
            return QSSGRenderTextureFormat::BC7;
        case QQuick3DTextureData::DXT1_RGBA:
            return QSSGRenderTextureFormat::RGBA_DXT1;
        case QQuick3DTextureData::DXT1_RGB:
            return QSSGRenderTextureFormat::RGB_DXT1;
        case QQuick3DTextureData::DXT3_RGBA:
            return QSSGRenderTextureFormat::RGBA_DXT3;
        case QQuick3DTextureData::DXT5_RGBA:
            return QSSGRenderTextureFormat::RGBA_DXT5;
        case QQuick3DTextureData::ETC2_RGB8:
            return QSSGRenderTextureFormat::RGB8_ETC2;
        case QQuick3DTextureData::ETC2_RGB8A1:
            return QSSGRenderTextureFormat::RGB8_PunchThrough_Alpha1_ETC2;
        case QQuick3DTextureData::ETC2_RGBA8:
            return QSSGRenderTextureFormat::RGBA8_ETC2_EAC;
        case QQuick3DTextureData::ASTC_4x4:
            return QSSGRenderTextureFormat::RGBA_ASTC_4x4;
        case QQuick3DTextureData::ASTC_5x4:
            return QSSGRenderTextureFormat::RGBA_ASTC_5x4;
        case QQuick3DTextureData::ASTC_5x5:
            return QSSGRenderTextureFormat::RGBA_ASTC_5x5;
        case QQuick3DTextureData::ASTC_6x5:
            return QSSGRenderTextureFormat::RGBA_ASTC_6x5;
        case QQuick3DTextureData::ASTC_6x6:
            return QSSGRenderTextureFormat::RGBA_ASTC_6x6;
        case QQuick3DTextureData::ASTC_8x5:
            return QSSGRenderTextureFormat::RGBA_ASTC_8x5;
        case QQuick3DTextureData::ASTC_8x6:
            return QSSGRenderTextureFormat::RGBA_ASTC_8x6;
        case QQuick3DTextureData::ASTC_8x8:
            return QSSGRenderTextureFormat::RGBA_ASTC_8x8;
        case QQuick3DTextureData::ASTC_10x5:
            return QSSGRenderTextureFormat::RGBA_ASTC_10x5;
        case QQuick3DTextureData::ASTC_10x6:
            return QSSGRenderTextureFormat::RGBA_ASTC_10x6;
        case QQuick3DTextureData::ASTC_10x8:
            return QSSGRenderTextureFormat::RGBA_ASTC_10x8;
        case QQuick3DTextureData::ASTC_10x10:
            return QSSGRenderTextureFormat::RGBA_ASTC_10x10;
        case QQuick3DTextureData::ASTC_12x10:
            return QSSGRenderTextureFormat::RGBA_ASTC_12x10;
        case QQuick3DTextureData::ASTC_12x12:
            return QSSGRenderTextureFormat::RGBA_ASTC_12x12;
        default:
            return QSSGRenderTextureFormat::RGBA8;
        }
    }
}

void tst_QQuick3DTextureData::testProperties()
{
    QQuick3DTextureData textureData;
    auto *node = static_cast<QSSGRenderTextureData *>(QQuick3DObjectPrivate::updateSpatialNode(&textureData, nullptr));
    const auto originalNode = node; // for comparisons later...
    QVERIFY(node);

    // test for expected defaults
    QVERIFY(textureData.textureData().isNull());
    QVERIFY(textureData.size().isEmpty());
    QVERIFY(!textureData.hasTransparency());
    QVERIFY(textureData.format() == QQuick3DTextureData::RGBA8);

    const QSize size(16, 16);
    textureData.setSize(size);
    node = static_cast<QSSGRenderTextureData *>(QQuick3DObjectPrivate::updateSpatialNode(&textureData, node));
    QCOMPARE(originalNode, node);
    QCOMPARE(size, node->size());

    const bool isTransparent = true;
    textureData.setHasTransparency(isTransparent);
    node = static_cast<QSSGRenderTextureData *>(QQuick3DObjectPrivate::updateSpatialNode(&textureData, node));
    QCOMPARE(originalNode, node);
    QCOMPARE(isTransparent, node->hasTransparency());

    const QByteArray data(16*16*4, 'x');
    textureData.setTextureData(data);
    node = static_cast<QSSGRenderTextureData *>(QQuick3DObjectPrivate::updateSpatialNode(&textureData, node));
    QCOMPARE(originalNode, node);
    QCOMPARE(data, node->textureData());

    auto metaEnum = QMetaEnum::fromType<QQuick3DTextureData::Format>();
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        const auto format = QQuick3DTextureData::Format(metaEnum.value(i));
        textureData.setFormat(format);
        node = static_cast<QSSGRenderTextureData *>(QQuick3DObjectPrivate::updateSpatialNode(&textureData, node));
        QCOMPARE(originalNode, node);
        QCOMPARE(int(convertToBackendFormat(format)), int(node->format().format));
    }
}

QTEST_APPLESS_MAIN(tst_QQuick3DTextureData)
#include "tst_qquick3dtexturedata.moc"
