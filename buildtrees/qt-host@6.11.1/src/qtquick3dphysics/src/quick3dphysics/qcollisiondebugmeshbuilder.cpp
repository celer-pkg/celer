// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcollisiondebugmeshbuilder_p.h"

QT_BEGIN_NAMESPACE

QCollisionDebugMeshBuilder::QCollisionDebugMeshBuilder() { }

void QCollisionDebugMeshBuilder::reset()
{
    m_vertices.clear();
    m_normals.clear();
}

void QCollisionDebugMeshBuilder::addLine(const QVector3D &start, const QVector3D &end,
                                         const QVector3D &normal)
{
    m_vertices.append(start);
    m_vertices.append(end);
    m_normals.append(normal);
}

QByteArray QCollisionDebugMeshBuilder::generateVertexArray()
{
    QByteArray output;
    const int size = m_normals.count();
    output.resize(4 * 4 * sizeof(float) * size);

    float *data = reinterpret_cast<float *>(output.data());

    for (int i = 0; i < size; ++i) {
        const QVector3D &start = m_vertices[i * 2];
        const QVector3D &end = m_vertices[i * 2 + 1];
        const QVector3D &normal = m_normals[i];

        data[0] = start.x();
        data[1] = start.y();
        data[2] = start.z();
        data[4] = 1.0f;

        data[5] = normal.x();
        data[6] = normal.y();
        data[7] = normal.z();
        data[8] = 0.0f;

        data += 8;
        data[0] = end.x();
        data[1] = end.y();
        data[2] = end.z();
        data[4] = 1.0f;

        data[5] = normal.x();
        data[6] = normal.y();
        data[7] = normal.z();
        data[8] = 0.0f;

        data += 8;
    }

    return output;
}

QT_END_NAMESPACE
