// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DSPECULARGLOSSYMATERIAL_P_H
#define QQUICK3DSPECULARGLOSSYMATERIAL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version toSpecularGlossyMaterial
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick3D/private/qquick3dmaterial_p.h>
#include <QtQuick3D/private/qquick3dtexture_p.h>

#include <QColor>
#include <QHash>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DSpecularGlossyMaterial : public QQuick3DMaterial
{
    Q_OBJECT
    Q_PROPERTY(Lighting lighting READ lighting WRITE setLighting NOTIFY lightingChanged)
    Q_PROPERTY(BlendMode blendMode READ blendMode WRITE setBlendMode NOTIFY blendModeChanged)

    Q_PROPERTY(QColor albedoColor READ albedoColor WRITE setAlbedoColor NOTIFY albedoColorChanged)
    Q_PROPERTY(QQuick3DTexture *albedoMap READ albedoMap WRITE setAlbedoMap NOTIFY albedoMapChanged)
    Q_PROPERTY(bool albedoSingleChannelEnabled READ albedoSingleChannelEnabled WRITE setAlbedoSingleChannelEnabled NOTIFY albedoSingleChannelEnabledChanged REVISION(6, 8))
    Q_PROPERTY(QQuick3DMaterial::TextureChannelMapping albedoChannel READ albedoChannel WRITE setAlbedoChannel NOTIFY albedoChannelChanged REVISION(6, 8))

    Q_PROPERTY(QColor specularColor READ specularColor WRITE setSpecularColor NOTIFY specularColorChanged)
    Q_PROPERTY(QQuick3DTexture *specularMap READ specularMap WRITE setSpecularMap NOTIFY specularMapChanged)
    Q_PROPERTY(bool specularSingleChannelEnabled READ specularSingleChannelEnabled WRITE setSpecularSingleChannelEnabled NOTIFY specularSingleChannelEnabledChanged REVISION(6, 8))
    Q_PROPERTY(QQuick3DMaterial::TextureChannelMapping specularChannel READ specularChannel WRITE setSpecularChannel NOTIFY specularChannelChanged REVISION(6, 8))

    Q_PROPERTY(float glossiness READ glossiness WRITE setGlossiness NOTIFY glossinessChanged)
    Q_PROPERTY(QQuick3DTexture *glossinessMap READ glossinessMap WRITE setGlossinessMap NOTIFY glossinessMapChanged)
    Q_PROPERTY(QQuick3DMaterial::TextureChannelMapping glossinessChannel READ glossinessChannel WRITE setGlossinessChannel NOTIFY glossinessChannelChanged)

    Q_PROPERTY(QVector3D emissiveFactor READ emissiveFactor WRITE setEmissiveFactor NOTIFY emissiveFactorChanged)
    Q_PROPERTY(QQuick3DTexture *emissiveMap READ emissiveMap WRITE setEmissiveMap NOTIFY emissiveMapChanged)
    Q_PROPERTY(bool emissiveSingleChannelEnabled READ emissiveSingleChannelEnabled WRITE setEmissiveSingleChannelEnabled NOTIFY emissiveSingleChannelEnabledChanged REVISION(6, 8))
    Q_PROPERTY(QQuick3DMaterial::TextureChannelMapping emissiveChannel READ emissiveChannel WRITE setEmissiveChannel NOTIFY emissiveChannelChanged REVISION(6, 8))

    Q_PROPERTY(bool invertOpacityMapValue READ invertOpacityMapValue WRITE setInvertOpacityMapValue NOTIFY invertOpacityMapValueChanged REVISION(6, 8))
    Q_PROPERTY(float opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(QQuick3DTexture *opacityMap READ opacityMap WRITE setOpacityMap NOTIFY opacityMapChanged)
    Q_PROPERTY(QQuick3DMaterial::TextureChannelMapping opacityChannel READ opacityChannel WRITE setOpacityChannel NOTIFY opacityChannelChanged)

    Q_PROPERTY(QQuick3DTexture *normalMap READ normalMap WRITE setNormalMap NOTIFY normalMapChanged)
    Q_PROPERTY(float normalStrength READ normalStrength WRITE setNormalStrength NOTIFY normalStrengthChanged)

    Q_PROPERTY(QQuick3DTexture *occlusionMap READ occlusionMap WRITE setOcclusionMap NOTIFY occlusionMapChanged)
    Q_PROPERTY(QQuick3DMaterial::TextureChannelMapping occlusionChannel READ occlusionChannel WRITE setOcclusionChannel NOTIFY occlusionChannelChanged)
    Q_PROPERTY(float occlusionAmount READ occlusionAmount WRITE setOcclusionAmount NOTIFY occlusionAmountChanged)

    Q_PROPERTY(AlphaMode alphaMode READ alphaMode WRITE setAlphaMode NOTIFY alphaModeChanged)
    Q_PROPERTY(float alphaCutoff READ alphaCutoff WRITE setAlphaCutoff NOTIFY alphaCutoffChanged)

    Q_PROPERTY(float pointSize READ pointSize WRITE setPointSize NOTIFY pointSizeChanged)
    Q_PROPERTY(float lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)

    Q_PROPERTY(QQuick3DTexture *heightMap READ heightMap WRITE setHeightMap NOTIFY heightMapChanged)
    Q_PROPERTY(QQuick3DMaterial::TextureChannelMapping heightChannel READ heightChannel WRITE setHeightChannel NOTIFY heightChannelChanged)
    Q_PROPERTY(float heightAmount READ heightAmount WRITE setHeightAmount NOTIFY heightAmountChanged)
    Q_PROPERTY(int minHeightMapSamples READ minHeightMapSamples WRITE setMinHeightMapSamples NOTIFY minHeightMapSamplesChanged)
    Q_PROPERTY(int maxHeightMapSamples READ maxHeightMapSamples WRITE setMaxHeightMapSamples NOTIFY maxHeightMapSamplesChanged)

    Q_PROPERTY(float clearcoatAmount READ clearcoatAmount WRITE setClearcoatAmount NOTIFY clearcoatAmountChanged)
    Q_PROPERTY(QQuick3DTexture *clearcoatMap READ clearcoatMap WRITE setClearcoatMap NOTIFY clearcoatMapChanged)
    Q_PROPERTY(QQuick3DMaterial::TextureChannelMapping clearcoatChannel READ clearcoatChannel WRITE setClearcoatChannel NOTIFY
                       clearcoatChannelChanged)
    Q_PROPERTY(float clearcoatRoughnessAmount READ clearcoatRoughnessAmount WRITE setClearcoatRoughnessAmount NOTIFY
                       clearcoatRoughnessAmountChanged)
    Q_PROPERTY(QQuick3DMaterial::TextureChannelMapping clearcoatRoughnessChannel READ clearcoatRoughnessChannel WRITE
                       setClearcoatRoughnessChannel NOTIFY clearcoatRoughnessChannelChanged)
    Q_PROPERTY(QQuick3DTexture *clearcoatRoughnessMap READ clearcoatRoughnessMap WRITE setClearcoatRoughnessMap NOTIFY
                       clearcoatRoughnessMapChanged)
    Q_PROPERTY(QQuick3DTexture *clearcoatNormalMap READ clearcoatNormalMap WRITE setClearcoatNormalMap NOTIFY
                       clearcoatNormalMapChanged)
    Q_PROPERTY(float clearcoatNormalStrength READ clearcoatNormalStrength WRITE setClearcoatNormalStrength NOTIFY clearcoatNormalStrengthChanged REVISION(6, 8))

    Q_PROPERTY(float transmissionFactor READ transmissionFactor WRITE setTransmissionFactor NOTIFY transmissionFactorChanged)
    Q_PROPERTY(QQuick3DTexture * transmissionMap READ transmissionMap WRITE setTransmissionMap NOTIFY transmissionMapChanged)
    Q_PROPERTY(QQuick3DMaterial::TextureChannelMapping transmissionChannel READ transmissionChannel WRITE setTransmissionChannel NOTIFY transmissionChannelChanged)

    Q_PROPERTY(float thicknessFactor READ thicknessFactor WRITE setThicknessFactor NOTIFY thicknessFactorChanged)
    Q_PROPERTY(QQuick3DTexture *thicknessMap READ thicknessMap WRITE setThicknessMap NOTIFY thicknessMapChanged)
    Q_PROPERTY(QQuick3DMaterial::TextureChannelMapping thicknessChannel READ thicknessChannel WRITE setThicknessChannel NOTIFY
                       thicknessChannelChanged)
    Q_PROPERTY(float attenuationDistance READ attenuationDistance WRITE setAttenuationDistance NOTIFY attenuationDistanceChanged)
    Q_PROPERTY(QColor attenuationColor READ attenuationColor WRITE setAttenuationColor NOTIFY attenuationColorChanged)

    Q_PROPERTY(bool vertexColorsEnabled READ vertexColorsEnabled WRITE setVertexColorsEnabled NOTIFY vertexColorsEnabledChanged REVISION(6, 5))

    Q_PROPERTY(bool fresnelScaleBiasEnabled READ fresnelScaleBiasEnabled WRITE setFresnelScaleBiasEnabled NOTIFY fresnelScaleBiasEnabledChanged REVISION(6, 8))
    Q_PROPERTY(float fresnelScale READ fresnelScale WRITE setFresnelScale NOTIFY fresnelScaleChanged REVISION(6, 8))
    Q_PROPERTY(float fresnelBias READ fresnelBias WRITE setFresnelBias NOTIFY fresnelBiasChanged REVISION(6, 8))
    Q_PROPERTY(float fresnelPower READ fresnelPower WRITE setFresnelPower NOTIFY fresnelPowerChanged REVISION(6, 8))

    Q_PROPERTY(bool clearcoatFresnelScaleBiasEnabled READ clearcoatFresnelScaleBiasEnabled WRITE setClearcoatFresnelScaleBiasEnabled NOTIFY clearcoatFresnelScaleBiasEnabledChanged REVISION(6, 8))
    Q_PROPERTY(float clearcoatFresnelScale READ clearcoatFresnelScale WRITE setClearcoatFresnelScale NOTIFY clearcoatFresnelScaleChanged REVISION(6, 8))
    Q_PROPERTY(float clearcoatFresnelBias READ clearcoatFresnelBias WRITE setClearcoatFresnelBias NOTIFY clearcoatFresnelBiasChanged REVISION(6, 8))
    Q_PROPERTY(float clearcoatFresnelPower READ clearcoatFresnelPower WRITE setClearcoatFresnelPower NOTIFY clearcoatFresnelPowerChanged REVISION(6, 8))

    Q_PROPERTY(bool vertexColorsMaskEnabled READ vertexColorsMaskEnabled WRITE setVertexColorsMaskEnabled NOTIFY vertexColorsMaskEnabledChanged REVISION(6, 8))
    Q_PROPERTY(VertexColorMaskFlags vertexColorRedMask READ vertexColorRedMask WRITE setVertexColorRedMask NOTIFY vertexColorRedMaskChanged REVISION(6, 8))
    Q_PROPERTY(VertexColorMaskFlags vertexColorGreenMask READ vertexColorGreenMask WRITE setVertexColorGreenMask NOTIFY vertexColorGreenMaskChanged REVISION(6, 8))
    Q_PROPERTY(VertexColorMaskFlags vertexColorBlueMask READ vertexColorBlueMask WRITE setVertexColorBlueMask NOTIFY vertexColorBlueMaskChanged REVISION(6, 8))
    Q_PROPERTY(VertexColorMaskFlags vertexColorAlphaMask READ vertexColorAlphaMask WRITE setVertexColorAlphaMask NOTIFY vertexColorAlphaMaskChanged REVISION(6, 8))

    QML_NAMED_ELEMENT(SpecularGlossyMaterial)
    QML_ADDED_IN_VERSION(6, 4)

public:
    enum Lighting {
        NoLighting = 0,
        FragmentLighting
    };
    Q_ENUM(Lighting)

    enum BlendMode {
        SourceOver = 0,
        Screen,
        Multiply
    };
    Q_ENUM(BlendMode)

    enum AlphaMode {
        Default = 0,
        Mask,
        Blend,
        Opaque
    };
    Q_ENUM(AlphaMode)

    enum VertexColorMask {
        NoMask = QQuick3DMaterial::NoMask,
        GlossinessMask = QQuick3DMaterial::RoughnessMask,
        NormalStrengthMask = QQuick3DMaterial::NormalStrengthMask,
        ClearcoatAmountMask = QQuick3DMaterial::ClearcoatAmountMask,
        ClearcoatRoughnessAmountMask = QQuick3DMaterial::ClearcoatRoughnessAmountMask,
        ClearcoatNormalStrengthMask = QQuick3DMaterial::NormalStrengthMask,
        HeightAmountMask = QQuick3DMaterial::HeightAmountMask,
        OcclusionAmountMask = QQuick3DMaterial::OcclusionAmountMask,
        ThicknessFactorMask = QQuick3DMaterial::ThicknessFactorMask,
        TransmissionFactorMask = QQuick3DMaterial::TransmissionFactorMask
    };
    Q_ENUM(VertexColorMask)
    Q_DECLARE_FLAGS(VertexColorMaskFlags, VertexColorMask)
    Q_FLAG(VertexColorMaskFlags)

    explicit QQuick3DSpecularGlossyMaterial(QQuick3DObject *parent = nullptr);
    ~QQuick3DSpecularGlossyMaterial() override;

    Lighting lighting() const;
    BlendMode blendMode() const;
    QColor albedoColor() const;
    QQuick3DTexture *albedoMap() const;
    Q_REVISION(6, 8) bool albedoSingleChannelEnabled() const;
    Q_REVISION(6, 8) TextureChannelMapping albedoChannel() const;
    Q_REVISION(6, 8) bool specularSingleChannelEnabled() const;
    Q_REVISION(6, 8) TextureChannelMapping specularChannel() const;
    Q_REVISION(6, 8) bool emissiveSingleChannelEnabled() const;
    Q_REVISION(6, 8) TextureChannelMapping emissiveChannel() const;
    QQuick3DTexture *emissiveMap() const;
    QVector3D emissiveFactor() const;
    float glossiness() const;
    QQuick3DTexture *glossinessMap() const;
    Q_REVISION(6, 8) bool invertOpacityMapValue() const;
    float opacity() const;
    QQuick3DTexture *opacityMap() const;
    QQuick3DTexture *normalMap() const;
    QColor specularColor() const;
    QQuick3DTexture *specularMap() const;
    float normalStrength() const;
    QQuick3DTexture *occlusionMap() const;
    float occlusionAmount() const;
    AlphaMode alphaMode() const;
    float alphaCutoff() const;
    TextureChannelMapping glossinessChannel() const;
    TextureChannelMapping opacityChannel() const;
    TextureChannelMapping occlusionChannel() const;
    float pointSize() const;
    float lineWidth() const;
    QQuick3DTexture *heightMap() const;
    TextureChannelMapping heightChannel() const;
    float heightAmount() const;
    int minHeightMapSamples() const;
    int maxHeightMapSamples() const;

    float clearcoatAmount() const;
    QQuick3DTexture *clearcoatMap() const;
    TextureChannelMapping clearcoatChannel() const;
    float clearcoatRoughnessAmount() const;
    TextureChannelMapping clearcoatRoughnessChannel() const;
    QQuick3DTexture *clearcoatRoughnessMap() const;
    QQuick3DTexture *clearcoatNormalMap() const;

    float transmissionFactor() const;
    QQuick3DTexture *transmissionMap() const;
    TextureChannelMapping transmissionChannel() const;

    float thicknessFactor() const;
    QQuick3DTexture *thicknessMap() const;
    TextureChannelMapping thicknessChannel() const;
    float attenuationDistance() const;
    QColor attenuationColor() const;

    Q_REVISION(6, 8) bool fresnelScaleBiasEnabled() const;
    Q_REVISION(6, 8) float fresnelScale() const;
    Q_REVISION(6, 8) float fresnelBias() const;
    Q_REVISION(6, 8) float fresnelPower() const;

    Q_REVISION(6, 8) bool clearcoatFresnelScaleBiasEnabled() const;
    Q_REVISION(6, 8) float clearcoatFresnelScale() const;
    Q_REVISION(6, 8) float clearcoatFresnelBias() const;
    Q_REVISION(6, 8) float clearcoatFresnelPower() const;

    Q_REVISION(6, 5) bool vertexColorsEnabled() const;
    Q_REVISION(6, 8) float clearcoatNormalStrength() const;

    Q_REVISION(6, 8) bool vertexColorsMaskEnabled() const;
    Q_REVISION(6, 8) VertexColorMaskFlags vertexColorRedMask() const;
    Q_REVISION(6, 8) VertexColorMaskFlags vertexColorGreenMask() const;
    Q_REVISION(6, 8) VertexColorMaskFlags vertexColorBlueMask() const;
    Q_REVISION(6, 8) VertexColorMaskFlags vertexColorAlphaMask() const;

public Q_SLOTS:
    void setLighting(QQuick3DSpecularGlossyMaterial::Lighting lighting);
    void setBlendMode(QQuick3DSpecularGlossyMaterial::BlendMode blendMode);
    void setAlbedoColor(const QColor &albedo);
    void setAlbedoMap(QQuick3DTexture *albedoMap);
    Q_REVISION(6, 8) void setAlbedoSingleChannelEnabled(bool albedoSingleChannelEnabled);
    Q_REVISION(6, 8) void setAlbedoChannel(QQuick3DMaterial::TextureChannelMapping channel);
    Q_REVISION(6, 8) void setSpecularSingleChannelEnabled(bool specularSingleChannelEnabled);
    Q_REVISION(6, 8) void setSpecularChannel(QQuick3DMaterial::TextureChannelMapping channel);
    Q_REVISION(6, 8) void setEmissiveSingleChannelEnabled(bool emissiveSingleChannelEnabled);
    Q_REVISION(6, 8) void setEmissiveChannel(QQuick3DMaterial::TextureChannelMapping channel);
    void setEmissiveMap(QQuick3DTexture *emissiveMap);
    void setEmissiveFactor(const QVector3D &emissiveFactor);
    void setGlossiness(float glossiness);
    void setGlossinessMap(QQuick3DTexture *glossinessMap);
    Q_REVISION(6, 8) void setInvertOpacityMapValue(bool invertOpacityMapValue);
    void setOpacity(float opacity);
    void setOpacityMap(QQuick3DTexture *opacityMap);
    void setNormalMap(QQuick3DTexture *normalMap);
    void setSpecularColor(const QColor &specular);
    void setSpecularMap(QQuick3DTexture *specularMap);
    void setNormalStrength(float normalStrength);
    void setOcclusionMap(QQuick3DTexture *occlusionMap);
    void setOcclusionAmount(float occlusionAmount);
    void setAlphaMode(QQuick3DSpecularGlossyMaterial::AlphaMode alphaMode);
    void setAlphaCutoff(float alphaCutoff);
    void setGlossinessChannel(QQuick3DMaterial::TextureChannelMapping channel);
    void setOpacityChannel(QQuick3DMaterial::TextureChannelMapping channel);
    void setOcclusionChannel(QQuick3DMaterial::TextureChannelMapping channel);
    void setPointSize(float size);
    void setLineWidth(float width);
    void setHeightMap(QQuick3DTexture *heightMap);
    void setHeightChannel(QQuick3DMaterial::TextureChannelMapping channel);
    void setHeightAmount(float heightAmount);
    void setMinHeightMapSamples(int samples);
    void setMaxHeightMapSamples(int samples);

    void setClearcoatAmount(float newClearcoatAmount);
    void setClearcoatMap(QQuick3DTexture *newClearcoatMap);
    void setClearcoatChannel(QQuick3DMaterial::TextureChannelMapping newClearcoatChannel);
    void setClearcoatRoughnessAmount(float newClearcoatRoughnessAmount);
    void setClearcoatRoughnessChannel(QQuick3DMaterial::TextureChannelMapping newClearcoatRoughnessChannel);
    void setClearcoatRoughnessMap(QQuick3DTexture *newClearcoatRoughnessMap);
    void setClearcoatNormalMap(QQuick3DTexture *newClearcoatNormalMap);
    Q_REVISION(6, 8) void setClearcoatNormalStrength( float newClearcoatNormalStrength );

    void setTransmissionFactor(float newTransmissionFactor);
    void setTransmissionMap(QQuick3DTexture *newTransmissionMap);
    void setTransmissionChannel(QQuick3DMaterial::TextureChannelMapping newTransmissionChannel);

    void setThicknessFactor(float newThicknessFactor);
    void setThicknessMap(QQuick3DTexture *newThicknessMap);
    void setThicknessChannel(QQuick3DMaterial::TextureChannelMapping newThicknessChannel);
    void setAttenuationDistance(float newAttenuationDistance);
    void setAttenuationColor(const QColor &newAttenuationColor);

    Q_REVISION(6, 8) void setFresnelScaleBiasEnabled(bool fresnelScaleBias);
    Q_REVISION(6, 8) void setFresnelScale(float fresnelScale);
    Q_REVISION(6, 8) void setFresnelBias(float fresnelBias);
    Q_REVISION(6, 8) void setFresnelPower(float fresnelPower);

    Q_REVISION(6, 8) void setClearcoatFresnelScaleBiasEnabled(bool clearcoatFresnelScaleBias);
    Q_REVISION(6, 8) void setClearcoatFresnelScale(float clearcoatFresnelScale);
    Q_REVISION(6, 8) void setClearcoatFresnelBias(float clearcoatFresnelBias);
    Q_REVISION(6, 8) void setClearcoatFresnelPower(float clearcoatFresnelPower);

    Q_REVISION(6, 5) void setVertexColorsEnabled(bool vertexColorsEnabled);

    Q_REVISION(6, 8) void setVertexColorsMaskEnabled(bool vertexColorsMaskEnabled);
    Q_REVISION(6, 8) void setVertexColorRedMask(VertexColorMaskFlags vertexColorRedMask);
    Q_REVISION(6, 8) void setVertexColorGreenMask(VertexColorMaskFlags vertexColorGreenMask);
    Q_REVISION(6, 8) void setVertexColorBlueMask(VertexColorMaskFlags vertexColorBlueMask);
    Q_REVISION(6, 8) void setVertexColorAlphaMask(VertexColorMaskFlags vertexColorAlphaMask);

Q_SIGNALS:
    void lightingChanged();
    void blendModeChanged();
    void albedoColorChanged();
    void albedoMapChanged();
    Q_REVISION(6, 8) void albedoSingleChannelEnabledChanged();
    Q_REVISION(6, 8) void albedoChannelChanged();
    Q_REVISION(6, 8) void specularSingleChannelEnabledChanged();
    Q_REVISION(6, 8) void specularChannelChanged();
    Q_REVISION(6, 8) void emissiveSingleChannelEnabledChanged();
    Q_REVISION(6, 8) void emissiveChannelChanged();
    void emissiveMapChanged();
    void emissiveFactorChanged();
    void glossinessChanged();
    void glossinessMapChanged();
    Q_REVISION(6, 8) void invertOpacityMapValueChanged();
    void opacityChanged();
    void opacityMapChanged();
    void normalMapChanged();
    void specularColorChanged();
    void specularMapChanged();
    void normalStrengthChanged();
    void occlusionMapChanged();
    void occlusionAmountChanged();
    void alphaModeChanged();
    void alphaCutoffChanged();
    void glossinessChannelChanged();
    void opacityChannelChanged();
    void occlusionChannelChanged();
    void pointSizeChanged();
    void lineWidthChanged();
    void heightMapChanged();
    void heightChannelChanged();
    void heightAmountChanged();
    void minHeightMapSamplesChanged();
    void maxHeightMapSamplesChanged();

    void clearcoatAmountChanged();
    void clearcoatMapChanged();
    void clearcoatChannelChanged();
    void clearcoatRoughnessAmountChanged();
    void clearcoatRoughnessChannelChanged();
    void clearcoatRoughnessMapChanged();
    void clearcoatNormalMapChanged();
    void clearcoatNormalStrengthChanged();

    void transmissionFactorChanged();
    void transmissionMapChanged();
    void transmissionChannelChanged();

    void thicknessFactorChanged();
    void thicknessMapChanged();
    void thicknessChannelChanged();
    void attenuationDistanceChanged();
    void attenuationColorChanged();

    Q_REVISION(6, 5) void vertexColorsEnabledChanged(bool vertexColorsEnabled);

    Q_REVISION(6, 8) void fresnelScaleBiasEnabledChanged(bool fresnelScaleBiasEnabled);
    Q_REVISION(6, 8) void fresnelScaleChanged(float fresnelScale);
    Q_REVISION(6, 8) void fresnelBiasChanged(float fresnelBias);
    Q_REVISION(6, 8) void fresnelPowerChanged(float fresnelPower);

    Q_REVISION(6, 8) void clearcoatFresnelScaleBiasEnabledChanged(bool clearcoatFresnelScaleBiasEnabled);
    Q_REVISION(6, 8) void clearcoatFresnelScaleChanged(float clearcoatFresnelScale);
    Q_REVISION(6, 8) void clearcoatFresnelBiasChanged(float clearcoatFresnelBias);
    Q_REVISION(6, 8) void clearcoatFresnelPowerChanged(float clearcoatFresnelPower);

    Q_REVISION(6, 8) void vertexColorsMaskEnabledChanged();
    Q_REVISION(6, 8) void vertexColorRedMaskChanged();
    Q_REVISION(6, 8) void vertexColorGreenMaskChanged();
    Q_REVISION(6, 8) void vertexColorBlueMaskChanged();
    Q_REVISION(6, 8) void vertexColorAlphaMaskChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    void markAllDirty() override;
    void itemChange(ItemChange, const ItemChangeData &) override;
private:

    enum DirtyType {
        LightingModeDirty = 0x00000001,
        BlendModeDirty =    0x00000002,
        AlbedoDirty =       0x00000004,
        EmissiveDirty =     0x00000008,
        SpecularDirty =     0x00000010,
        OpacityDirty =      0x00000020,
        NormalDirty =       0x00000040,
        GlossyDirty =       0x00000080,
        OcclusionDirty =    0x00000100,
        AlphaModeDirty =    0x00000200,
        PointSizeDirty =    0x00000400,
        LineWidthDirty =    0x00000800,
        HeightDirty =       0x00001000,
        ClearcoatDirty =    0x00002000,
        TransmissionDirty = 0x00004000,
        VolumeDirty =       0x00008000,
        VertexColorsDirty = 0x00001000
    };

    void updateSceneManager(QQuick3DSceneManager *window);

    Lighting m_lighting = FragmentLighting;
    BlendMode m_blendMode = SourceOver;
    AlphaMode m_alphaMode = Default;
    QColor m_albedo = Qt::white;
    QQuick3DTexture *m_albedoMap = nullptr;
    bool m_albedoSingleChannelEnabled = false;
    TextureChannelMapping m_albedoChannel = QQuick3DMaterial::R;
    bool m_specularSingleChannelEnabled = false;
    TextureChannelMapping m_specularChannel = QQuick3DMaterial::R;
    bool m_emissiveSingleChannelEnabled = false;
    TextureChannelMapping m_emissiveChannel = QQuick3DMaterial::R;
    QVector3D m_emissiveFactor;
    QQuick3DTexture *m_emissiveMap = nullptr;
    QQuick3DTexture *m_glossinessMap = nullptr;
    QQuick3DTexture *m_opacityMap = nullptr;
    QQuick3DTexture *m_normalMap = nullptr;
    QQuick3DTexture *m_specularMap = nullptr;
    QQuick3DTexture *m_occlusionMap = nullptr;
    float m_glossiness = 1.0f;
    bool m_invertOpacityMapValue = false;
    float m_opacity = 1.0f;
    QColor m_specular = Qt::white;
    float m_normalStrength = 1.0f;
    float m_occlusionAmount = 1.0f;
    float m_alphaCutoff = 0.5f;
    TextureChannelMapping m_glossinessChannel = QQuick3DMaterial::A;
    TextureChannelMapping m_opacityChannel = QQuick3DMaterial::A;
    TextureChannelMapping m_occlusionChannel = QQuick3DMaterial::R;
    float m_pointSize = 1.0f;
    float m_lineWidth = 1.0f;
    QQuick3DTexture *m_heightMap = nullptr;
    TextureChannelMapping m_heightChannel = QQuick3DMaterial::R;
    float m_heightAmount = 0.0f;
    int m_minHeightMapSamples = 8;
    int m_maxHeightMapSamples = 32;
    float m_clearcoatAmount = 0.0f;
    QQuick3DTexture *m_clearcoatMap = nullptr;
    TextureChannelMapping m_clearcoatChannel = QQuick3DMaterial::R;
    float m_clearcoatRoughnessAmount = 0.0f;
    TextureChannelMapping m_clearcoatRoughnessChannel = QQuick3DMaterial::G;
    QQuick3DTexture *m_clearcoatRoughnessMap = nullptr;
    QQuick3DTexture *m_clearcoatNormalMap = nullptr;
    float m_clearcoatNormalStrength = 1.0f;
    float m_transmissionFactor = 0.0f;
    QQuick3DTexture *m_transmissionMap = nullptr;
    TextureChannelMapping m_transmissionChannel = QQuick3DMaterial::R;
    float m_thicknessFactor = 0.0f;
    QQuick3DTexture *m_thicknessMap = nullptr;
    TextureChannelMapping m_thicknessChannel = QQuick3DMaterial::G;
    float m_attenuationDistance = std::numeric_limits<float>::infinity();
    QColor m_attenuationColor = Qt::white;
    bool m_fresnelScaleBiasEnabled = false;
    float m_fresnelScale = 1.0f;
    float m_fresnelBias = 0.0f;
    float m_fresnelPower = 5.0f;
    bool m_clearcoatFresnelScaleBiasEnabled = false;
    float m_clearcoatFresnelScale = 1.0f;
    float m_clearcoatFresnelBias = 0.0f;
    float m_clearcoatFresnelPower = 5.0f;
    bool m_vertexColorsEnabled = true;
    bool m_vertexColorsMaskEnabled = false;
    VertexColorMaskFlags m_vertexColorRedMask = NoMask;
    VertexColorMaskFlags m_vertexColorGreenMask = NoMask;
    VertexColorMaskFlags m_vertexColorBlueMask = NoMask;
    VertexColorMaskFlags m_vertexColorAlphaMask = NoMask;

    quint32 m_dirtyAttributes = 0xffffffff; // all dirty by default
    void markDirty(DirtyType type);

    static constexpr float ensureNormalized(float val) { return qBound(0.0f, val, 1.0f); }
};

QT_END_NAMESPACE

#endif // QQUICK3DSPECULARGLOSSYMATERIAL_P_H
