// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "torusgeometry_p.h"
#include <limits>

#if QT_CONFIG(concurrent)
#include <QtConcurrentRun>
#endif

QT_BEGIN_NAMESPACE

/*!
    \qmltype TorusGeometry
    \inqmlmodule QtQuick3D.Helpers
    \inherits Geometry
    \since 6.9
    \brief Provides geometry for a torus.

    TorusGeometry is a geometry type that represents a torus. The torus is defined by the number of
    rings and segments, and the radius of the torus.

*/

/*!
    \qmlproperty int TorusGeometry::rings
    Specifies the number of rings in the torus. The default value is 50.
*/

/*!
    \qmlproperty int TorusGeometry::segments
    Specifies the number of segments in the torus. The default value is 50.
*/

/*!
    \qmlproperty real TorusGeometry::radius
    Specifies the radius of the torus. The default value is 100.
*/

/*!
    \qmlproperty real TorusGeometry::tubeRadius
    Specifies the radius of the tube of the torus. The default value is 10.
*/

/*!
    \qmlproperty bool TorusGeometry::asynchronous

    This property holds whether the geometry generation should be asynchronous.
*/

/*!
    \qmlproperty bool TorusGeometry::status
    \readonly

    This property holds the status of the geometry generation when asynchronous is true.

    \value TorusGeometry.Null The geometry generation has not started
    \value TorusGeometry.Ready The geometry generation is complete.
    \value TorusGeometry.Loading The geometry generation is in progress.
    \value TorusGeometry.Error The geometry generation failed.
*/

TorusGeometry::TorusGeometry(QQuick3DObject *parent)
    : QQuick3DGeometry(parent)
{
#if QT_CONFIG(concurrent)
    connect(&m_geometryDataWatcher, &QFutureWatcher<GeometryData>::finished, this, &TorusGeometry::requestFinished);
#endif
    scheduleGeometryUpdate();
}

TorusGeometry::~TorusGeometry()
{

}

int TorusGeometry::rings() const
{
    return m_rings;
}

void TorusGeometry::setRings(int newRings)
{
    if (m_rings == newRings)
        return;
    m_rings = newRings;
    emit ringsChanged();
    scheduleGeometryUpdate();
}

int TorusGeometry::segments() const
{
    return m_segments;
}

void TorusGeometry::setSegments(int newSegments)
{
    if (m_segments == newSegments)
        return;
    m_segments = newSegments;
    emit segmentsChanged();
    scheduleGeometryUpdate();
}

float TorusGeometry::radius() const
{
    return m_radius;
}

void TorusGeometry::setRadius(float newRadius)
{
    if (qFuzzyCompare(m_radius, newRadius))
        return;
    m_radius = newRadius;
    emit radiusChanged();
    scheduleGeometryUpdate();
}

float TorusGeometry::tubeRadius() const
{
    return m_tubeRadius;
}

void TorusGeometry::setTubeRadius(float newTubeRadius)
{
    if (qFuzzyCompare(m_tubeRadius, newTubeRadius))
        return;
    m_tubeRadius = newTubeRadius;
    emit tubeRadiusChanged();
    scheduleGeometryUpdate();
}

bool TorusGeometry::asynchronous() const
{
    return m_asynchronous;
}

void TorusGeometry::setAsynchronous(bool newAsynchronous)
{
    if (m_asynchronous == newAsynchronous)
        return;
    m_asynchronous = newAsynchronous;
    emit asynchronousChanged();
}

TorusGeometry::Status TorusGeometry::status() const
{
    return m_status;
}

void TorusGeometry::doUpdateGeometry()
{
    // reset the flag since we are processing the update
    m_geometryUpdateRequested = false;

#if QT_CONFIG(concurrent)
    if (m_geometryDataFuture.isRunning()) {
        m_pendingAsyncUpdate = true;
        return;
    }
#endif

    // Check for validity of the parameters before generation
    if (m_rings <= 0 || m_segments <= 0 || m_radius <= 0 || m_tubeRadius <= 0) {
        clear();
        update();
        return;
    }

#if QT_CONFIG(concurrent)

    if (m_asynchronous) {
        m_geometryDataFuture = QtConcurrent::run(generateTorusGeometryAsync,
                                                 m_rings,
                                                 m_segments,
                                                 m_radius,
                                                 m_tubeRadius);
        m_geometryDataWatcher.setFuture(m_geometryDataFuture);
        m_status = Status::Loading;
        Q_EMIT statusChanged();
    } else {
#else
    {

#endif // QT_CONFIG(concurrent)
        updateGeometry(generateTorusGeometry(m_rings, m_segments, m_radius, m_tubeRadius));
    }
}

void TorusGeometry::requestFinished()
{
#if QT_CONFIG(concurrent)
    const auto output = m_geometryDataFuture.takeResult();
    updateGeometry(output);
#endif
}

void TorusGeometry::scheduleGeometryUpdate()
{
    if (!m_geometryUpdateRequested) {
        QMetaObject::invokeMethod(this, "doUpdateGeometry", Qt::QueuedConnection);
        m_geometryUpdateRequested = true;
    }
}

void TorusGeometry::updateGeometry(const GeometryData &geometryData)
{
    setStride(sizeof(float) * 8); // 3 for position, 3 for normal, 2 for uv0
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                 0,
                 QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::NormalSemantic,
                 3 * sizeof(float),
                 QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::TexCoord0Semantic,
                 6 * sizeof(float),
                 QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::IndexSemantic,
                 0,
                 QQuick3DGeometry::Attribute::U32Type);

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

const size_t FLOAT_SIZE = sizeof(float);
const size_t VEC3_SIZE = sizeof(QVector3D);
const size_t VEC2_SIZE = sizeof(QVector2D);
const size_t UINT_SIZE = sizeof(quint32);

TorusGeometry::GeometryData TorusGeometry::generateTorusGeometry(int rings, int segments, float radius, float tubeRadius)
{
    GeometryData geomData;

    // Bounds initialization
    QVector3D boundsMin(std::numeric_limits<float>::max(),
                        std::numeric_limits<float>::max(),
                        std::numeric_limits<float>::max());

    QVector3D boundsMax(std::numeric_limits<float>::lowest(),
                        std::numeric_limits<float>::lowest(),
                        std::numeric_limits<float>::lowest());

    // Pre-calculate the number of vertices and indices
    int numVerts = (rings + 1) * (segments + 1);
    int numIndices = rings * segments * 6; // Two triangles per quad

    // Reserve space in the QByteArrays (vertex data: position (3 floats), normal (3 floats), UV (2 floats))
    int vertexDataSize = numVerts * (VEC3_SIZE + VEC3_SIZE + VEC2_SIZE); // Position, Normal, UV
    geomData.vertexData.resize(vertexDataSize);

    // Indices are stored as unsigned 32-bit integers
    int indexDataSize = numIndices * UINT_SIZE;
    geomData.indexData.resize(indexDataSize);

    // Get raw pointers to the QByteArray data
    char* vertexPtr = geomData.vertexData.data();
    char* indexPtr = geomData.indexData.data();

    int vertexOffset = 0;
    int indexOffset = 0;

    // Iterate over the rings and segments to compute vertices, normals, and UVs
    for (int i = 0; i <= rings; ++i) {
        for (int j = 0; j <= segments; ++j) {
            float u = (i / (float)rings) * M_PI * 2;
            float v = (j / (float)segments) * M_PI * 2;

            float centerX = radius * qCos(u);
            float centerZ = radius * qSin(u);

            float posX = centerX + tubeRadius * qCos(v) * qCos(u);
            float posY = tubeRadius * qSin(v);
            float posZ = centerZ + tubeRadius * qCos(v) * qSin(u);

            // Update bounds
            boundsMin.setX(qMin(boundsMin.x(), posX));
            boundsMin.setY(qMin(boundsMin.y(), posY));
            boundsMin.setZ(qMin(boundsMin.z(), posZ));

            boundsMax.setX(qMax(boundsMax.x(), posX));
            boundsMax.setY(qMax(boundsMax.y(), posY));
            boundsMax.setZ(qMax(boundsMax.z(), posZ));

            // Position data (3 floats)
            memcpy(vertexPtr + vertexOffset, &posX, FLOAT_SIZE);
            memcpy(vertexPtr + vertexOffset + FLOAT_SIZE, &posY, FLOAT_SIZE);
            memcpy(vertexPtr + vertexOffset + 2 * FLOAT_SIZE, &posZ, FLOAT_SIZE);
            vertexOffset += 3 * FLOAT_SIZE;

            // Calculate normal
            QVector3D normal = QVector3D(posX - centerX, posY, posZ - centerZ).normalized();

            // Normal data (3 floats)
            memcpy(vertexPtr + vertexOffset, &normal, VEC3_SIZE);
            vertexOffset += VEC3_SIZE;

            // UV data (2 floats)
            float uvX = 1.0f - (i / (float)rings);
            float uvY = j / (float)segments;
            memcpy(vertexPtr + vertexOffset, &uvX, FLOAT_SIZE);
            memcpy(vertexPtr + vertexOffset + FLOAT_SIZE, &uvY, FLOAT_SIZE);
            vertexOffset += 2 * FLOAT_SIZE;
        }
    }

    // Generate indices
    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < segments; ++j) {
            int a = (segments + 1) * i + j;
            int b = (segments + 1) * (i + 1) + j;
            int c = (segments + 1) * (i + 1) + j + 1;
            int d = (segments + 1) * i + j + 1;

            // First triangle (a, d, b)
            memcpy(indexPtr + indexOffset, &a, UINT_SIZE);
            memcpy(indexPtr + indexOffset + UINT_SIZE, &d, UINT_SIZE);
            memcpy(indexPtr + indexOffset + 2 * UINT_SIZE, &b, UINT_SIZE);
            indexOffset += 3 * UINT_SIZE;

            // Second triangle (b, d, c)
            memcpy(indexPtr + indexOffset, &b, UINT_SIZE);
            memcpy(indexPtr + indexOffset + UINT_SIZE, &d, UINT_SIZE);
            memcpy(indexPtr + indexOffset + 2 * UINT_SIZE, &c, UINT_SIZE);
            indexOffset += 3 * UINT_SIZE;
        }
    }

    // Set the computed bounds
    geomData.boundsMin = boundsMin;
    geomData.boundsMax = boundsMax;

    return geomData;
}

#if QT_CONFIG(concurrent)
void TorusGeometry::generateTorusGeometryAsync(QPromise<TorusGeometry::GeometryData> &promise, int rings, int segments, float radius, float tubeRadius)
{
    auto output = generateTorusGeometry(rings, segments, radius, tubeRadius);
    promise.addResult(output);
}
#endif

QT_END_NAMESPACE
