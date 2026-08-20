// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>

QT_BEGIN_NAMESPACE

QSSGRenderLight::QSSGRenderLight(QSSGRenderGraphObject::Type type)
    : QSSGRenderNode(type)
    , m_scope(nullptr)
    , m_diffuseColor(1, 1, 1)
    , m_specularColor(1, 1, 1)
    , m_ambientColor(0, 0, 0)
    , m_brightness(1)
    , m_constantFade(1)
    , m_linearFade(0)
    , m_quadraticFade(1)
    , m_coneAngle(40.0f)
    , m_innerConeAngle(30.0f)
    , m_castShadow(false)
    , m_shadowBias(10.0f)
    , m_shadowFactor(75.0f)
    , m_shadowMapRes(512)
    , m_shadowMapFar(5000.0f)
    , m_shadowFilter(35.0f)
{
    Q_ASSERT(QSSGRenderGraphObject::isLight(type));
    markDirty(DirtyFlag::LightDirty);
}

void QSSGRenderLight::markDirty(DirtyFlag dirtyFlag)
{
    m_lightDirtyFlags |= FlagT(dirtyFlag);
    QSSGRenderNode::markDirty(QSSGRenderNode::DirtyFlag::SubNodeDirty);
}

void QSSGRenderLight::clearDirty(DirtyFlag dirtyFlag)
{
    m_lightDirtyFlags &= ~FlagT(dirtyFlag);
    QSSGRenderNode::clearDirty(QSSGRenderNode::DirtyFlag::SubNodeDirty);
}

QT_END_NAMESPACE
