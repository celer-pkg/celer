// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CONVEXMESHSHAPE_H
#define CONVEXMESHSHAPE_H

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

#include "qmeshshape_p.h"

QT_BEGIN_NAMESPACE

class Q_QUICK3DPHYSICS_EXPORT QConvexMeshShape : public QMeshShape
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ConvexMeshShape)
    virtual QMeshShape::MeshType shapeType() const override;
    virtual bool isStaticShape() const override;
};

QT_END_NAMESPACE

#endif // CONVEXMESHSHAPE_H
