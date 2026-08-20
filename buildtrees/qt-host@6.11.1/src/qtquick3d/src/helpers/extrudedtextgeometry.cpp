// Copyright (C) 2024 The Qt Company Ltd.
// Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "extrudedtextgeometry_p.h"
#include <QPainterPath>
#include <QtGui/private/qtriangulator_p.h>
#include <QVector3D>

#if QT_CONFIG(concurrent)
#include <QtConcurrentRun>
#endif

QT_BEGIN_NAMESPACE

namespace {

static float edgeSplitAngle = 90.f * 0.1f;

struct TriangulationData {
    struct Outline {
        int begin;
        int end;
    };

    std::vector<QVector3D> vertices;
    std::vector<ExtrudedTextGeometry::IndexType> indices;
    std::vector<Outline> outlines;
    std::vector<ExtrudedTextGeometry::IndexType> outlineIndices;
    bool inverted;
};

TriangulationData triangulate(const QString &text, const QFont &font, float scale)
{
    TriangulationData result;
    int beginOutline = 0;

    // Initialize path with text and extract polygons
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addText(0, 0, font, text);
    QList<QPolygonF> polygons = path.toSubpathPolygons(QTransform().scale(1., -1.));

    // maybe glyph has no geometry
    if (polygons.empty())
        return result;

    const size_t prevNumIndices = result.indices.size();

    // Reset path and add previously extracted polygons (which where spatially transformed)
    path = QPainterPath();
    path.setFillRule(Qt::WindingFill);
    for (QPolygonF &p : polygons)
        path.addPolygon(p);

    // Extract polylines out of the path, this allows us to retrieve indices for each glyph outline
    QPolylineSet polylines = qPolyline(path);
    std::vector<ExtrudedTextGeometry::IndexType> tmpIndices;
    tmpIndices.resize(size_t(polylines.indices.size()));
    memcpy(tmpIndices.data(), polylines.indices.data(), size_t(polylines.indices.size()) * sizeof(ExtrudedTextGeometry::IndexType));

    int lastIndex = 0;
    for (const ExtrudedTextGeometry::IndexType idx : tmpIndices) {
        if (idx == std::numeric_limits<ExtrudedTextGeometry::IndexType>::max()) {
            const int endOutline = lastIndex;
            result.outlines.push_back({beginOutline, endOutline});
            beginOutline = endOutline;
        } else {
            result.outlineIndices.push_back(idx);
            ++lastIndex;
        }
    }

    // Triangulate path
    QTransform transform;
    transform.scale(scale, scale);
    const QTriangleSet triangles = qTriangulate(path, transform);

    // Append new indices to result.indices buffer
    result.indices.resize(result.indices.size() + size_t(triangles.indices.size()));
    memcpy(&result.indices[prevNumIndices], triangles.indices.data(), size_t(triangles.indices.size()) * sizeof(ExtrudedTextGeometry::IndexType));
    for (size_t i = prevNumIndices, m = result.indices.size(); i < m; ++i)
        result.indices[i] += ExtrudedTextGeometry::IndexType(result.vertices.size());

    // Append new triangles to result.vertices
    result.vertices.reserve(size_t(triangles.vertices.size()) / 2);
    for (qsizetype i = 0, m = triangles.vertices.size(); i < m; i += 2)
        result.vertices.push_back(QVector3D(triangles.vertices[i] / font.pointSizeF(), triangles.vertices[i + 1] / font.pointSizeF(), 0.0f));

    return result;
}

inline QVector3D mix(const QVector3D &a, const QVector3D &b, float ratio)
{
    return a + (b - a) * ratio;
}

} // anonymous namespace


/*!
    \qmltype ExtrudedTextGeometry
    \inqmlmodule QtQuick3D.Helpers
    \inherits Geometry
    \since 6.9
    \brief Provides geometry for extruded text.

    ExtrudedTextGeometry provides geometry for extruded text. The text is extruded along the z-axis.
    The text and font can be set, and the depth of the extrusion can be controlled.
    The size of the generated geometry is controlled by the scale and depth properties. The topology
    of the geometry is defined by the font.pointSize.

    The origin of the mesh is the rear left end of the text's baseline.
*/

/*!
    \qmlproperty string ExtrudedTextGeometry::text

    This property holds the text that will be extruded.
*/

/*!
    \qmlproperty font ExtrudedTextGeometry::font

    This property holds the font that will be used to render the text.

    \note The mesh geometry is normalized by the font's pointSize, so a larger pointSize
    will result in smoother, rather than larger, text. pixelSize should not
    be used.
*/

/*!
    \qmlproperty real ExtrudedTextGeometry::depth

    This property holds the depth of the extrusion.
*/

/*!
    \qmlproperty real ExtrudedTextGeometry::scale

    This property holds a scalar value of how the geometry should be scaled.
    This property only affects the size of the text, not the depth of the extrusion.
*/

/*!
    \qmlproperty bool ExtrudedTextGeometry::asynchronous

    This property holds whether the geometry generation should be asynchronous.
*/

/*!
    \qmlproperty bool ExtrudedTextGeometry::status
    \readonly

    This property holds the status of the geometry generation when asynchronous is true.

    \value ExtrudedTextGeometry.Null The geometry generation has not started
    \value ExtrudedTextGeometry.Ready The geometry generation is complete.
    \value ExtrudedTextGeometry.Loading The geometry generation is in progress.
    \value ExtrudedTextGeometry.Error The geometry generation failed.
*/


ExtrudedTextGeometry::ExtrudedTextGeometry(QQuick3DObject *parent)
    : QQuick3DGeometry(parent)
{
#if QT_CONFIG(concurrent)
    connect(&m_geometryDataWatcher, &QFutureWatcher<GeometryData>::finished, this, &ExtrudedTextGeometry::requestFinished);
#endif
    scheduleGeometryUpdate();
}

ExtrudedTextGeometry::~ExtrudedTextGeometry()
{

}

QString ExtrudedTextGeometry::text() const
{
    return m_text;
}

void ExtrudedTextGeometry::setText(const QString &newText)
{
    if (m_text == newText)
        return;
    m_text = newText;
    emit textChanged();
    scheduleGeometryUpdate();
}

QFont ExtrudedTextGeometry::font() const
{
    return m_font;
}

void ExtrudedTextGeometry::setFont(const QFont &newFont)
{
    if (m_font == newFont)
        return;
    m_font = newFont;
    emit fontChanged();
    scheduleGeometryUpdate();
}

float ExtrudedTextGeometry::depth() const
{
    return m_depth;
}

void ExtrudedTextGeometry::setDepth(float newDepth)
{
    if (qFuzzyCompare(m_depth, newDepth))
        return;
    m_depth = newDepth;
    emit depthChanged();
    scheduleGeometryUpdate();
}

float ExtrudedTextGeometry::scale() const
{
    return m_scale;
}

void ExtrudedTextGeometry::setScale(float newScale)
{
    if (qFuzzyCompare(m_scale, newScale))
        return;
    m_scale = newScale;
    emit scaleChanged();
    scheduleGeometryUpdate();
}

bool ExtrudedTextGeometry::asynchronous() const
{
    return m_asynchronous;
}

void ExtrudedTextGeometry::setAsynchronous(bool newAsynchronous)
{
    if (m_asynchronous == newAsynchronous)
        return;
    m_asynchronous = newAsynchronous;
    emit asynchronousChanged();
}

ExtrudedTextGeometry::Status ExtrudedTextGeometry::status() const
{
    return m_status;
}

void ExtrudedTextGeometry::doUpdateGeometry()
{
    // reset the flag since we are processing the update
    m_geometryUpdateRequested = false;

#if QT_CONFIG(concurrent)
    if (m_geometryDataFuture.isRunning()) {
        m_pendingAsyncUpdate = true;
        return;
    }
#endif

    // If text is empty, clear the geometry
    // Note this happens after we check if we are already running an update
    // asynchronously.
    if (m_text.isEmpty()) {
        clear();
        update();
        return;
    }

#if QT_CONFIG(concurrent)

    if (m_asynchronous) {
        m_geometryDataFuture = QtConcurrent::run(generateExtrudedTextGeometryAsync,
                                                 m_text,
                                                 m_font,
                                                 m_depth,
                                                 m_scale);
        m_geometryDataWatcher.setFuture(m_geometryDataFuture);
        m_status = Status::Loading;
        Q_EMIT statusChanged();
    } else {
#else
    {

#endif // QT_CONFIG(concurrent)
        updateGeometry(generateExtrudedTextGeometry(m_text, m_font, m_depth, m_scale));
    }
}

void ExtrudedTextGeometry::requestFinished()
{
#if QT_CONFIG(concurrent)
    const auto output = m_geometryDataFuture.takeResult();
    updateGeometry(output);
#endif
}

void ExtrudedTextGeometry::scheduleGeometryUpdate()
{
    if (!m_geometryUpdateRequested) {
        QMetaObject::invokeMethod(this, "doUpdateGeometry", Qt::QueuedConnection);
        m_geometryUpdateRequested = true;
    }
}

void ExtrudedTextGeometry::updateGeometry(const GeometryData &geometryData)
{
    // clear();
    setStride(sizeof(float) * 6); // 3 for position, 3 for normal
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                 0,
                 QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::NormalSemantic,
                 3 * sizeof(float),
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

ExtrudedTextGeometry::GeometryData ExtrudedTextGeometry::generateExtrudedTextGeometry(const QString &text,
                                                                                      const QFont &font,
                                                                                      float depth,
                                                                                      float scale)
{
    GeometryData output;

    struct Vertex {
        QVector3D position;
        QVector3D normal;
    };

    std::vector<IndexType> indices;
    std::vector<Vertex> vertices;

    TriangulationData data = triangulate(text, font, scale);

    const IndexType numVertices = IndexType(data.vertices.size());
    const size_t numIndices = data.indices.size();

    vertices.reserve(data.vertices.size() * 2);
    for (QVector3D &v : data.vertices) // front face
        vertices.push_back({ v, QVector3D(0.0f, 0.0f, -1.0f) });
    for (QVector3D &v : data.vertices) // back face
        vertices.push_back({ QVector3D(v.x(), v.y(), depth), QVector3D(0.0f, 0.0f, 1.0f) });

    int verticesIndex = int(vertices.size());
    for (size_t i = 0; i < data.outlines.size(); ++i) {
        const int begin = data.outlines[i].begin;
        const int end = data.outlines[i].end;
        const int verticesIndexBegin = verticesIndex;

        if (begin == end)
            continue;

        QVector3D prevNormal = QVector3D::crossProduct(
                                       vertices[data.outlineIndices[end - 1] + numVertices].position - vertices[data.outlineIndices[end - 1]].position,
                                       vertices[data.outlineIndices[begin]].position - vertices[data.outlineIndices[end - 1]].position).normalized();

        for (int j = begin; j < end; ++j) {
            const bool isLastIndex = (j == end - 1);
            const IndexType cur = data.outlineIndices[j];
            const IndexType next = data.outlineIndices[((j - begin + 1) % (end - begin)) + begin]; // normalize, bring in range and adjust
            const QVector3D normal = QVector3D::crossProduct(vertices[cur + numVertices].position - vertices[cur].position, vertices[next].position - vertices[cur].position).normalized();

            // use smooth normals in case of a short angle
            const bool smooth = QVector3D::dotProduct(prevNormal, normal) > (90.0f - edgeSplitAngle) / 90.0f;
            const QVector3D resultNormal = smooth ? mix(prevNormal, normal, 0.5f) : normal;
            if (!smooth)             {
                vertices.push_back({vertices[cur].position,               prevNormal});
                vertices.push_back({vertices[cur + numVertices].position, prevNormal});
                verticesIndex += 2;
            }

            vertices.push_back({vertices[cur].position,               resultNormal});
            vertices.push_back({vertices[cur + numVertices].position, resultNormal});

            const int v0 = verticesIndex;
            const int v1 = verticesIndex + 1;
            const int v2 = isLastIndex ? verticesIndexBegin     : verticesIndex + 2;
            const int v3 = isLastIndex ? verticesIndexBegin + 1 : verticesIndex + 3;

            indices.push_back(v0);
            indices.push_back(v1);
            indices.push_back(v2);
            indices.push_back(v2);
            indices.push_back(v1);
            indices.push_back(v3);

            verticesIndex += 2;
            prevNormal = normal;
        }
    }

    // Indices for the front and back faces
    const int indicesOffset = int(indices.size());
    indices.resize(indices.size() + numIndices * 2);

    // copy values for back faces
    IndexType *indicesFaces = indices.data() + indicesOffset;
    memcpy(indicesFaces, data.indices.data(), numIndices * sizeof(IndexType));

    // insert values for front face and flip triangles
    for (size_t j = 0; j < numIndices; j += 3) {
        indicesFaces[numIndices + j    ] = indicesFaces[j    ] + numVertices;
        indicesFaces[numIndices + j + 1] = indicesFaces[j + 2] + numVertices;
        indicesFaces[numIndices + j + 2] = indicesFaces[j + 1] + numVertices;
    }

    for (const auto &vertex : vertices) {
        const auto &p = vertex.position;
        output.boundsMin = QVector3D(qMin(output.boundsMin.x(), p.x()), qMin(output.boundsMin.y(), p.y()), qMin(output.boundsMin.z(), p.z()));
        output.boundsMax = QVector3D(qMax(output.boundsMax.x(), p.x()), qMax(output.boundsMax.y(), p.y()), qMax(output.boundsMax.z(), p.z()));
    }


    output.vertexData.resize(vertices.size() * sizeof(Vertex));
    memcpy(output.vertexData.data(), vertices.data(), vertices.size() * sizeof(Vertex));


    output.indexData.resize(indices.size() * sizeof(IndexType));
    memcpy(output.indexData.data(), indices.data(), indices.size() * sizeof(IndexType));


    return output;
}
#if QT_CONFIG(concurrent)
void ExtrudedTextGeometry::generateExtrudedTextGeometryAsync(QPromise<GeometryData> &promise,
                                                             const QString &text,
                                                             const QFont &font,
                                                             float depth,
                                                             float scale)
{
    GeometryData output = generateExtrudedTextGeometry(text, font, depth, scale);
    promise.addResult(output);
}
#endif

QT_END_NAMESPACE
