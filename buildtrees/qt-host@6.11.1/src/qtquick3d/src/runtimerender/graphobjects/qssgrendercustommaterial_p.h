// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_CUSTOM_MATERIAL_H
#define QSSG_RENDER_CUSTOM_MATERIAL_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qurl.h>
#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>
#include <rhi/qrhi.h>

#include <QtQuick3DRuntimeRender/qtquick3druntimerenderexports.h>
#include <QtQuick3DRuntimeRender/private/qssgrendergraphobject_p.h>
#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>

QT_BEGIN_NAMESPACE

struct QSSGRenderImage;
struct QSSGShaderMaterialAdapter;
class QQuick3DShaderUtilsTextureInput;
class QQuick3DTexture;

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderCustomMaterial : public QSSGRenderGraphObject
{
    QSSGRenderCustomMaterial();
    ~QSSGRenderCustomMaterial();

    struct TextureProperty
    {
        QQuick3DShaderUtilsTextureInput *texInput = nullptr;
        QSSGRenderImage *texImage = nullptr;
        QByteArray name;
        QSSGRenderShaderValue::Type shaderDataType;
        QSSGRenderTextureFilterOp minFilterType = QSSGRenderTextureFilterOp::Linear;
        QSSGRenderTextureFilterOp magFilterType = QSSGRenderTextureFilterOp::Linear;
        QSSGRenderTextureFilterOp mipFilterType = QSSGRenderTextureFilterOp::Linear;
        QSSGRenderTextureCoordOp horizontalClampType = QSSGRenderTextureCoordOp::ClampToEdge;
        QSSGRenderTextureCoordOp verticalClampType = QSSGRenderTextureCoordOp::ClampToEdge;
        QSSGRenderTextureCoordOp zClampType = QSSGRenderTextureCoordOp::ClampToEdge;
        QQuick3DTexture *lastConnectedTexture = nullptr;
        QMetaObject::Connection minFilterChangedConn;
        QMetaObject::Connection magFilterChangedConn;
        QMetaObject::Connection mipFilterChangedConn;
        QMetaObject::Connection horizontalTilingChangedConn;
        QMetaObject::Connection verticalTilingChangedConn;
        QMetaObject::Connection depthTilingChangedConn;
    };
    using TexturePropertyList = QList<TextureProperty>;

    struct Property
    {
        Property() = default;
        Property(const QByteArray &name, const QVariant &value, QSSGRenderShaderValue::Type shaderDataType, int pid = -1)
            : name(name), value(value), shaderDataType(shaderDataType), pid(pid)
        { }
        QByteArray name;
        QVariant value;
        QSSGRenderShaderValue::Type shaderDataType;
        int pid;
    };
    using PropertyList = QList<Property>;

    enum class Flags : quint8
    {
        Dirty = 0x1,
        AlwaysDirty = 0x2
    };
    using FlagT = std::underlying_type_t<Flags>;

    enum class ShadingMode // must match QQuick3DCustomMaterial::ShadingMode
    {
        Unshaded,
        Shaded
    };

    enum class CustomShaderPresenceFlag {
        Vertex = 1 << 0,
        Fragment = 1 << 1
    };
    Q_DECLARE_FLAGS(CustomShaderPresence, CustomShaderPresenceFlag)

    enum class RenderFlag {
        Blending = 1 << 0,
        ScreenTexture = 1 << 1,
        DepthTexture = 1 << 2,
        AoTexture = 1 << 3,
        OverridesPosition = 1 << 4,
        ProjectionMatrix = 1 << 5,
        InverseProjectionMatrix = 1 << 6,
        ScreenMipTexture = 1 << 7,
        VarColor = 1 << 8,
        IblOrientation = 1 << 9,
        Lightmap = 1 << 10,
        Skinning = 1 << 11,
        Morphing = 1 << 12,
        ViewIndex = 1 << 13,
        Clearcoat = 1 << 14,
        ClearcoatFresnelScaleBias = 1 << 15,
        FresnelScaleBias = 1 << 16,
        Transmission = 1 << 17,
        NormalTexture = 1 << 18,
        MotionVectorTexture = 1 << 19
    };
    Q_DECLARE_FLAGS(RenderFlags, RenderFlag)

    enum {
        RegularShaderPathKeyIndex = 0,
        MultiViewShaderPathKeyIndex = 1
    };
    QByteArray m_shaderPathKey[2];
    CustomShaderPresence m_customShaderPresence;

    TexturePropertyList m_textureProperties;
    PropertyList m_properties;

    QSSGRenderImage *m_iblProbe = nullptr;
    QSSGRenderImage *m_emissiveMap = nullptr;
    QSSGCullFaceMode m_cullMode = QSSGCullFaceMode::Back;
    QSSGDepthDrawMode m_depthDrawMode = QSSGDepthDrawMode::OpaqueOnly;
    RenderFlags m_renderFlags;
    QRhiGraphicsPipeline::BlendFactor m_srcBlend;
    QRhiGraphicsPipeline::BlendFactor m_dstBlend;
    QRhiGraphicsPipeline::BlendFactor m_srcAlphaBlend;
    QRhiGraphicsPipeline::BlendFactor m_dstAlphaBlend;
    float m_lineWidth = 1.0f;

    QSSGRenderGraphObject *m_nextSibling = nullptr;

    ShadingMode m_shadingMode = ShadingMode::Shaded;

    FlagT m_flags { FlagT(Flags::Dirty) };
    bool incompleteBuildTimeObject = false; // Used by the shadergen tool
    bool m_usesSharedVariables = false;

    void markDirty();
    void clearDirty();
    void setAlwaysDirty(bool v);
    [[nodiscard]] inline bool isDirty() const { return ((m_flags & (FlagT(Flags::Dirty) | FlagT(Flags::AlwaysDirty))) != 0); }

    QSSGShaderMaterialAdapter *adapter = nullptr;

    QString debugObjectName;
};



Q_DECLARE_OPERATORS_FOR_FLAGS(QSSGRenderCustomMaterial::CustomShaderPresence)

QT_END_NAMESPACE

#endif
