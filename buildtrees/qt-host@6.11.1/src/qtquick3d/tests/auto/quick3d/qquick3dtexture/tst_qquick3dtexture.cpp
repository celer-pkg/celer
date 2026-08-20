// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QtQuick/qquickitem.h>
#include <QtQuick3D/private/qquick3dtexture_p.h>
#include <QtQuick3D/QQuick3DTextureData>
#include <QtQuick3D/private/qquick3dviewport_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderimage_p.h>

class tst_QQuick3DTexture : public QObject
{
    Q_OBJECT

    // Work-around to get access to updateSpatialNode
    class Texture : public QQuick3DTexture
    {
    public:
        using QQuick3DTexture::updateSpatialNode;
    };

private slots:
    void testSetSource();
    void testSetSourceItem();
    void testMappingAndTilingModes();
    void testSamplerFilteringModes();
    void testTransformations();
    void testTextureData();
};

void tst_QQuick3DTexture::testSetSource()
{
    Texture texture;
    std::unique_ptr<QSSGRenderImage> node;

    node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
    QVERIFY(node);
    // check that we get the same node out when passing in the old one
    QCOMPARE(node.get(), texture.updateSpatialNode(node.get()));

    QSignalSpy spy(&texture, SIGNAL(sourceChanged()));
    QCOMPARE(spy.size(), 0);

    const QUrl fileWithScheme {QString::fromLatin1("file:path/to/resource")};
    const QString expectedPath {QString::fromLatin1("path/to/resource")};
    texture.setSource(fileWithScheme);
    node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
    QCOMPARE(expectedPath, node->m_imagePath.path());
    QCOMPARE(spy.size(), 1);

    // Same url again
    texture.setSource(fileWithScheme);
    QCOMPARE(spy.size(), 1);
}

void tst_QQuick3DTexture::testSetSourceItem()
{
    Texture texture;

    QQuick3DViewport view3D;
    QQuickItem item;
    item.setParentItem(&view3D);

    qRegisterMetaType<QQuickItem *>();
    QSignalSpy spy(&texture, SIGNAL(sourceItemChanged()));
    QCOMPARE(spy.size(), 0);

    QVERIFY(!texture.sourceItem());
    texture.setSourceItem(&item);
    QVERIFY(texture.sourceItem());
    QCOMPARE(spy.size(), 1);

    // Same item again
    texture.setSourceItem(&item);
    QCOMPARE(spy.size(), 1);
}

void tst_QQuick3DTexture::testMappingAndTilingModes()
{
    Texture texture;
    std::unique_ptr<QSSGRenderImage> node;

    // This test relies on the two different enums (QQ3DT::MappingModes & QSSGRI::MappingModes)
    // having the same values
    for (const auto mappingMode : {QQuick3DTexture::UV, QQuick3DTexture::LightProbe, QQuick3DTexture::Environment })
    {
        texture.setMappingMode(mappingMode);
        node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
        QCOMPARE(int(mappingMode), int(node->m_mappingMode));
    }

    // This also relies on the two enums (QQ3DT::TilingMode & QSSGRenderTextureCoordOp)
    // having the same values
    for (const auto tilingMode : {QQuick3DTexture::ClampToEdge, QQuick3DTexture::MirroredRepeat, QQuick3DTexture::Repeat })
    {
        texture.setHorizontalTiling(tilingMode);
        node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
        QCOMPARE(tilingMode, int(node->m_horizontalTilingMode));

        texture.setVerticalTiling(tilingMode);
        node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
        QCOMPARE(tilingMode, int(node->m_verticalTilingMode));
    }
}

void tst_QQuick3DTexture::testSamplerFilteringModes()
{
    Texture texture;
    std::unique_ptr<QSSGRenderImage> node;

    for (const auto filterMode : {QQuick3DTexture::None, QQuick3DTexture::Linear, QQuick3DTexture::Nearest}) {
        texture.setMinFilter(filterMode);
        node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
        QCOMPARE(int(filterMode), int(node->m_minFilterType));

        texture.setMagFilter(filterMode);
        node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
        QCOMPARE(int(filterMode), int(node->m_magFilterType));

        texture.setMipFilter(filterMode);
        node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
        QCOMPARE(int(filterMode), int(node->m_mipFilterType));
    }

    // generate mipmaps
    node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
    QCOMPARE(node->m_generateMipmaps, false);

    texture.setGenerateMipmaps(true);
    node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
    QCOMPARE(node->m_generateMipmaps, true);
}

void tst_QQuick3DTexture::testTransformations()
{
    Texture texture;
    std::unique_ptr<QSSGRenderImage> node;

    const float scaleU = 0.7f;
    texture.setScaleU(scaleU);
    node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
    QCOMPARE(scaleU, node->m_scale.x());

    const float scaleV = 0.36f;
    texture.setScaleV(scaleV);
    node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
    QCOMPARE(scaleV, node->m_scale.y());

    const float rotationUV = 0.38f;
    texture.setRotationUV(rotationUV);
    node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
    QCOMPARE(rotationUV, node->m_rotation);

    const float positionU = 0.39f;
    texture.setPositionU(positionU);
    node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
    QCOMPARE(positionU, node->m_position.x());

    const float positionV = 0.40f;
    texture.setPositionV(positionV);
    node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
    QCOMPARE(positionV, node->m_position.y());

    const float pivotU = 0.41f;
    texture.setPivotU(pivotU);
    node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
    QCOMPARE(pivotU, node->m_pivot.x());

    const float pivotV = 0.42f;
    texture.setPivotV(pivotV);
    node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
    QCOMPARE(pivotV, node->m_pivot.y());
}

void tst_QQuick3DTexture::testTextureData()
{
    Texture texture;
    QQuick3DTextureData textureData;
    std::unique_ptr<QSSGRenderImage> node;

    node.reset(static_cast<QSSGRenderImage *>(texture.updateSpatialNode(nullptr)));
    QVERIFY(node);

    // check that we get the same node out when passing in the old one
    QCOMPARE(node.get(), texture.updateSpatialNode(node.get()));

    // No textureData by default
    QSignalSpy spy(&texture, SIGNAL(textureDataChanged()));
    QCOMPARE(spy.size(), 0);

    // Set textureData
    QVERIFY(!texture.textureData());
    texture.setTextureData(&textureData);
    QVERIFY(texture.textureData());
    QCOMPARE(spy.size(), 1);

    // Same textureData again
    texture.setTextureData(&textureData);
    QCOMPARE(spy.size(), 1);
}

QTEST_APPLESS_MAIN(tst_QQuick3DTexture)
#include "tst_qquick3dtexture.moc"
