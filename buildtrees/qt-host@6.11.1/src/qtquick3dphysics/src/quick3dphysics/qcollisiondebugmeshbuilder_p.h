// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef COLLISIONDEBUGMESHBUILDER_H
#define COLLISIONDEBUGMESHBUILDER_H

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

#include <QtCore/QVector>
#include <QtGui/QVector3D>

QT_BEGIN_NAMESPACE

class QCollisionDebugMeshBuilder
{
public:
    QCollisionDebugMeshBuilder();

    void reset();

    void addLine(const QVector3D &start, const QVector3D &end,
                 const QVector3D &normal = QVector3D(0, 0, 1));

    QByteArray generateVertexArray();

private:
    QVector<QVector3D> m_vertices;
    QVector<QVector3D> m_normals;
};

QT_END_NAMESPACE

#endif // COLLISIONDEBUGMESHBUILDER_H
