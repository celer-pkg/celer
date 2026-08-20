// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_LIGHT_H
#define QSSG_RENDER_LIGHT_H

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

#include <QtQuick3DRuntimeRender/private/qssgrendernode_p.h>

QT_BEGIN_NAMESPACE

struct QSSGRenderImage;

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderLight : public QSSGRenderNode
{
    enum class DirtyFlag : quint8
    {
        LightDirty = 0x1
    };
    using FlagT = std::underlying_type_t<DirtyFlag>;

    // Must match QQuick3DAbstractLight::QSSGSoftShadowQuality
    enum class SoftShadowQuality {
        Hard = 0,
        PCF4,
        PCF8,
        PCF16,
        PCF32,
        PCF64,
    };

    static constexpr DirtyFlag DirtyMask { std::numeric_limits<FlagT>::max() };

    QSSGRenderNode *m_scope;
    QVector3D m_diffuseColor; // colors are 0-1 normalized
    QVector3D m_specularColor; // colors are 0-1 normalized
    QVector3D m_ambientColor; // colors are 0-1 normalized

    // The variables below are in the same range as Studio
    // Only valid if node is a point light
    float m_brightness;
    float m_constantFade;
    float m_linearFade;
    float m_quadraticFade;

    float m_coneAngle; // 0-180
    float m_innerConeAngle; // 0-180

    FlagT m_lightDirtyFlags = 0;
    bool m_castShadow; // true if this light produce shadows
    float m_shadowBias; // depth shift to avoid self-shadowing artifacts
    float m_shadowFactor; // Darkening factor for ESMs
    quint32 m_shadowMapRes; // Resolution of shadow map
    float m_shadowMapFar; // Far clip plane for the shadow map
    float m_shadowFilter; // Shadow map filter step size
    SoftShadowQuality m_softShadowQuality = SoftShadowQuality::PCF4;

    float m_pcfFactor = 2.0f;
    bool m_use32BitShadowmap = false;

    bool m_bakingEnabled = false;
    bool m_fullyBaked = false; // direct+indirect

    // Cascading shadow map options
    float m_csmSplit1 = 0.1f;
    float m_csmSplit2 = 0.25f;
    float m_csmSplit3 = 0.5f;
    int m_csmNumSplits = 0;
    float m_csmBlendRatio = 0.05f;
    bool m_lockShadowmapTexels = false;

    // Defaults to directional light
    explicit QSSGRenderLight(Type type = Type::DirectionalLight);

    [[nodiscard]] inline bool isEnabled() const { return (m_brightness > 0.0f); }

    [[nodiscard]] inline bool isDirty(DirtyFlag dirtyFlag = DirtyMask) const
    {
        return ((m_lightDirtyFlags & FlagT(dirtyFlag)) != 0)
                || ((dirtyFlag == DirtyMask) && QSSGRenderNode::isDirty());
    }
    void markDirty(DirtyFlag dirtyFlag);
    void clearDirty(DirtyFlag dirtyFlag);
};
QT_END_NAMESPACE

#endif
