// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_IMAGE_H
#define QSSG_RENDER_IMAGE_H

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

#include <QtQuick3DRuntimeRender/private/qssgrendergraphobject_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderimagetexture_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>
#include <QtQuick3DRuntimeRender/private/qtquick3druntimerenderglobal_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendererutil_p.h>
#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>

#include <QtGui/QVector2D>

QT_BEGIN_NAMESPACE
class QSSGRenderContextInterface;
class QSGTexture;
class QSSGRenderTextureData;

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderImage : public QSSGRenderGraphObject
{
    enum class Flag
    {
        Dirty = 1 << 0,
        TransformDirty = 1 << 1,
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    enum class MappingModes : quint8
    {
        Normal = 0, // UV mapping
        Environment = 1,
        LightProbe = 2,
    };

    Q_DISABLE_COPY(QSSGRenderImage)

    QSSGRenderGraphObject *m_parent = nullptr;

    QSSGRenderPath m_imagePath;
    // The QSGTexture (from sourceItem) is not sharable between Qt Quick render
    // threads, when the threaded render loop is in use. That's why we allow
    // this exception here; the (per-QQuickWindow, and so per-render-thread)
    // BufferManager will refuse to use this if the threads don't match.
    QSGTexture *m_qsgTexture = nullptr; // overrides m_imagePath and m_rawTextureData when non-null
    QSSGRenderTextureData *m_rawTextureData = nullptr; // overrides m_imagePath and m_qsgTexture when non-null
    QSSGRenderExtension *m_extensionsSource = nullptr;

    Flags m_flags;

    QVector2D m_scale { 1.0f, 1.0f };
    QVector2D m_pivot { 0.0f, 0.0f };
    QVector2D m_position { 0.0f, 0.0f };
    float m_rotation = 0.0f; // degrees
    bool m_flipU = false;
    bool m_flipV = false;
    int m_indexUV = 0;
    MappingModes m_mappingMode = MappingModes::Normal;
    QSSGRenderTextureCoordOp m_horizontalTilingMode = QSSGRenderTextureCoordOp::Repeat;
    QSSGRenderTextureCoordOp m_verticalTilingMode = QSSGRenderTextureCoordOp::Repeat;
    QSSGRenderTextureCoordOp m_depthTilingMode = QSSGRenderTextureCoordOp::Repeat;
    QSSGRenderTextureFilterOp m_magFilterType = QSSGRenderTextureFilterOp::Linear;
    QSSGRenderTextureFilterOp m_minFilterType = QSSGRenderTextureFilterOp::Linear;
    QSSGRenderTextureFilterOp m_mipFilterType = QSSGRenderTextureFilterOp::Linear;
    QSSGRenderTextureFormat m_format = QSSGRenderTextureFormat::Unknown;
    bool m_generateMipmaps = false;

    // Changing any of the above variables is covered by the Dirty flag, while
    // the texture transform is covered by TransformDirty.
    QMatrix4x4 m_textureTransform;

    QSSGRenderImage(QSSGRenderGraphObject::Type type = QSSGRenderGraphObject::Type::Image2D);
    ~QSSGRenderImage();

    bool clearDirty();
    void calculateTextureTransform();
    bool isImageTransformIdentity() const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSSGRenderImage::Flags)

QT_END_NAMESPACE

#endif
