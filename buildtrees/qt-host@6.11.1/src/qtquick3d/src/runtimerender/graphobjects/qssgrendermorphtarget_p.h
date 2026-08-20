// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_MORPH_TARGET_H
#define QSSG_RENDER_MORPH_TARGET_H

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
#include <QtQuick3DRuntimeRender/private/qssgrendererutil_p.h>

#include <QtCore/QVector>

QT_BEGIN_NAMESPACE

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderMorphTarget : public QSSGRenderGraphObject
{
    enum class InputAttribute : quint8
    {
        Position   = 0x01,
        Normal     = 0x02,
        Tangent    = 0x04,
        Binormal   = 0x08,
        TexCoord0  = 0x10,
        TexCoord1  = 0x20,
        Color      = 0x40
    };
    Q_DECLARE_FLAGS(InputAttributes, InputAttribute);

    float weight = 0.0f;
    InputAttributes attributes;

    QSSGRenderMorphTarget();
    ~QSSGRenderMorphTarget();
};
QT_END_NAMESPACE

#endif
