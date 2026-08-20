// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default



#include <QtQuick3DRuntimeRender/private/qssgrendermorphtarget_p.h>

QT_BEGIN_NAMESPACE

QSSGRenderMorphTarget::QSSGRenderMorphTarget()
    : QSSGRenderGraphObject(QSSGRenderGraphObject::Type::MorphTarget)
{
}

QSSGRenderMorphTarget::~QSSGRenderMorphTarget() = default;

QT_END_NAMESPACE
