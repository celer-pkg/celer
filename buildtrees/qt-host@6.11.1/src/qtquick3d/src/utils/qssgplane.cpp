// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgplane_p.h"

QT_BEGIN_NAMESPACE

namespace {
float magnitude(const QVector3D &vector)
{
    return std::sqrt(vector.x() * vector.x() + vector.y() * vector.y() + vector.z() * vector.z());
}
}

void QSSGPlane::normalize()
{
    float denom = 1.0f / magnitude(n);
    n *= denom;
    d *= denom;
}

QT_END_NAMESPACE
