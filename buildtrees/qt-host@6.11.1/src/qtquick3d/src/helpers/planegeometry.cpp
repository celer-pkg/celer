// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "planegeometry_p.h"
#include <limits>

#if QT_CONFIG(concurrent)
#include <QtConcurrentRun>
#endif

QT_BEGIN_NAMESPACE

/*!
    \qmltype PlaneGeometry
    \inqmlmodule QtQuick3D.Helpers
    \inherits Geometry
    \since 6.9
    \brief Provides geometry for a plane.

    PlaneGeometry is a geometry type that represents a plane. The plane's size is
    defined by its height and width properties. The topology of the plane is defined by
    the meshResolution property. The orientation of the plane is defined by the plane property.
*/

/*!
    \qmlproperty real PlaneGeometry::width
    The width of the plane. The default value is 100.0.
*/

/*!
    \qmlproperty real PlaneGeometry::height
    The height of the plane. The default value is 100.0.
*/


/*!
    \qmlproperty size PlaneGeometry::meshResolution
    The resolution of the plane. The default value is QSize(2, 2).
*/

/*!
    \qmlproperty PlaneGeometry::Plane PlaneGeometry::plane
    The orientation of the plane. The default value is PlaneGeometry.XY.
    All geometry will be created along the selected plane, and the front
    face and normal will point towards the remaining positive axis, unless
    reversed is true.

    \value PlaneGeometry.XY The plane is oriented along the XY plane.
    \value PlaneGeometry.XZ The plane is oriented along the XZ plane.
    \value PlaneGeometry.ZY The plane is oriented along the ZY plane.
*/

/*!
    \qmlproperty bool PlaneGeometry::reversed
    This property holds whether the plane is flipped. This changes both the normal as
    well as the winding order of the plane.  The default value is false, which means that
    when a Plane is created with the XY orientation, the normal will point in the positive
    Z direction and the winding order will be counter-clockwise. When reversed is true,
    the normal will point in the negative Z direction and the winding order will be clockwise.
*/

/*!
    \qmlproperty bool PlaneGeometry::mirrored
    This property holds whether the UV coordinates of the plane are flipped vertically.
*/

/*!
    \qmlproperty bool PlaneGeometry::asynchronous

    This property holds whether the geometry generation should be asynchronous.
*/

/*!
    \qmlproperty bool PlaneGeometry::status
    \readonly

    This property holds the status of the geometry generation when asynchronous is true.

    \value PlaneGeometry.Null The geometry generation has not started
    \value PlaneGeometry.Ready The geometry generation is complete.
    \value PlaneGeometry.Loading The geometry generation is in progress.
    \value PlaneGeometry.Error The geometry generation failed.
*/

PlaneGeometry::PlaneGeometry(QQuick3DObject *parent)
    : QQuick3DGeometry(parent)
{
#if QT_CONFIG(concurrent)
    connect(&m_geometryDataWatcher, &QFutureWatcher<GeometryData>::finished, this, &PlaneGeometry::requestFinished);
#endif
    scheduleGeometryUpdate();
}

PlaneGeometry::~PlaneGeometry()
{

}

float PlaneGeometry::width() const
{
    return m_width;
}

void PlaneGeometry::setWidth(float newWidth)
{
    if (qFuzzyCompare(m_width, newWidth))
        return;
    m_width = newWidth;
    emit widthChanged();
    scheduleGeometryUpdate();
}

float PlaneGeometry::height() const
{
    return m_height;
}

void PlaneGeometry::setHeight(float newHeight)
{
    if (qFuzzyCompare(m_height, newHeight))
        return;
    m_height = newHeight;
    emit heightChanged();
    scheduleGeometryUpdate();
}

QSize PlaneGeometry::meshResolution() const
{
    return m_meshResolution;
}

void PlaneGeometry::setMeshResolution(const QSize &newMeshResolution)
{
    if (m_meshResolution == newMeshResolution)
        return;
    m_meshResolution = newMeshResolution;
    emit meshResolutionChanged();
    scheduleGeometryUpdate();
}

PlaneGeometry::Plane PlaneGeometry::plane() const
{
    return m_plane;
}

void PlaneGeometry::setPlane(Plane newPlane)
{
    if (m_plane == newPlane)
        return;
    m_plane = newPlane;
    emit planeChanged();
    scheduleGeometryUpdate();
}

bool PlaneGeometry::reversed() const
{
    return m_reversed;
}

void PlaneGeometry::setReversed(bool newReversed)
{
    if (m_reversed == newReversed)
        return;
    m_reversed = newReversed;
    emit reversedChanged();
    scheduleGeometryUpdate();
}

bool PlaneGeometry::mirrored() const
{
    return m_mirrored;
}

void PlaneGeometry::setMirrored(bool newMirrored)
{
    if (m_mirrored == newMirrored)
        return;
    m_mirrored = newMirrored;
    emit mirroredChanged();
    scheduleGeometryUpdate();
}

bool PlaneGeometry::asynchronous() const
{
    return m_asynchronous;
}

void PlaneGeometry::setAsynchronous(bool newAsynchronous)
{
    if (m_asynchronous == newAsynchronous)
        return;
    m_asynchronous = newAsynchronous;
    emit asynchronousChanged();
}

PlaneGeometry::Status PlaneGeometry::status() const
{
    return m_status;
}

void PlaneGeometry::doUpdateGeometry()
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
    if (m_width <= 0 || m_height <= 0 || m_meshResolution.width() <= 0 || m_meshResolution.height() <= 0) {
        clear();
        update();
        return;
    }

#if QT_CONFIG(concurrent)
    if (m_asynchronous) {
        m_geometryDataFuture = QtConcurrent::run(generatePlaneGeometryAsync,
                                                 m_width,
                                                 m_height,
                                                 m_meshResolution,
                                                 m_plane,
                                                 m_reversed,
                                                 m_mirrored);
        m_geometryDataWatcher.setFuture(m_geometryDataFuture);
        m_status = Status::Loading;
        Q_EMIT statusChanged();
    } else {
#else
    {

#endif // QT_CONFIG(concurrent)
        updateGeometry(generatePlaneGeometry(m_width, m_height, m_meshResolution, m_plane, m_reversed, m_mirrored));
    }
}

void PlaneGeometry::requestFinished()
{
#if QT_CONFIG(concurrent)
    const auto output = m_geometryDataFuture.takeResult();
    updateGeometry(output);
#endif
}

void PlaneGeometry::scheduleGeometryUpdate()
{
    if (!m_geometryUpdateRequested) {
        QMetaObject::invokeMethod(this, "doUpdateGeometry", Qt::QueuedConnection);
        m_geometryUpdateRequested = true;
    }
}

void PlaneGeometry::updateGeometry(const GeometryData &geometryData)
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

PlaneGeometry::GeometryData PlaneGeometry::generatePlaneGeometry(float width, float height, QSize meshResolution, Plane plane, bool reversed, bool mirrored)
{
    GeometryData geometryData;

    int quadsX = meshResolution.width();
    int quadsY = meshResolution.height();

    const int numVertices = (quadsX + 1) * (quadsY + 1);
    const int numIndices = quadsX * quadsY * 6; // Two triangles per quad

    const int vertexStride = sizeof(float) * (3 + 2 + 3); // vec3 (position), vec2 (uv), vec3 (normal)
    const int indexStride = sizeof(uint16_t); // 16-bit index

    geometryData.vertexData.resize(numVertices * vertexStride);
    geometryData.indexData.resize(numIndices * indexStride);

    QVector3D boundsMin(std::numeric_limits<float>::max(),
                        std::numeric_limits<float>::max(),
                        std::numeric_limits<float>::max());

    QVector3D boundsMax(std::numeric_limits<float>::lowest(),
                        std::numeric_limits<float>::lowest(),
                        std::numeric_limits<float>::lowest());

    float* vertexPtr = reinterpret_cast<float*>(geometryData.vertexData.data());
    uint16_t* indexPtr = reinterpret_cast<uint16_t*>(geometryData.indexData.data());

    QVector3D normal;
    switch (plane) {
    case Plane::XY:
        normal = QVector3D(0, 0, 1);
        break;
    case Plane::XZ:
        normal = QVector3D(0, 1, 0);
        break;
    case Plane::ZY:
        normal = QVector3D(1, 0, 0);
        break;
    }

    // Flip normal if the plane is reversed
    if (reversed)
        normal = -normal;

    for (int y = 0; y <= quadsY; ++y) {
        for (int x = 0; x <= quadsX; ++x) {
            // Normalized UV coordinates
            float u = static_cast<float>(x) / quadsX;
            float v = static_cast<float>(y) / quadsY;

            // Position in local space based on plane orientation
            float posX = width * (u - 0.5f);
            float posY = height * (v - 0.5f);

            if (mirrored)
                v = 1.0f - v;

            if (reversed)
                u = 1.0f - u;


            QVector3D position;
            switch (plane) {
            case Plane::XY:
                position = QVector3D(posX, posY, 0.0f);
                break;
            case Plane::XZ:
                position = QVector3D(posX, 0.0f, -posY);
                break;
            case Plane::ZY:
                position = QVector3D(0.0f, posY, -posX);
                break;
            }

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

            // Update bounds
            boundsMin.setX(std::min(boundsMin.x(), position.x()));
            boundsMin.setY(std::min(boundsMin.y(), position.y()));
            boundsMin.setZ(std::min(boundsMin.z(), position.z()));

            boundsMax.setX(std::max(boundsMax.x(), position.x()));
            boundsMax.setY(std::max(boundsMax.y(), position.y()));
            boundsMax.setZ(std::max(boundsMax.z(), position.z()));
        }
    }

    // Generate indices
    for (int y = 0; y < quadsY; ++y) {
        for (int x = 0; x < quadsX; ++x) {
            uint16_t a = static_cast<uint16_t>(y * (quadsX + 1) + x);
            uint16_t b = static_cast<uint16_t>(a + quadsX + 1);
            uint16_t c = static_cast<uint16_t>(b + 1);
            uint16_t d = static_cast<uint16_t>(a + 1);

            if (reversed) {
                // Reverse the triangle winding order
                *indexPtr++ = a;
                *indexPtr++ = b;
                *indexPtr++ = d;

                *indexPtr++ = b;
                *indexPtr++ = c;
                *indexPtr++ = d;
            } else {
                // Normal winding order
                *indexPtr++ = a;
                *indexPtr++ = d;
                *indexPtr++ = b;

                *indexPtr++ = b;
                *indexPtr++ = d;
                *indexPtr++ = c;
            }
        }
    }

    // Return the geometry data
    geometryData.boundsMax = boundsMax;
    geometryData.boundsMin = boundsMin;

    return geometryData;
}

#if QT_CONFIG(concurrent)
void PlaneGeometry::generatePlaneGeometryAsync(QPromise<PlaneGeometry::GeometryData> &promise,
                                               float width,
                                               float height,
                                               QSize meshResolution,
                                               Plane plane,
                                               bool reversed,
                                               bool mirrored)
{
    auto output = generatePlaneGeometry(width, height, meshResolution, plane, reversed, mirrored);
    promise.addResult(output);
}
#endif

QT_END_NAMESPACE
