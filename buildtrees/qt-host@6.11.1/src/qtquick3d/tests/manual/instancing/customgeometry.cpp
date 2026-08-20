// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "customgeometry.h"

#include <qmath.h>
#include <QRandomGenerator>
#include <QColor>

//struct Vertex {
//    QVector3D position;
////    qint32 joints[4];
////    float weights[4];

//    float pad;
//};

//static const int s_vertexSize = sizeof(Vertex);
//Q_STATIC_ASSERT(s_vertexSize == 16);

CustomGeometry::CustomGeometry(QQuick3DObject *parent)
    : QQuick3DGeometry(parent)
{
    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                 QQuick3DGeometry::Attribute::ComponentType::F32Type);

    addAttribute(QQuick3DGeometry::Attribute::IndexSemantic, 0,
                 QQuick3DGeometry::Attribute::ComponentType::U32Type);

    addAttribute(QQuick3DGeometry::Attribute::ColorSemantic, 3 * sizeof(float),
                 QQuick3DGeometry::Attribute::ComponentType::F32Type);

//    addAttribute(QQuick3DGeometry::Attribute::JointSemantic, offsetof(Vertex, joints[0]),
//                 QQuick3DGeometry::Attribute::ComponentType::I32Type);
//    addAttribute(QQuick3DGeometry::Attribute::WeightSemantic, offsetof(Vertex, weights[0]),
//                 QQuick3DGeometry::Attribute::ComponentType::F32Type);
}

QList<QVector3D> CustomGeometry::positions() const
{
    return m_positions;
}

QList<qint32> CustomGeometry::joints() const
{
    return m_joints;
}

QList<float> CustomGeometry::weights() const
{
    return m_weights;
}

QList<quint32> CustomGeometry::indexes() const
{
    return m_indexes;
}

void CustomGeometry::setPositions(const QList<QVector3D> &positions)
{
    if (positions == m_positions)
        return;
    m_positions = positions;
    emit positionsChanged();
    m_vertexDirty = true;
}

void CustomGeometry::setJoints(const QList<qint32> &joints)
{
    if (joints == m_joints)
        return;
    m_joints = joints;
    emit jointsChanged();
    m_vertexDirty = true;
}

void CustomGeometry::setWeights(const QList<float> &weights)
{
    if (weights == m_weights)
        return;
    m_weights = weights;
    emit weightsChanged();
    m_vertexDirty = true;
}

void CustomGeometry::setIndexes(const QList<quint32> &indexes)
{
    if (indexes == m_indexes)
        return;
    m_indexes = indexes;
    emit indexesChanged();
    m_indexDirty = true;
}

#if 0
QVector3D cubeVertices[8] =
{
    QVector3D(-1, -1, -1),
    QVector3D(1, -1, -1),
    QVector3D(1, 1, -1),
    QVector3D(-1, 1, -1),
    QVector3D(-1, -1, 1),
    QVector3D(1, -1, 1),
    QVector3D(1, 1, 1),
    QVector3D(-1, 1, 1)
};

int cubeIndices[6 * 6] =
{
    0, 1, 3, 3, 1, 2,
    1, 5, 2, 2, 5, 6,
    5, 4, 6, 6, 4, 7,
    4, 0, 7, 7, 0, 3,
    3, 2, 7, 7, 2, 6,
    4, 5, 0, 0, 5, 1
};
constexpr int vertsPerCube = 8;
constexpr int trianglesPerCube = 6*2;
#else
QVector3D cubeVertices[6] =
{
    QVector3D(0, -1, 0),
    QVector3D(-1, 0, -1),
    QVector3D(-1, 0, 1),
    QVector3D(1, 0, 1),
    QVector3D(1, 0, -1),
    QVector3D(0, 1, 0)
};
constexpr int vertsPerCube = 6;
constexpr int trianglesPerCube = 8;

int cubeIndices[trianglesPerCube*3] =
{
    0,4,1, 0,3,4, 0,2,3, 0,1,2,
    5,1,4, 5,4,3, 5,3,2, 5,2,1
};
#endif

constexpr int numCubes = 1;

constexpr int floatsPerVert = 8;
constexpr qsizetype vertexBufferSizeInBytes = numCubes * vertsPerCube * floatsPerVert * sizeof(float);

constexpr qsizetype indexBufferSizeInBytes = numCubes * sizeof(cubeIndices);

QSSGRenderGraphObject *CustomGeometry::updateSpatialNode(QSSGRenderGraphObject *node)
{
#if 1
    if (m_vertexDirty) {
        QByteArray iBuffer;
        iBuffer.resize(indexBufferSizeInBytes);

        QByteArray vBuffer;
        vBuffer.resize(vertexBufferSizeInBytes);
        vBuffer.fill(0);

        int *iData = reinterpret_cast<int*>(iBuffer.data());
        float *vData = reinterpret_cast<float*>(vBuffer.data());
        QRandomGenerator *rgen = QRandomGenerator::global();
        for (int c = 0; c < numCubes; ++c) {

            auto rand = [rgen]() {return rgen->bounded(10000)/50.0 - 100; };

            QColor cubeColor(rgen->bounded(255), rgen->bounded(255), rgen->bounded(255));
            QVector3D offs = numCubes == 1 ? QVector3D{} : QVector3D(2*rand(), rand(), 2*rand());
            float scale = numCubes == 1 ? 1.0 : rgen->bounded(100)/100.0;
            scale = scale * 10;
            //scale = scale/3.0;
            //.............
            for (int i = 0; i < vertsPerCube; ++i) {
                int idx = i * floatsPerVert + c * vertsPerCube * floatsPerVert;
                QVector3D vertex = cubeVertices[i] * scale * -1 + offs; // our polygons' faces are oriented wrong
                //qDebug() << "cube" << c << "vert" << i << vertex;
                vData[idx] = vertex.x();
                vData[idx+1] = vertex.y();
                vData[idx+2] = vertex.z();

                vData[idx+3] = cubeColor.redF();
                vData[idx+4] = cubeColor.greenF();
                vData[idx+5] = cubeColor.blueF();
                vData[idx+6] = 1.0;
            }

            //qDebug() << "iData" << indexBufferSizeInBytes << iBuffer.size();
            constexpr int idxPerCube = trianglesPerCube * 3;
            for (int i = 0; i < trianglesPerCube*3; ++i) {
                //qDebug() << "trying:" << QString::number(i) << i + c*idxpercube << "of" << indexBufferSizeInBytes/sizeof(float);
                iData[i + c * idxPerCube] = cubeIndices[i] + c * vertsPerCube;
             }

        }

        setStride(floatsPerVert * sizeof(float));
        setVertexData(vBuffer);
        setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
        setIndexData(iBuffer);
        m_vertexDirty = false;
    }
#else
    if (m_vertexDirty) {
        m_vertexDirty = false;

        const int numVertexes = m_positions.size();
        m_vertexBuffer.resize(numVertexes * s_vertexSize);
        Vertex *vert = reinterpret_cast<Vertex *>(m_vertexBuffer.data());

        for (int i = 0; i < numVertexes; ++i) {
            Vertex &v = vert[i];
            v.position = m_positions[i];
        }

        setStride(s_vertexSize);
        setVertexData(m_vertexBuffer);
        setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    }

    if (m_indexDirty) {
        m_indexDirty = false;
        m_indexBuffer = QByteArray(reinterpret_cast<char *>(m_indexes.data()), m_indexes.size() * sizeof(quint32));
        setIndexData(m_indexBuffer);
    }
#endif
    node = QQuick3DGeometry::updateSpatialNode(node);
    return node;
}
