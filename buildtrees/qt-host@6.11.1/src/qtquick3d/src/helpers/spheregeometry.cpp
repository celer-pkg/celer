// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "spheregeometry_p.h"
#include <limits>

#if QT_CONFIG(concurrent)
#include <QtConcurrentRun>
#endif

QT_BEGIN_NAMESPACE

/*!
    \qmltype SphereGeometry
    \inqmlmodule QtQuick3D.Helpers
    \inherits Geometry
    \since 6.9
    \brief Provides geometry for a sphere.

    SphereGeometry is a geometry type that represents a sphere. The sphere's size is
    defined by its radius. The topology of the sphere is defined by the number of
    rings and segments.
*/

/*!
    \qmlproperty real SphereGeometry::radius
    The radius of the sphere. The default value is 100.0.
*/

/*!
    \qmlproperty int SphereGeometry::rings
    The number of rings in the sphere. The default value is 16.
*/

/*!
    \qmlproperty int SphereGeometry::segments
    The number of segments in the sphere. The default value is 32.
*/

/*!
    \qmlproperty bool SphereGeometry::asynchronous

    This property holds whether the geometry generation should be asynchronous.
*/

/*!
    \qmlproperty bool SphereGeometry::status
    \readonly

    This property holds the status of the geometry generation when asynchronous is true.

    \value SphereGeometry.Null The geometry generation has not started
    \value SphereGeometry.Ready The geometry generation is complete.
    \value SphereGeometry.Loading The geometry generation is in progress.
    \value SphereGeometry.Error The geometry generation failed.
*/

SphereGeometry::SphereGeometry(QQuick3DObject *parent)
    : QQuick3DGeometry(parent)
{
#if QT_CONFIG(concurrent)
    connect(&m_geometryDataWatcher, &QFutureWatcher<GeometryData>::finished, this, &SphereGeometry::requestFinished);
#endif
    scheduleGeometryUpdate();
}

SphereGeometry::~SphereGeometry()
{

}

float SphereGeometry::radius() const
{
    return m_radius;
}

void SphereGeometry::setRadius(float newRadius)
{
    if (qFuzzyCompare(m_radius, newRadius))
        return;
    m_radius = newRadius;
    emit radiusChanged();
    scheduleGeometryUpdate();
}

int SphereGeometry::rings() const
{
    return m_rings;
}

void SphereGeometry::setRings(int newRings)
{
    if (m_rings == newRings)
        return;
    m_rings = newRings;
    emit ringsChanged();
    scheduleGeometryUpdate();
}

int SphereGeometry::segments() const
{
    return m_segments;
}

void SphereGeometry::setSegments(int newSegments)
{
    if (m_segments == newSegments)
        return;
    m_segments = newSegments;
    emit segmentsChanged();
    scheduleGeometryUpdate();
}

bool SphereGeometry::asynchronous() const
{
    return m_asynchronous;
}

void SphereGeometry::setAsynchronous(bool newAsynchronous)
{
    if (m_asynchronous == newAsynchronous)
        return;
    m_asynchronous = newAsynchronous;
    emit asynchronousChanged();
}

SphereGeometry::Status SphereGeometry::status() const
{
    return m_status;
}

void SphereGeometry::doUpdateGeometry()
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
    if (m_radius < 0 || m_rings < 1 || m_segments < 3) {
        clear();
        update();
        return;
    }

#if QT_CONFIG(concurrent)
    if (m_asynchronous) {
        m_geometryDataFuture = QtConcurrent::run(generateSphereGeometryAsync,
                                                 m_radius,
                                                 m_rings,
                                                 m_segments);
        m_geometryDataWatcher.setFuture(m_geometryDataFuture);
        m_status = Status::Loading;
        Q_EMIT statusChanged();
    } else {
#else
    {

#endif // QT_CONFIG(concurrent)
        updateGeometry(generateSphereGeometry(m_radius, m_rings, m_segments));
    }
}

void SphereGeometry::requestFinished()
{
#if QT_CONFIG(concurrent)
    const auto output = m_geometryDataFuture.takeResult();
    updateGeometry(output);
#endif
}

void SphereGeometry::scheduleGeometryUpdate()
{
    if (!m_geometryUpdateRequested) {
        QMetaObject::invokeMethod(this, "doUpdateGeometry", Qt::QueuedConnection);
        m_geometryUpdateRequested = true;
    }
}

void SphereGeometry::updateGeometry(const GeometryData &geometryData)
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

SphereGeometry::GeometryData SphereGeometry::generateSphereGeometry(float radius, int rings, int segments)
{
    GeometryData geometryData;

    // Pre-compute the size of the vertex and index data
    const int numVertices = (rings + 1) * (segments + 1);
    const int numIndices = rings * segments * 6;
    const int vertexStride = sizeof(float) * (3 + 2 + 3); // 3 for position, 2 for uv, 3 for normal
    const int indexStride = sizeof(uint16_t);

    // Resize the QByteArrays to fit all the vertex and index data
    geometryData.vertexData.resize(numVertices * vertexStride);
    geometryData.indexData.resize(numIndices * indexStride);


    // Bounds initialization using std::numeric_limits<float>::max() and lowest()
    QVector3D boundsMin(std::numeric_limits<float>::max(),
                        std::numeric_limits<float>::max(),
                        std::numeric_limits<float>::max());

    QVector3D boundsMax(std::numeric_limits<float>::lowest(),
                        std::numeric_limits<float>::lowest(),
                        std::numeric_limits<float>::lowest());

    // Temporary pointers for direct writing into the QByteArray
    float* vertexPtr = reinterpret_cast<float*>(geometryData.vertexData.data());
    uint16_t* indexPtr = reinterpret_cast<uint16_t*>(geometryData.indexData.data());

    // Loop through rings and segments to generate the vertex data
    for (int i = 0; i <= rings; ++i) {
        float phi = M_PI * i / rings;  // from 0 to PI
        float y = radius * std::cos(phi);
        float ringRadius = radius * std::sin(phi);

        for (int j = 0; j <= segments; ++j) {
            float theta = 2 * M_PI * j / segments;  // from 0 to 2PI
            float x = ringRadius * std::cos(theta);
            float z = ringRadius * std::sin(theta);

            // Position (vec3)
            *vertexPtr++ = x;
            *vertexPtr++ = y;
            *vertexPtr++ = z;

            // UV coordinates (vec2)
            *vertexPtr++ = 1.0f - static_cast<float>(j) / segments;
            *vertexPtr++ = 1.0f - static_cast<float>(i) / rings;

            // Normalized normal vector (vec3)
            QVector3D normal(x, y, z);
            normal.normalize();
            *vertexPtr++ = normal.x();
            *vertexPtr++ = normal.y();
            *vertexPtr++ = normal.z();

            // Update bounds
            boundsMin.setX(std::min(boundsMin.x(), x));
            boundsMin.setY(std::min(boundsMin.y(), y));
            boundsMin.setZ(std::min(boundsMin.z(), z));
            boundsMax.setX(std::max(boundsMax.x(), x));
            boundsMax.setY(std::max(boundsMax.y(), y));
            boundsMax.setZ(std::max(boundsMax.z(), z));
        }
    }

    // Loop through rings and segments to generate the index data
    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < segments; ++j) {
            uint16_t a = static_cast<uint16_t>(i * (segments + 1) + j);
            uint16_t b = static_cast<uint16_t>(a + segments + 1);
            uint16_t c = static_cast<uint16_t>(b + 1);
            uint16_t d = static_cast<uint16_t>(a + 1);

            // First triangle (a, d, b)
            *indexPtr++ = a;
            *indexPtr++ = d;
            *indexPtr++ = b;

            // Second triangle (b, d, c)
            *indexPtr++ = b;
            *indexPtr++ = d;
            *indexPtr++ = c;
        }
    }

    geometryData.boundsMin = boundsMin;
    geometryData.boundsMax = boundsMax;

    // Return the geometry data
    return geometryData;
}

#if QT_CONFIG(concurrent)
void SphereGeometry::generateSphereGeometryAsync(QPromise<SphereGeometry::GeometryData> &promise,
                                                 float radius,
                                                 int rings,
                                                 int segments)
{
    auto output = generateSphereGeometry(radius, rings, segments);
    promise.addResult(output);
}
#endif

QT_END_NAMESPACE
