// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDERABLE_IMAGE_H
#define QSSG_RENDERABLE_IMAGE_H

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

#include <QtQuick3DRuntimeRender/private/qtquick3druntimerenderglobal_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderimage_p.h>

QT_BEGIN_NAMESPACE

// Some precomputed information on a given image. When generating a renderable,
// the layer preparation step goes through all the possible images on a
// material (which includes all regular texture maps, but does not include
// light probes or custom texture properties for custom materials), and for
// each valid image it generates, if not already done, the QRhiTexture (for the
// current scene's window, and so render thread), and calculates some other
// data and flags.

struct QSSGRenderableImage
{
    enum class Type : quint8 {
        Unknown = 0,
        Diffuse,
        Opacity,
        Specular,
        Emissive,
        Bump,
        SpecularAmountMap,
        Normal,
        Translucency,
        Roughness,
        BaseColor,
        Metalness,
        Occlusion,
        Height,
        Clearcoat,
        ClearcoatRoughness,
        ClearcoatNormal,
        Transmission,
        Thickness
    };
    const QSSGRenderImage &m_imageNode;
    QSSGRenderImageTexture m_texture;
    QSSGRenderableImage *m_nextImage;
    Type m_mapType;
    QSSGRenderableImage(Type inMapType, const QSSGRenderImage &inImageNode, const QSSGRenderImageTexture &inTexture)
        : m_imageNode(inImageNode), m_texture(inTexture), m_nextImage(nullptr),  m_mapType(inMapType)
    {
    }
};
QT_END_NAMESPACE
#endif
