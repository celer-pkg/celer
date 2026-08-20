// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtQuick3DRuntimeRender/private/qssgrenderimage_p.h>
#include <QtQuick/QSGTexture>

QT_BEGIN_NAMESPACE

QSSGRenderImage::QSSGRenderImage(QSSGRenderGraphObject::Type type)
    : QSSGRenderGraphObject(type, FlagT(QSSGRenderGraphObject::Flags::HasGraphicsResources))
{
    m_flags.setFlag(Flag::Dirty);
    m_flags.setFlag(Flag::TransformDirty);
}

QSSGRenderImage::~QSSGRenderImage() = default;

bool QSSGRenderImage::clearDirty()
{
    const bool wasDirty = m_flags.testFlag(Flag::Dirty);
    m_flags.setFlag(Flag::Dirty, false);
    if (m_flags.testFlag(Flag::TransformDirty)) {
        calculateTextureTransform();
        return true;
    }
    return wasDirty;
}

void QSSGRenderImage::calculateTextureTransform()
{
    m_flags.setFlag(Flag::TransformDirty, false);

    m_textureTransform = QMatrix4x4();
    if (m_flipU) {
        m_textureTransform *= QMatrix4x4(-1.f, 0.f, 0.f, 1.f,
                                         0.f,  1.f, 0.f, 0.f,
                                         0.f,  0.f, 1.f, 0.f,
                                         0.f,  0.f, 0.f, 1.f);
    }
    if (m_flipV) {
        m_textureTransform *= QMatrix4x4(1.f,  0.f, 0.f, 0.f,
                                         0.f, -1.f, 0.f, 1.f,
                                         0.f,  0.f, 1.f, 0.f,
                                         0.f,  0.f, 0.f, 1.f);
    }

    QMatrix4x4 pivot;
    QMatrix4x4 pivot_r;
    QMatrix4x4 translation;
    QMatrix4x4 rotation;
    QMatrix4x4 scale;

    pivot.translate(m_pivot.x(), m_pivot.y());
    pivot_r.translate(-m_pivot.x(), -m_pivot.y());
    translation.translate(m_position.x(), m_position.y());
    scale.scale(m_scale.x(), m_scale.y());
    rotation.rotate(m_rotation, QVector3D(0, 0, 1));

    m_textureTransform *= translation;
    m_textureTransform *= pivot;
    m_textureTransform *= rotation;
    m_textureTransform *= scale;
    m_textureTransform *= pivot_r;
}

bool QSSGRenderImage::isImageTransformIdentity() const
{
    if (m_mappingMode != MappingModes::Normal)
        return false;
    return m_textureTransform.isIdentity();
}

QT_END_NAMESPACE
