// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "examplegeometry.h"
#include <QtGui/QVector3D>
#include <array>

ExampleTriangleGeometry::ExampleTriangleGeometry()
{
    constexpr int kStride = sizeof(QVector3D);
    QByteArray vertexData(36 * kStride, Qt::Initialization::Uninitialized);
    QVector3D *p = reinterpret_cast<QVector3D *>(vertexData.data());

    *p = QVector3D(-50.0f,-50.0f,-50.0f); p++;
    *p = QVector3D(-50.0f,-50.0f, 50.0f); p++;
    *p = QVector3D(-50.0f, 50.0f, 50.0f); p++;
    *p = QVector3D(+50.0f, 50.0f,-50.0f); p++;
    *p = QVector3D(-50.0f,-50.0f,-50.0f); p++;
    *p = QVector3D(-50.0f, 50.0f,-50.0f); p++;
    *p = QVector3D(+50.0f,-50.0f, 50.0f); p++;
    *p = QVector3D(-50.0f,-50.0f,-50.0f); p++;
    *p = QVector3D(+50.0f,-50.0f,-50.0f); p++;
    *p = QVector3D(+50.0f, 50.0f,-50.0f); p++;
    *p = QVector3D(+50.0f,-50.0f,-50.0f); p++;
    *p = QVector3D(-50.0f,-50.0f,-50.0f); p++;
    *p = QVector3D(-50.0f,-50.0f,-50.0f); p++;
    *p = QVector3D(-50.0f, 50.0f, 50.0f); p++;
    *p = QVector3D(-50.0f, 50.0f,-50.0f); p++;
    *p = QVector3D(+50.0f,-50.0f, 50.0f); p++;
    *p = QVector3D(-50.0f,-50.0f, 50.0f); p++;
    *p = QVector3D(-50.0f,-50.0f,-50.0f); p++;
    *p = QVector3D(-50.0f, 50.0f, 50.0f); p++;
    *p = QVector3D(-50.0f,-50.0f, 50.0f); p++;
    *p = QVector3D(+50.0f,-50.0f, 50.0f); p++;
    *p = QVector3D(+50.0f, 50.0f, 50.0f); p++;
    *p = QVector3D(+50.0f,-50.0f,-50.0f); p++;
    *p = QVector3D(+50.0f, 50.0f,-50.0f); p++;
    *p = QVector3D(+50.0f,-50.0f,-50.0f); p++;
    *p = QVector3D(+50.0f, 50.0f, 50.0f); p++;
    *p = QVector3D(+50.0f,-50.0f, 50.0f); p++;
    *p = QVector3D(+50.0f, 50.0f, 50.0f); p++;
    *p = QVector3D(+50.0f, 50.0f,-50.0f); p++;
    *p = QVector3D(-50.0f, 50.0f,-50.0f); p++;
    *p = QVector3D(+50.0f, 50.0f, 50.0f); p++;
    *p = QVector3D(-50.0f, 50.0f,-50.0f); p++;
    *p = QVector3D(-50.0f, 50.0f, 50.0f); p++;
    *p = QVector3D(+50.0f, 50.0f, 50.0f); p++;
    *p = QVector3D(-50.0f, 50.0f, 50.0f); p++;
    *p = QVector3D(+50.0f,-50.0f, 50.0f); p++;

    setVertexData(vertexData);
    setStride(kStride);
    setBounds(QVector3D(-50.0f, -50.0f, -50.0f), QVector3D(+50.0f, +50.0f, +50.0f));
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                 0,
                 QQuick3DGeometry::Attribute::F32Type);
}
