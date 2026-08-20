// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "cuboidgeometry_p.h"

#if QT_CONFIG(concurrent)
#include <QtConcurrentRun>
#endif

QT_BEGIN_NAMESPACE

/*!
    \qmltype CuboidGeometry
    \inqmlmodule QtQuick3D.Helpers
    \inherits Geometry
    \since 6.9
    \brief Provides geometry for a cuboid.

 CuboidGeometry is a geometry type that represents a cuboid. The cuboid's size is
 defined by its xExtent, yExtent, and zExtent properties. The topology of the plane is defined by
 the yzMeshResolution, xzMeshResolution, and xyMeshResolution properties.
*/

/*!
    \qmlproperty real CuboidGeometry::xExtent
    The x extent of the cuboid. The default value is 100.0.
*/

/*!
    \qmlproperty real CuboidGeometry::yExtent
    The y extent of the cuboid. The default value is 100.0.
*/

/*!
    \qmlproperty real CuboidGeometry::zExtent
    The z extent of the cuboid. The default value is 100.0.
*/

/*!
    \qmlproperty size CuboidGeometry::yzMeshResolution
    The number of segments in the y and z direction. The default value is 2x2.
*/

/*!
    \qmlproperty size CuboidGeometry::xzMeshResolution
    The number of segments in the x and z direction. The default value is 2x2.
*/

/*!
    \qmlproperty size CuboidGeometry::xyMeshResolution
    The number of segments in the x and y direction. The default value is 2x2.
*/

/*!
    \qmlproperty bool CuboidGeometry::asynchronous

    This property holds whether the geometry generation should be asynchronous.
*/

/*!
    \qmlproperty bool CuboidGeometry::status
    \readonly

    This property holds the status of the geometry generation when asynchronous is true.

    \value CuboidGeometry.Null The geometry generation has not started
    \value CuboidGeometry.Ready The geometry generation is complete.
    \value CuboidGeometry.Loading The geometry generation is in progress.
    \value CuboidGeometry.Error The geometry generation failed.
*/


CuboidGeometry::CuboidGeometry(QQuick3DObject *parent)
    : QQuick3DGeometry(parent)
{
#if QT_CONFIG(concurrent)
    connect(&m_geometryDataWatcher, &QFutureWatcher<GeometryData>::finished, this, &CuboidGeometry::requestFinished);
#endif
    scheduleGeometryUpdate();
}

CuboidGeometry::~CuboidGeometry()
{

}

float CuboidGeometry::xExtent() const
{
    return m_xExtent;
}

void CuboidGeometry::setXExtent(float newXExtent)
{
    if (qFuzzyCompare(m_xExtent, newXExtent))
        return;
    m_xExtent = newXExtent;
    emit xExtentChanged();
    scheduleGeometryUpdate();
}

float CuboidGeometry::yExtent() const
{
    return m_yExtent;
}

void CuboidGeometry::setYExtent(float newYExtent)
{
    if (qFuzzyCompare(m_yExtent, newYExtent))
        return;
    m_yExtent = newYExtent;
    emit yExtentChanged();
    scheduleGeometryUpdate();
}

float CuboidGeometry::zExtent() const
{
    return m_zExtent;
}

void CuboidGeometry::setZExtent(float newZExtent)
{
    if (qFuzzyCompare(m_zExtent, newZExtent))
        return;
    m_zExtent = newZExtent;
    emit zExtentChanged();
    scheduleGeometryUpdate();
}

QSize CuboidGeometry::yzMeshResolution() const
{
    return m_yzMeshResolution;
}

void CuboidGeometry::setYzMeshResolution(const QSize &newYzMeshResolution)
{
    if (m_yzMeshResolution == newYzMeshResolution)
        return;
    m_yzMeshResolution = newYzMeshResolution;
    emit yzMeshResolutionChanged();
    scheduleGeometryUpdate();
}

QSize CuboidGeometry::xzMeshResolution() const
{
    return m_xzMeshResolution;
}

void CuboidGeometry::setXzMeshResolution(const QSize &newXzMeshResolution)
{
    if (m_xzMeshResolution == newXzMeshResolution)
        return;
    m_xzMeshResolution = newXzMeshResolution;
    emit xzMeshResolutionChanged();
    scheduleGeometryUpdate();
}

QSize CuboidGeometry::xyMeshResolution() const
{
    return m_xyMeshResolution;
}

void CuboidGeometry::setXyMeshResolution(const QSize &newXyMeshResolution)
{
    if (m_xyMeshResolution == newXyMeshResolution)
        return;
    m_xyMeshResolution = newXyMeshResolution;
    emit xyMeshResolutionChanged();
    scheduleGeometryUpdate();
}

bool CuboidGeometry::asynchronous() const
{
    return m_asynchronous;
}

void CuboidGeometry::setAsynchronous(bool newAsynchronous)
{
    if (m_asynchronous == newAsynchronous)
        return;
    m_asynchronous = newAsynchronous;
    emit asynchronousChanged();
}

CuboidGeometry::Status CuboidGeometry::status() const
{
    return m_status;
}

void CuboidGeometry::doUpdateGeometry()
{
    // reset the flag since we are processing the update
    m_geometryUpdateRequested = false;

#if QT_CONFIG(concurrent)
    if (m_geometryDataFuture.isRunning()) {
        m_pendingAsyncUpdate = true;
        return;
    }
#endif



    // Check validity of the geometry parameters
    if (m_xExtent <= 0 || m_yExtent <= 0 || m_yExtent <= 0) {
        clear();
        update();
        return;
    }

#if QT_CONFIG(concurrent)
    if (m_asynchronous) {
        m_geometryDataFuture = QtConcurrent::run(generateCuboidGeometryAsync,
                                                 m_xExtent,
                                                 m_yExtent,
                                                 m_zExtent,
                                                 m_yzMeshResolution,
                                                 m_xzMeshResolution,
                                                 m_xyMeshResolution);
        m_geometryDataWatcher.setFuture(m_geometryDataFuture);
        m_status = Status::Loading;
        Q_EMIT statusChanged();
    } else {
#else
    {

#endif // QT_CONFIG(concurrent)
        updateGeometry(generateCuboidGeometry(m_xExtent,
                                              m_yExtent,
                                              m_zExtent,
                                              m_yzMeshResolution,
                                              m_xzMeshResolution,
                                              m_xyMeshResolution));
    }
}

void CuboidGeometry::requestFinished()
{
#if QT_CONFIG(concurrent)
    const auto output = m_geometryDataFuture.takeResult();
    updateGeometry(output);
#endif
}

void CuboidGeometry::scheduleGeometryUpdate()
{
    if (!m_geometryUpdateRequested) {
        QMetaObject::invokeMethod(this, "doUpdateGeometry", Qt::QueuedConnection);
        m_geometryUpdateRequested = true;
    }
}

void CuboidGeometry::updateGeometry(const GeometryData &geometryData)
{
    setStride(sizeof(float) * 8); // 3 for position, 2 for uv0, 3 for normal
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                 0,
                 QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::TexCoord0Semantic,
                 3 * sizeof(float),
                 QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::NormalSemantic,
                 5 * sizeof(float),
                 QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::IndexSemantic,
                 0,
                 QQuick3DGeometry::Attribute::U16Type);

    setBounds(geometryData.boundsMin, geometryData.boundsMax);
    setVertexData(geometryData.vertexData);
    setIndexData(geometryData.indexData);

           // If the geometry update was requested while the geometry was being generated asynchronously,
           // we need to schedule another geometry update now that the geometry is ready.
    if (m_pendingAsyncUpdate) {
        m_pendingAsyncUpdate = false;
        scheduleGeometryUpdate();
    } else {
        m_status = Status::Ready;
        Q_EMIT statusChanged();
    }
    update();
    emit geometryChanged();
}

CuboidGeometry::GeometryData CuboidGeometry::generateCuboidGeometry(float xExtent, float yExtent, float zExtent, QSize yzMeshResolution, QSize xzMeshResolution, QSize xyMeshResolution)
{
    GeometryData geometryData;

    const float halfXExtent = xExtent / 2.0f;
    const float halfYExtent = yExtent / 2.0f;
    const float halfZExtent = zExtent / 2.0f;
    geometryData.boundsMin = QVector3D(-halfXExtent, -halfYExtent, -halfZExtent);
    geometryData.boundsMax = QVector3D(halfXExtent, halfYExtent, halfZExtent);

    // Total number of vertices and indices required
    int numVertices = (yzMeshResolution.width() + 1) * (yzMeshResolution.height() + 1) * 2 +
            (xzMeshResolution.width() + 1) * (xzMeshResolution.height() + 1) * 2 +
            (xyMeshResolution.width() + 1) * (xyMeshResolution.height() + 1) * 2;

    int numIndices = yzMeshResolution.width() * yzMeshResolution.height() * 6 * 2 +
            xzMeshResolution.width() * xzMeshResolution.height() * 6 * 2 +
            xyMeshResolution.width() * xyMeshResolution.height() * 6 * 2;

    const int vertexStride = sizeof(float) * (3 + 2 + 3); // vec3 (position), vec2 (uv), vec3 (normal)
    const int indexStride = sizeof(uint16_t); // 16-bit index

    // Allocate memory for vertex and index data
    geometryData.vertexData.resize(numVertices * vertexStride);
    geometryData.indexData.resize(numIndices * indexStride);

    // Set up raw pointers for direct memory manipulation
    float* vertexPtr = reinterpret_cast<float*>(geometryData.vertexData.data());
    uint16_t* indexPtr = reinterpret_cast<uint16_t*>(geometryData.indexData.data());

    // Inline lambda for plane generation
    auto generatePlane = [](float* &vertexPtr, uint16_t* &indexPtr, uint16_t &indexOffset, float width, float height, const QVector3D& origin, const QVector3D& right, const QVector3D& up, const QVector3D& normal, QSize resolution, bool flipWinding = false) {
        const int quadsX = resolution.width();
        const int quadsY = resolution.height();
        const float halfWidth = width / 2.0f;
        const float halfHeight = height / 2.0f;
        quint16 vertexCount = 0;

        // Generate vertices
        for (int y = 0; y <= quadsY; ++y) {
            for (int x = 0; x <= quadsX; ++x) {
                // Normalized UV coordinates
                float u = static_cast<float>(x) / quadsX;
                float v = static_cast<float>(y) / quadsY;

                // Compute the position of the vertex
                QVector3D position = origin + right * (u * width - halfWidth) + up * (v * height - halfHeight);

                // Write position
                *vertexPtr++ = position.x();
                *vertexPtr++ = position.y();
                *vertexPtr++ = position.z();

                // Write UV coordinates
                *vertexPtr++ = u;
                *vertexPtr++ = v;

                // Write normal
                *vertexPtr++ = normal.x();
                *vertexPtr++ = normal.y();
                *vertexPtr++ = normal.z();

                ++vertexCount;
            }
        }

        // Generate indices
        for (int y = 0; y < quadsY; ++y) {
            for (int x = 0; x < quadsX; ++x) {
                uint16_t a = indexOffset + static_cast<uint16_t>(y * (quadsX + 1) + x);
                uint16_t b = static_cast<uint16_t>(a + quadsX + 1);
                uint16_t c = static_cast<uint16_t>(b + 1);
                uint16_t d = static_cast<uint16_t>(a + 1);

                if (!flipWinding) {
                    // First triangle (a, d, b)
                    *indexPtr++ = a;
                    *indexPtr++ = d;
                    *indexPtr++ = b;

                    // Second triangle (b, d, c)
                    *indexPtr++ = b;
                    *indexPtr++ = d;
                    *indexPtr++ = c;
                } else {
                    *indexPtr++ = a;
                    *indexPtr++ = b;
                    *indexPtr++ = d;

                    *indexPtr++ = b;
                    *indexPtr++ = c;
                    *indexPtr++ = d;
                }
            }
        }
        indexOffset += vertexCount;
    };

    // Generate the 6 faces of the cuboid
    // Right and Left (YZ plane)
    uint16_t indexOffset = 0;
    generatePlane(vertexPtr, indexPtr, indexOffset, zExtent, yExtent, QVector3D(halfXExtent, 0, 0), QVector3D(0, 0, -1), QVector3D(0, 1, 0), QVector3D(1, 0, 0), yzMeshResolution);
    generatePlane(vertexPtr, indexPtr, indexOffset, zExtent, yExtent, QVector3D(-halfXExtent, 0, 0), QVector3D(0, 0, 1), QVector3D(0, 1, 0), QVector3D(-1, 0, 0), yzMeshResolution);


    // Top and Bottom (XZ plane)
    generatePlane(vertexPtr, indexPtr, indexOffset, xExtent, zExtent, QVector3D(0, halfYExtent, 0), QVector3D(-1, 0, 0), QVector3D(0, 0, 1), QVector3D(0, 1, 0), xzMeshResolution);
    generatePlane(vertexPtr, indexPtr, indexOffset, xExtent, zExtent, QVector3D(0, -halfYExtent, 0), QVector3D(1, 0, 0), QVector3D(0, 0, 1), QVector3D(0, -1, 0), xzMeshResolution);

    // Front and Back (XY plane)
    generatePlane(vertexPtr, indexPtr, indexOffset, xExtent, yExtent, QVector3D(0, 0, halfZExtent), QVector3D(1, 0, 0), QVector3D(0, 1, 0), QVector3D(0, 0, 1), xyMeshResolution);
    generatePlane(vertexPtr, indexPtr, indexOffset, xExtent, yExtent, QVector3D(0, 0, -halfZExtent), QVector3D(-1, 0, 0), QVector3D(0, 1, 0), QVector3D(0, 0, -1), xyMeshResolution);

    // Return the geometry data
    return geometryData;
}

#if QT_CONFIG(concurrent)
void CuboidGeometry::generateCuboidGeometryAsync(QPromise<CuboidGeometry::GeometryData> &promise,
                                                 float xExtent,
                                                 float yExtent,
                                                 float zExtent,
                                                 QSize yzMeshResolution,
                                                 QSize xzMeshResolution,
                                                 QSize xyMeshResolution)
{
    auto output = generateCuboidGeometry(xExtent, yExtent, zExtent, yzMeshResolution, xzMeshResolution, xyMeshResolution);
    promise.addResult(output);
}
#endif

QT_END_NAMESPACE
