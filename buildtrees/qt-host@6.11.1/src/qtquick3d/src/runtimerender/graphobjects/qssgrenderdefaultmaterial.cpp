// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterial_p.h>
#include <QtQuick3DRuntimeRender/private/qssgshadermaterialadapter_p.h>

QT_BEGIN_NAMESPACE

QSSGRenderDefaultMaterial::QSSGRenderDefaultMaterial(QSSGRenderGraphObject::Type type)
    : QSSGRenderGraphObject(type)
{
    Q_ASSERT(type == QSSGRenderGraphObject::Type::DefaultMaterial ||
             type == QSSGRenderGraphObject::Type::PrincipledMaterial ||
             type == QSSGRenderGraphObject::Type::SpecularGlossyMaterial);
    if (type == QSSGRenderGraphObject::Type::PrincipledMaterial) {
        occlusionChannel = TextureChannelMapping::R;
        roughnessChannel = TextureChannelMapping::G;
        metalnessChannel = TextureChannelMapping::B;
    }

    adapter = QSSGShaderMaterialAdapter::create(*this);
}

QSSGRenderDefaultMaterial::~QSSGRenderDefaultMaterial()
{
    delete adapter;
}

void QSSGRenderDefaultMaterial::clearDirty()
{
    dirty = false;
}

QSSGRenderCustomMaterial::QSSGRenderCustomMaterial()
    : QSSGRenderGraphObject(Type::CustomMaterial)
{
    adapter = QSSGShaderMaterialAdapter::create(*this);
}

QSSGRenderCustomMaterial::~QSSGRenderCustomMaterial()
{
    delete adapter;
}

void QSSGRenderCustomMaterial::markDirty()
{
    m_flags |= FlagT(Flags::Dirty);
}

void QSSGRenderCustomMaterial::clearDirty()
{
    m_flags &= ~FlagT(Flags::Dirty);
}

void QSSGRenderCustomMaterial::setAlwaysDirty(bool alwaysDirty)
{
    if (alwaysDirty)
        m_flags |= FlagT(Flags::AlwaysDirty);
    else
        m_flags &= ~FlagT(Flags::AlwaysDirty);
}

QT_END_NAMESPACE
