// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef MESHLOADER_P_H
#define MESHLOADER_P_H

#include "datavisualizationglobal_p.h"
#include <QtGui/QVector2D>

QT_BEGIN_NAMESPACE

class MeshLoader
{
    public:
        static bool loadOBJ(const QString &path, QList<QVector3D> &out_vertices,
                            QList<QVector2D> &out_uvs, QList<QVector3D> &out_normals);
};

QT_END_NAMESPACE

#endif
