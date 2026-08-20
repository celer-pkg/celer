// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>

#include <QtQuick3D/private/qquick3dscenemanager_p.h>
#include <QtQuick3D/private/qquick3ddefaultmaterial_p.h>
#include <QtQuick3D/private/qquick3dcustommaterial_p.h>
#include <QtQuick3D/private/qquick3dprincipledmaterial_p.h>
#include <QtQuick3D/private/qquick3dspecularglossymaterial_p.h>
#include <QtQuick3D/private/qquick3dviewport_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterial_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercustommaterial_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderimage_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>

class tst_QQuick3DMaterials : public QObject
{
    Q_OBJECT

    // Work-around to get access to updateSpatialNode
    class Material : public QQuick3DDefaultMaterial
    {
    public:
        using QQuick3DMaterial::updateSpatialNode;
    };

    class CustomMaterial : public QQuick3DCustomMaterial
    {
    public:
        using QQuick3DCustomMaterial::updateSpatialNode;
    };

    class PrincipledMaterial : public QQuick3DPrincipledMaterial
    {
    public:
        using QQuick3DPrincipledMaterial::updateSpatialNode;
    };

    class SpecularGlossyMaterial : public QQuick3DSpecularGlossyMaterial
    {
    public:
        using QQuick3DSpecularGlossyMaterial::updateSpatialNode;
    };

    class Texture : public QQuick3DTexture
    {
    public:
        using QQuick3DTexture::updateSpatialNode;
    };

private slots:
    void testDefaultProperties();
    void testDefaultTextures();
    void testDefaultEnums();
    void testPrincipledProperties();
    void testPrincipledTextures();
    void testPrincipledEnums();
    void testSpecularGlossyProperties();
    void testSpecularGlossyTextures();
    void testSpecularGlossyEnums();
    void testCustomMaterials();
};

void tst_QQuick3DMaterials::testDefaultProperties()
{
    Material material;
    auto node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(nullptr));
    const auto originalNode = node; // for comparisons later...
    QVERIFY(node);

    const float opacity = 0.4f;
    material.setOpacity(opacity);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QCOMPARE(material.opacity(), node->opacity);

    const float specularAmount = 10.0f;
    material.setSpecularAmount(specularAmount);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QCOMPARE(material.specularAmount(), node->specularAmount);

    const float emissiveFactor = 0.5f;
    material.setEmissiveFactor(QVector3D(emissiveFactor, emissiveFactor, emissiveFactor));
    material.setLighting(QQuick3DDefaultMaterial::FragmentLighting);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    auto expectedEmissiceColor = material.emissiveFactor();
    QCOMPARE(expectedEmissiceColor, node->emissiveColor);

    const float bumpAmount = 0.3f;
    material.setBumpAmount(bumpAmount);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QCOMPARE(material.bumpAmount(), node->bumpAmount);

    material.setVertexColorsEnabled(true);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(material.vertexColorsEnabled(), node->vertexColorsEnabled);
    material.setVertexColorsEnabled(false);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(material.vertexColorsEnabled(), node->vertexColorsEnabled);

    QColor color1("#12345678");
    QVector4D color1Vec4 = QSSGUtils::color::sRGBToLinear(color1);
    material.setDiffuseColor(color1);
    material.setEmissiveFactor(color1Vec4.toVector3D());
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QCOMPARE(color1Vec4, node->color);
    // Note: emissiveColor doesn't contain alpha
    QCOMPARE(QVector3D(color1Vec4), node->emissiveColor);

}

void tst_QQuick3DMaterials::testDefaultTextures()
{
    Material material;
    auto node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(nullptr));
    const auto originalNode = node; // for comparisons later...
    QVERIFY(node);

    Texture texture1;
    texture1.setSource(QUrl(":/Built_with_Qt.png"));
    Texture texture2;
    texture2.setSource(QUrl(":/Built_with_Qt_2.png"));

    QQuick3DSceneManager sceneManager;
    sceneManager.updateDirtyResource(&texture1);
    sceneManager.updateDirtyResource(&texture2);

    // Diffusemaps
    material.setDiffuseMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QVERIFY(texture1.getRenderImage());
    QCOMPARE(texture1.getRenderImage(), node->colorMap);

    // Specularmaps
    QVERIFY(!node->specularMap);
    QVERIFY(!node->specularReflection);
    QVERIFY(!material.specularMap());
    QVERIFY(!material.specularReflectionMap());
    material.setSpecularMap(&texture2);
    material.setSpecularReflectionMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.specularMap());
    QCOMPARE(material.specularMap()->getRenderImage(), node->specularMap);
    QVERIFY(material.specularReflectionMap());
    QCOMPARE(material.specularReflectionMap()->getRenderImage(), node->specularReflection);

    // Bumpmap
    QVERIFY(!node->bumpMap);
    QVERIFY(!material.bumpMap());
    material.setBumpMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.bumpMap());
    QCOMPARE(material.bumpMap()->getRenderImage(), node->bumpMap);

    // Opacitymap
    QVERIFY(!node->opacityMap);
    QVERIFY(!material.opacityMap());
    material.setOpacityMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.opacityMap());
    QCOMPARE(material.opacityMap()->getRenderImage(), node->opacityMap);
    QCOMPARE(material.opacityMap()->getRenderImage(), material.bumpMap()->getRenderImage());
}

void tst_QQuick3DMaterials::testDefaultEnums()
{
    Material material;
    auto node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(nullptr));
    QVERIFY(node);

    // These test rely on the different enums having the same values
    auto lightModes = { QQuick3DDefaultMaterial::Lighting::NoLighting,
                        QQuick3DDefaultMaterial::Lighting::FragmentLighting };
    for (const auto lightMode : lightModes)
    {
        material.setLighting(lightMode);
        node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(nullptr));
        QCOMPARE(int(material.lighting()), int(node->lighting));
    }

    auto blendModes = { QQuick3DDefaultMaterial::BlendMode::SourceOver,
                        QQuick3DDefaultMaterial::BlendMode::Screen,
                        QQuick3DDefaultMaterial::BlendMode::Multiply };
    for (const auto blendMode : blendModes)
    {
        material.setBlendMode(blendMode);
        node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(nullptr));
        QCOMPARE(int(material.blendMode()), int(node->blendMode));
    }

    auto specularModes = { QQuick3DDefaultMaterial::SpecularModel::Default,
                           QQuick3DDefaultMaterial::SpecularModel::KGGX};
    for (const auto specularMode : specularModes)
    {
        material.setSpecularModel(specularMode);
        node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(nullptr));
        QCOMPARE(int(material.specularModel()), int(node->specularModel));
    }

    auto depthDrawModes = {
        QQuick3DMaterial::DepthDrawMode::OpaqueOnlyDepthDraw,
        QQuick3DMaterial::DepthDrawMode::AlwaysDepthDraw,
        QQuick3DMaterial::DepthDrawMode::NeverDepthDraw,
        QQuick3DMaterial::DepthDrawMode::OpaquePrePassDepthDraw
    };

    for (const auto depthDrawMode : depthDrawModes) {
        material.setDepthDrawMode(depthDrawMode);
        node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(nullptr));
        QCOMPARE(int(material.depthDrawMode()), int(node->depthDrawMode));
    }
}

void tst_QQuick3DMaterials::testPrincipledProperties()
{
    PrincipledMaterial material;
    auto node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(nullptr));
    const auto originalNode = node; // for comparisons later...
    QVERIFY(node);

    const float metalness = 0.1f;
    material.setMetalness(metalness);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QCOMPARE(metalness, material.metalness());
    QCOMPARE(metalness, node->metalnessAmount);

    // Note: metalness needs to be disabled for specularAmount and specularTint
    // to affect backend.
    material.setMetalness(0.0f);

    const float specularAmount = 0.2f;
    material.setSpecularAmount(specularAmount);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(specularAmount, material.specularAmount());
    QCOMPARE(specularAmount, node->specularAmount);

    const float specularTint = 0.3f;
    material.setSpecularTint(specularTint);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(specularTint, material.specularTint());
    // Note: Backend uses vector3D for specularTint
    QCOMPARE(specularTint, node->specularTint.y());

    const float roughness = 0.4f;
    material.setRoughness(roughness);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(roughness, material.roughness());
    QCOMPARE(roughness, node->specularRoughness);

    const float opacity = 0.7f;
    material.setOpacity(opacity);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(opacity, material.opacity());
    QCOMPARE(opacity, node->opacity);

    const float normalStrength = 0.8f;
    material.setNormalStrength(normalStrength);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(normalStrength, material.normalStrength());
    QCOMPARE(normalStrength, node->bumpAmount);

    const float occlusionAmount = 0.9f;
    material.setOcclusionAmount(occlusionAmount);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(occlusionAmount, material.occlusionAmount());
    QCOMPARE(occlusionAmount, node->occlusionAmount);

    const float alphaCutoff = 0.1f;
    material.setAlphaCutoff(alphaCutoff);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(alphaCutoff, material.alphaCutoff());
    QCOMPARE(alphaCutoff, node->alphaCutoff);

    QColor color1("#12345678");
    QVector4D color1Vec4 = QSSGUtils::color::sRGBToLinear(color1);
    QColor color2("#cccccccc");
    QVector3D color2Vec3 = QSSGUtils::color::sRGBToLinear(color2).toVector3D();
    material.setBaseColor(color1);
    material.setEmissiveFactor(color2Vec3);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QCOMPARE(color1Vec4, node->color);
    // Note: emissiveColor doesn't contain alpha
    QCOMPARE(color2Vec3, node->emissiveColor);

    const float heightAmount = 0.2f;
    material.setHeightAmount(heightAmount);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(heightAmount, material.heightAmount());
    QCOMPARE(heightAmount, node->heightAmount);

    const int minHeightMapSamples = 11;
    material.setMinHeightMapSamples(minHeightMapSamples);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(minHeightMapSamples, material.minHeightMapSamples());
    QCOMPARE(minHeightMapSamples, node->minHeightSamples);

    const int maxHeightMapSamples = 33;
    material.setMaxHeightMapSamples(maxHeightMapSamples);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(maxHeightMapSamples, material.maxHeightMapSamples());
    QCOMPARE(maxHeightMapSamples, node->maxHeightSamples);

    QVERIFY(!node->isClearcoatEnabled());
    float clearcoatAmount = 0.3f;
    material.setClearcoatAmount(clearcoatAmount);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(clearcoatAmount, material.clearcoatAmount());
    QCOMPARE(clearcoatAmount, node->clearcoatAmount);
    QVERIFY(node->isClearcoatEnabled());
    clearcoatAmount = 0.0f;
    material.setClearcoatAmount(clearcoatAmount);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(!node->isClearcoatEnabled());

    const float clearcoatRoughnessAmount = 0.4f;
    material.setClearcoatRoughnessAmount(clearcoatRoughnessAmount);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(clearcoatRoughnessAmount, material.clearcoatRoughnessAmount());
    QCOMPARE(clearcoatRoughnessAmount, node->clearcoatRoughnessAmount);

    QVERIFY(material.metalness() == 0.0f);
    QVERIFY(node->metalnessAmount == 0.0f);
    QVERIFY(!node->isTransmissionEnabled());
    const float transmissionFactor = 0.5f;
    material.setTransmissionFactor(transmissionFactor);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(transmissionFactor, material.transmissionFactor());
    QCOMPARE(transmissionFactor, node->transmissionFactor);
    QVERIFY(node->isTransmissionEnabled());
    material.setTransmissionFactor(0.0f);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(!node->isTransmissionEnabled());

    const float thicknessFactor = 0.6f;
    material.setThicknessFactor(thicknessFactor);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(thicknessFactor, material.thicknessFactor());
    QCOMPARE(thicknessFactor, node->thicknessFactor);
    const float attenuationDistance = 0.7f;
    material.setAttenuationDistance(attenuationDistance);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(attenuationDistance, material.attenuationDistance());
    QCOMPARE(attenuationDistance, node->attenuationDistance);
    const QColor attenuationColor = QColor(Qt::green);
    const QVector3D attenuationColorVector = QSSGUtils::color::sRGBToLinear(attenuationColor).toVector3D();
    material.setAttenuationColor(attenuationColor);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(attenuationColor, material.attenuationColor());
    QCOMPARE(attenuationColorVector, node->attenuationColor);

    const float ior = 1.45f;
    material.setIndexOfRefraction(ior);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(ior, material.indexOfRefraction());
    QCOMPARE(ior, node->ior);
}

void tst_QQuick3DMaterials::testPrincipledTextures()
{
    PrincipledMaterial material;
    auto node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(nullptr));
    const auto originalNode = node; // for comparisons later...
    QVERIFY(node);

    Texture texture1;
    texture1.setSource(QUrl(":/Built_with_Qt.png"));
    Texture texture2;
    texture2.setSource(QUrl(":/Built_with_Qt_2.png"));

    QQuick3DSceneManager sceneManager;
    sceneManager.updateDirtyResource(&texture1);
    sceneManager.updateDirtyResource(&texture2);

    // BasecolorMap
    QVERIFY(!material.baseColorMap());
    material.setBaseColorMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QVERIFY(material.baseColorMap());
    QVERIFY(texture1.getRenderImage());
    QCOMPARE(texture1.getRenderImage(), node->colorMap);

    // MetalnessMap
    material.setMetalnessMap(&texture2);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QVERIFY(texture2.getRenderImage());
    QCOMPARE(texture2.getRenderImage(), node->metalnessMap);

    // SpecularMap
    // Note: metalness needs to be disabled for setSpecularMap to affect backend.
    material.setMetalness(0.0f);
    QVERIFY(!material.specularMap());
    material.setSpecularMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.specularMap());
    QCOMPARE(texture1.getRenderImage(), node->specularMap);

    // RoughnessMap
    QVERIFY(!material.roughnessMap());
    material.setRoughnessMap(&texture2);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.roughnessMap());
    QCOMPARE(texture2.getRenderImage(), node->roughnessMap);

    // EmissiveMap
    QVERIFY(!material.emissiveMap());
    material.setEmissiveMap(&texture2);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.emissiveMap());
    QCOMPARE(texture2.getRenderImage(), node->emissiveMap);

    // OpacityMap
    QVERIFY(!material.opacityMap());
    material.setOpacityMap(&texture2);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.opacityMap());
    QCOMPARE(texture2.getRenderImage(), node->opacityMap);

    // NormalMap
    QVERIFY(!material.normalMap());
    material.setNormalMap(&texture2);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.normalMap());
    QCOMPARE(texture2.getRenderImage(), node->normalMap);

    // SpecularReflectionMap
    QVERIFY(!material.specularReflectionMap());
    material.setSpecularReflectionMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.specularReflectionMap());
    QCOMPARE(texture1.getRenderImage(), node->specularReflection);

    // OcclusionMap
    QVERIFY(!material.occlusionMap());
    material.setOcclusionMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.occlusionMap());
    QCOMPARE(texture1.getRenderImage(), node->occlusionMap);

    // HeightMap
    QVERIFY(!material.heightMap());
    material.setHeightMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.heightMap());
    QCOMPARE(texture1.getRenderImage(), node->heightMap);
    const QQuick3DMaterial::TextureChannelMapping channelMapping1 = QQuick3DMaterial::A;
    material.setHeightChannel(channelMapping1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(channelMapping1, material.heightChannel());
    QCOMPARE(qToUnderlying(channelMapping1),      // QQuick3DMaterial::TextureChannelMapping
             qToUnderlying(node->heightChannel)); // QSSGRenderDefaultMaterial::TextureChannelMapping

    // ClearcoatMap
    QVERIFY(!material.clearcoatMap());
    material.setClearcoatMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.clearcoatMap());
    QCOMPARE(texture1.getRenderImage(), node->clearcoatMap);
    QVERIFY(material.clearcoatChannel() == QQuick3DMaterial::R);
    const QQuick3DMaterial::TextureChannelMapping channelMapping = QQuick3DMaterial::B;
    material.setClearcoatChannel(channelMapping);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(channelMapping, material.clearcoatChannel());
    QCOMPARE(qToUnderlying(channelMapping),          // QQuick3DMaterial::TextureChannelMapping
             qToUnderlying(node->clearcoatChannel)); // QSSGRenderDefaultMaterial::TextureChannelMapping

    // ClearcoatRoughnessMap
    QVERIFY(!material.clearcoatRoughnessMap());
    material.setClearcoatRoughnessMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.clearcoatRoughnessMap());
    QCOMPARE(texture1.getRenderImage(), node->clearcoatRoughnessMap);
    const QQuick3DMaterial::TextureChannelMapping channelMapping2 = QQuick3DMaterial::A;
    material.setClearcoatRoughnessChannel(channelMapping2);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(channelMapping2, material.clearcoatRoughnessChannel());
    QCOMPARE(qToUnderlying(channelMapping2),                  // QQuick3DMaterial::TextureChannelMapping
             qToUnderlying(node->clearcoatRoughnessChannel)); // QSSGRenderDefaultMaterial::TextureChannelMapping

    // ClearcoatNormalMap
    QVERIFY(!material.clearcoatNormalMap());
    material.setClearcoatNormalMap(&texture2);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.clearcoatNormalMap());
    QCOMPARE(texture2.getRenderImage(), node->clearcoatNormalMap);

    // TransmissionMap
    QVERIFY(!material.transmissionMap());
    material.setTransmissionMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.transmissionMap());
    QCOMPARE(texture1.getRenderImage(), node->transmissionMap);
    const QQuick3DMaterial::TextureChannelMapping channelMapping3 = QQuick3DMaterial::B;
    material.setTransmissionChannel(channelMapping3);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(channelMapping3, material.transmissionChannel());
    QCOMPARE(qToUnderlying(channelMapping3),            // QQuick3DMaterial::TextureChannelMapping
             qToUnderlying(node->transmissionChannel)); // QSSGRenderDefaultMaterial::TextureChannelMapping

    // Thickness Map
    QVERIFY(!material.thicknessMap());
    material.setThicknessMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.thicknessMap());
    QCOMPARE(texture1.getRenderImage(), node->thicknessMap);
    const QQuick3DMaterial::TextureChannelMapping channelMapping4 = QQuick3DMaterial::A;
    material.setThicknessChannel(channelMapping4);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(channelMapping4, material.thicknessChannel());
    QCOMPARE(qToUnderlying(channelMapping4),         // QQuick3DMaterial::TextureChannelMapping
             qToUnderlying(node->thicknessChannel)); // QSSGRenderDefaultMaterial::TextureChannelMapping
}

void tst_QQuick3DMaterials::testPrincipledEnums()
{
    PrincipledMaterial material;
    auto node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(nullptr));
    QVERIFY(node);

    // These test rely on the different enums having the same values
    auto lightModes = { QQuick3DPrincipledMaterial::Lighting::NoLighting,
                        QQuick3DPrincipledMaterial::Lighting::FragmentLighting };
    for (const auto lightMode : lightModes) {
        material.setLighting(lightMode);
        node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
        QCOMPARE(int(material.lighting()), int(node->lighting));
    }

    auto blendModes = { QQuick3DPrincipledMaterial::BlendMode::SourceOver,
                        QQuick3DPrincipledMaterial::BlendMode::Screen,
                        QQuick3DPrincipledMaterial::BlendMode::Multiply };
    for (const auto blendMode : blendModes) {
        material.setBlendMode(blendMode);
        node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
        QCOMPARE(int(material.blendMode()), int(node->blendMode));
    }

    auto alphaModes = { QQuick3DPrincipledMaterial::AlphaMode::Default,
                        QQuick3DPrincipledMaterial::AlphaMode::Mask,
                        QQuick3DPrincipledMaterial::AlphaMode::Blend };
    for (const auto alphaMode : alphaModes) {
        material.setAlphaMode(alphaMode);
        node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
        QCOMPARE(int(material.alphaMode()), int(node->alphaMode));
    }

    auto depthDrawModes = {
        QQuick3DMaterial::DepthDrawMode::OpaqueOnlyDepthDraw,
        QQuick3DMaterial::DepthDrawMode::AlwaysDepthDraw,
        QQuick3DMaterial::DepthDrawMode::NeverDepthDraw,
        QQuick3DMaterial::DepthDrawMode::OpaquePrePassDepthDraw
    };

    for (const auto depthDrawMode : depthDrawModes) {
        material.setDepthDrawMode(depthDrawMode);
        node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(nullptr));
        QCOMPARE(int(material.depthDrawMode()), int(node->depthDrawMode));
    }
}

void tst_QQuick3DMaterials::testSpecularGlossyProperties()
{
    SpecularGlossyMaterial material;
    auto node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(nullptr));
    const auto originalNode = node; // for comparisons later...
    QVERIFY(node);

    const QColor specularColor = QColor::fromRgbF(0.3f, 0.3f, 0.3f, 1.0f);
    const QVector3D specular = QSSGUtils::color::sRGBToLinear(specularColor).toVector3D();
    material.setSpecularColor(specularColor);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(specularColor, material.specularColor());
    QCOMPARE(specular.y(), node->specularTint.y()); // yes, stored in specularTint

    const float glossiness = 0.4f;
    material.setGlossiness(glossiness);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(glossiness, material.glossiness());
    QCOMPARE(glossiness, node->specularRoughness); // yes that is correct

    const float opacity = 0.7f;
    material.setOpacity(opacity);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(opacity, material.opacity());
    QCOMPARE(opacity, node->opacity);

    const float normalStrength = 0.8f;
    material.setNormalStrength(normalStrength);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(normalStrength, material.normalStrength());
    QCOMPARE(normalStrength, node->bumpAmount);

    const float occlusionAmount = 0.9f;
    material.setOcclusionAmount(occlusionAmount);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(occlusionAmount, material.occlusionAmount());
    QCOMPARE(occlusionAmount, node->occlusionAmount);

    const float alphaCutoff = 0.1f;
    material.setAlphaCutoff(alphaCutoff);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(alphaCutoff, material.alphaCutoff());
    QCOMPARE(alphaCutoff, node->alphaCutoff);

    QColor color1("#12345678");
    QVector4D color1Vec4 = QSSGUtils::color::sRGBToLinear(color1);
    QColor color2("#cccccccc");
    QVector3D color2Vec3 = QSSGUtils::color::sRGBToLinear(color2).toVector3D();
    material.setAlbedoColor(color1);
    material.setEmissiveFactor(color2Vec3);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QCOMPARE(color1Vec4, node->color);
    // Note: emissiveColor doesn't contain alpha
    QCOMPARE(color2Vec3, node->emissiveColor);

    const float heightAmount = 0.2f;
    material.setHeightAmount(heightAmount);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(heightAmount, material.heightAmount());
    QCOMPARE(heightAmount, node->heightAmount);

    const int minHeightMapSamples = 11;
    material.setMinHeightMapSamples(minHeightMapSamples);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(minHeightMapSamples, material.minHeightMapSamples());
    QCOMPARE(minHeightMapSamples, node->minHeightSamples);

    const int maxHeightMapSamples = 33;
    material.setMaxHeightMapSamples(maxHeightMapSamples);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(maxHeightMapSamples, material.maxHeightMapSamples());
    QCOMPARE(maxHeightMapSamples, node->maxHeightSamples);

    QVERIFY(!node->isClearcoatEnabled());
    float clearcoatAmount = 0.3f;
    material.setClearcoatAmount(clearcoatAmount);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(clearcoatAmount, material.clearcoatAmount());
    QCOMPARE(clearcoatAmount, node->clearcoatAmount);
    QVERIFY(node->isClearcoatEnabled());
    clearcoatAmount = 0.0f;
    material.setClearcoatAmount(clearcoatAmount);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(!node->isClearcoatEnabled());

    const float clearcoatRoughnessAmount = 0.4f;
    material.setClearcoatRoughnessAmount(clearcoatRoughnessAmount);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(clearcoatRoughnessAmount, material.clearcoatRoughnessAmount());
    QCOMPARE(clearcoatRoughnessAmount, node->clearcoatRoughnessAmount);

    QVERIFY(!node->isTransmissionEnabled());
    const float transmissionFactor = 0.5f;
    material.setTransmissionFactor(transmissionFactor);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(transmissionFactor, material.transmissionFactor());
    QCOMPARE(transmissionFactor, node->transmissionFactor);
    QVERIFY(node->isTransmissionEnabled());
    material.setTransmissionFactor(0.0f);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(!node->isTransmissionEnabled());

    const float thicknessFactor = 0.6f;
    material.setThicknessFactor(thicknessFactor);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(thicknessFactor, material.thicknessFactor());
    QCOMPARE(thicknessFactor, node->thicknessFactor);
    const float attenuationDistance = 0.7f;
    material.setAttenuationDistance(attenuationDistance);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(attenuationDistance, material.attenuationDistance());
    QCOMPARE(attenuationDistance, node->attenuationDistance);
    const QColor attenuationColor = QColor(Qt::green);
    const QVector3D attenuationColorVector = QSSGUtils::color::sRGBToLinear(attenuationColor).toVector3D();
    material.setAttenuationColor(attenuationColor);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(attenuationColor, material.attenuationColor());
    QCOMPARE(attenuationColorVector, node->attenuationColor);
}

void tst_QQuick3DMaterials::testSpecularGlossyTextures()
{
    SpecularGlossyMaterial material;
    auto node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(nullptr));
    const auto originalNode = node; // for comparisons later...
    QVERIFY(node);

    Texture texture1;
    texture1.setSource(QUrl(":/Built_with_Qt.png"));
    Texture texture2;
    texture2.setSource(QUrl(":/Built_with_Qt_2.png"));

    QQuick3DSceneManager sceneManager;
    sceneManager.updateDirtyResource(&texture1);
    sceneManager.updateDirtyResource(&texture2);

    // Albedo
    QVERIFY(!material.albedoMap());
    material.setAlbedoMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QVERIFY(material.albedoMap());
    QVERIFY(texture1.getRenderImage());
    QCOMPARE(texture1.getRenderImage(), node->colorMap);

    // SpecularMap
    material.setSpecularMap(&texture2);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(originalNode, node);
    QVERIFY(texture2.getRenderImage());
    QCOMPARE(texture2.getRenderImage(), node->specularMap);

    // GlossinessMap
    QVERIFY(!material.glossinessMap());
    material.setGlossinessMap(&texture2);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.glossinessMap());
    QCOMPARE(texture2.getRenderImage(), node->roughnessMap); // stored as roughness in backend

    // EmissiveMap
    QVERIFY(!material.emissiveMap());
    material.setEmissiveMap(&texture2);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.emissiveMap());
    QCOMPARE(texture2.getRenderImage(), node->emissiveMap);

    // OpacityMap
    QVERIFY(!material.opacityMap());
    material.setOpacityMap(&texture2);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.opacityMap());
    QCOMPARE(texture2.getRenderImage(), node->opacityMap);

    // NormalMap
    QVERIFY(!material.normalMap());
    material.setNormalMap(&texture2);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.normalMap());
    QCOMPARE(texture2.getRenderImage(), node->normalMap);

    // OcclusionMap
    QVERIFY(!material.occlusionMap());
    material.setOcclusionMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.occlusionMap());
    QCOMPARE(texture1.getRenderImage(), node->occlusionMap);

    // HeightMap
    QVERIFY(!material.heightMap());
    material.setHeightMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.heightMap());
    QCOMPARE(texture1.getRenderImage(), node->heightMap);
    const QQuick3DMaterial::TextureChannelMapping channelMapping1 = QQuick3DMaterial::A;
    material.setHeightChannel(channelMapping1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(channelMapping1, material.heightChannel());
    QCOMPARE(qToUnderlying(channelMapping1),      // QQuick3DMaterial::TextureChannelMapping
             qToUnderlying(node->heightChannel)); // QSSGRenderDefaultMaterial::TextureChannelMapping

    // ClearcoatMap
    QVERIFY(!material.clearcoatMap());
    material.setClearcoatMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.clearcoatMap());
    QCOMPARE(texture1.getRenderImage(), node->clearcoatMap);
    QVERIFY(material.clearcoatChannel() == QQuick3DMaterial::R);
    const QQuick3DMaterial::TextureChannelMapping channelMapping = QQuick3DMaterial::B;
    material.setClearcoatChannel(channelMapping);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(channelMapping, material.clearcoatChannel());
    QCOMPARE(qToUnderlying(channelMapping),          // QQuick3DMaterial::TextureChannelMapping
             qToUnderlying(node->clearcoatChannel)); // QSSGRenderDefaultMaterial::TextureChannelMapping

    // ClearcoatRoughnessMap
    QVERIFY(!material.clearcoatRoughnessMap());
    material.setClearcoatRoughnessMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.clearcoatRoughnessMap());
    QCOMPARE(texture1.getRenderImage(), node->clearcoatRoughnessMap);
    const QQuick3DMaterial::TextureChannelMapping channelMapping2 = QQuick3DMaterial::A;
    material.setClearcoatRoughnessChannel(channelMapping2);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(channelMapping2, material.clearcoatRoughnessChannel());
    QCOMPARE(qToUnderlying(channelMapping2),                  // QQuick3DMaterial::TextureChannelMapping
             qToUnderlying(node->clearcoatRoughnessChannel)); // QSSGRenderDefaultMaterial::TextureChannelMapping

    // ClearcoatNormalMap
    QVERIFY(!material.clearcoatNormalMap());
    material.setClearcoatNormalMap(&texture2);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.clearcoatNormalMap());
    QCOMPARE(texture2.getRenderImage(), node->clearcoatNormalMap);

    // TransmissionMap
    QVERIFY(!material.transmissionMap());
    material.setTransmissionMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.transmissionMap());
    QCOMPARE(texture1.getRenderImage(), node->transmissionMap);
    const QQuick3DMaterial::TextureChannelMapping channelMapping3 = QQuick3DMaterial::B;
    material.setTransmissionChannel(channelMapping3);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(channelMapping3, material.transmissionChannel());
    QCOMPARE(qToUnderlying(channelMapping3),            // QQuick3DMaterial::TextureChannelMapping
             qToUnderlying(node->transmissionChannel)); // QSSGRenderDefaultMaterial::TextureChannelMapping

    // Thickness Map
    QVERIFY(!material.thicknessMap());
    material.setThicknessMap(&texture1);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QVERIFY(material.thicknessMap());
    QCOMPARE(texture1.getRenderImage(), node->thicknessMap);
    const QQuick3DMaterial::TextureChannelMapping channelMapping4 = QQuick3DMaterial::A;
    material.setThicknessChannel(channelMapping4);
    node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
    QCOMPARE(channelMapping4, material.thicknessChannel());
    QCOMPARE(qToUnderlying(channelMapping4),         // QQuick3DMaterial::TextureChannelMapping
             qToUnderlying(node->thicknessChannel)); // QSSGRenderDefaultMaterial::TextureChannelMapping
}

void tst_QQuick3DMaterials::testSpecularGlossyEnums()
{
    SpecularGlossyMaterial material;
    auto node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(nullptr));
    QVERIFY(node);

    // These test rely on the different enums having the same values
    auto lightModes = { QQuick3DSpecularGlossyMaterial::Lighting::NoLighting,
                        QQuick3DSpecularGlossyMaterial::Lighting::FragmentLighting };
    for (const auto lightMode : lightModes) {
        material.setLighting(lightMode);
        node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
        QCOMPARE(int(material.lighting()), int(node->lighting));
    }

    auto blendModes = { QQuick3DSpecularGlossyMaterial::BlendMode::SourceOver,
                        QQuick3DSpecularGlossyMaterial::BlendMode::Screen,
                        QQuick3DSpecularGlossyMaterial::BlendMode::Multiply };
    for (const auto blendMode : blendModes) {
        material.setBlendMode(blendMode);
        node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
        QCOMPARE(int(material.blendMode()), int(node->blendMode));
    }

    auto alphaModes = { QQuick3DSpecularGlossyMaterial::AlphaMode::Default,
                        QQuick3DSpecularGlossyMaterial::AlphaMode::Mask,
                        QQuick3DSpecularGlossyMaterial::AlphaMode::Blend };
    for (const auto alphaMode : alphaModes) {
        material.setAlphaMode(alphaMode);
        node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(node));
        QCOMPARE(int(material.alphaMode()), int(node->alphaMode));
    }

    auto depthDrawModes = {
        QQuick3DMaterial::DepthDrawMode::OpaqueOnlyDepthDraw,
        QQuick3DMaterial::DepthDrawMode::AlwaysDepthDraw,
        QQuick3DMaterial::DepthDrawMode::NeverDepthDraw,
        QQuick3DMaterial::DepthDrawMode::OpaquePrePassDepthDraw
    };

    for (const auto depthDrawMode : depthDrawModes) {
        material.setDepthDrawMode(depthDrawMode);
        node = static_cast<QSSGRenderDefaultMaterial *>(material.updateSpatialNode(nullptr));
        QCOMPARE(int(material.depthDrawMode()), int(node->depthDrawMode));
    }
}


void tst_QQuick3DMaterials::testCustomMaterials()
{
    QSKIP("Test can not work as implemented without crashing");
    CustomMaterial material;
    QQuick3DViewport *view3D = new QQuick3DViewport;
    material.setParent(view3D);
    auto node = static_cast<QSSGRenderCustomMaterial *>(material.updateSpatialNode(nullptr));
    QVERIFY(node);

    QQuick3DShaderUtilsTextureInput mTexture;
    QQuick3DTexture qTexture;
    QSignalSpy spy(&mTexture, SIGNAL(textureChanged()));
    mTexture.setTexture(&qTexture);
    QCOMPARE(spy.size(), 1);
    QVERIFY(&qTexture == mTexture.texture());

    QQuick3DShaderUtilsBuffer mBuffer;
    auto format = QQuick3DShaderUtilsBuffer::TextureFormat::RGBA8;
    mBuffer.setFormat(format);
    auto filterOp = QQuick3DShaderUtilsBuffer::TextureFilterOperation::Nearest;
    mBuffer.setTextureFilterOperation(filterOp);
    QCOMPARE(format, mBuffer.format());
    QCOMPARE(filterOp, mBuffer.textureFilterOperation());

    // TODO: Extend custom material testing with proper shaders

    view3D->deleteLater();
}

QTEST_APPLESS_MAIN(tst_QQuick3DMaterials)
#include "tst_qquick3dmaterials.moc"
