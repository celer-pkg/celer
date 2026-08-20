// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticlesceneshape_p.h"
#include "qquick3dparticlemodelshape_p.h"
#include "qquick3dparticlerandomizer_p.h"
#include "qquick3dparticlesystem_p.h"
#include "qquick3dparticleutils_p.h"
#include <QtQml/qqmlfile.h>
#include <QtQuick3D/QQuick3DGeometry>
#include <QtQuick3D/private/qquick3dmodel_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>
#include <algorithm>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ParticleSceneShape3D
    \inherits ParticleAbstractShape3D
    \inqmlmodule QtQuick3D.Particles3D
    \brief Offers particle shape from a scene for emitters and affectors.
    \since 6.11

    The ParticleSceneShape3D element can be used to get particle shape from a 3D scene.
    The shape envelopes the scene into single shape based on the models in a scene.

    For example,

    \qml
    ParticleEmitter3D {
        id: emitter
        particle: particle

        ...

        shape: ParticleSceneShape3D {
            id: sceneShape
            scene: sceneRoot
            sceneCenter: node.scenePosition
            sceneExtents: Qt.vector3d(1000, 500, 1000)
        }

        ...
    }
    \endqml
*/

QQuick3DParticleSceneShape::QQuick3DParticleSceneShape(QObject *parent)
    : QQuick3DParticleAbstractShape(parent)
{

}

QQuick3DParticleSceneShape::~QQuick3DParticleSceneShape()
{
    delete m_geometry;
    clearModelVertexPositions();
}

/*!
    \qmlproperty Node ParticleSceneShape3D::scene

    This property holds the root node of the scene. The scene shape is dynamically
    calculated based on the child models of the scene. If the scene changes,
    the shape is recalculated. If a model is not visible in the scene, it won't be
    included in the shape. Only the node visibility is checked by the implementation.
    If the material is fully transparent or the node gets filtered out, the shape
    will still contain those nodes unless they are manyally excluded.
*/
QQuick3DNode *QQuick3DParticleSceneShape::scene() const
{
    return m_scene;
}

void QQuick3DParticleSceneShape::setScene(QQuick3DNode *scene)
{
    if (m_scene == scene)
        return;
    m_scene = scene;
    clearModelVertexPositions();
    if (scene)
        createShapeData();
}

/*!
    \qmlproperty Node ParticleSceneShape3D::sceneCenter

    This property holds the center point where the shape is calculated.

    The default value is \c Qt.vector3(0, 0, 0).
*/
QVector3D QQuick3DParticleSceneShape::sceneCenter() const
{
    return m_sceneCenter;
}

void QQuick3DParticleSceneShape::setSceneCenter(const QVector3D &center)
{
    if (m_sceneCenter == center)
        return;

    m_sceneCenter = center;
    Q_EMIT sceneCenterChanged();
    markDirty();
}

/*!
    \qmlproperty Node ParticleSceneShape3D::sceneExtents

    This property holds the extents for which the shape is calculated.
    The extents are added to the scene center and any polygon outside
    this volume are left out of the shape. If this value is empty, all
    polygons are included.

    The default value is \c Qt.vector3(0, 0, 0).
*/
QVector3D QQuick3DParticleSceneShape::sceneExtents() const
{
    return m_sceneExtents;
}

void QQuick3DParticleSceneShape::setSceneExtents(const QVector3D &extents)
{
    if (m_sceneExtents == extents)
        return;

    m_sceneExtents = extents;
    Q_EMIT sceneExtentsChanged();
    markDirty();
}

/*!
    \qmlproperty real ParticleSceneShape3D::shapeResolution

    This property holds the resolution of the shape. The higher the value
    the more accurately the shape resembles individual models.

    The minimum value is 1.0, the maximum value is 100.0;
    \default 10.0;
*/
float QQuick3DParticleSceneShape::shapeResolution() const
{
    return m_shapeResolution;
}

void QQuick3DParticleSceneShape::setShapeResolution(float resolution)
{
    if (qFuzzyCompare(resolution, m_shapeResolution))
        return;

    m_shapeResolution = qBound(1.0f, resolution, 100.0f);
    Q_EMIT sceneExtentsChanged();
    markDirty();
}

/*!
    \qmlproperty Node ParticleSceneShape3D::geometry

    This property holds the geometry of the generated shape. It can be used to render
    the shape to visualize it.
*/
QQuick3DGeometry *QQuick3DParticleSceneShape::geometry() const
{
    return m_geometry;
}

/*!
    \qmlproperty list<Node> ParticleSceneShape3D::excludedNodes

    This property holds the list of nodes that are excluded when building
    the shape. Model is excluded from the shape if it is one of the nodes
    or their child node.
*/
QList<QQuick3DNode *> QQuick3DParticleSceneShape::excludedNodes() const
{
    return m_excludeList;
}

void QQuick3DParticleSceneShape::setExcludedNodes(const QList<QQuick3DNode *> &nodes)
{
    if (nodes == m_excludeList)
        return;
    m_excludeList = nodes;
    Q_EMIT excludedNodesChanged();
    markDirty(false, true);
}

QVector3D QQuick3DParticleSceneShape::getPosition(int particleIndex)
{
    return randomPositionModel(particleIndex);
}

QVector3D QQuick3DParticleSceneShape::getSurfaceNormal(int particleIndex)
{
    if (m_cachedIndex != particleIndex)
        getPosition(particleIndex);
    return m_cachedNormal;
}

void QQuick3DParticleSceneShape::markDirty(bool dirty, bool excludeDirty)
{
    for (auto &model : m_data) {
        model.dirty |= dirty;
        model.excludedDirty |= excludeDirty;
    }
}

/*
    SceneNapkinMesh is a grid mesh that is placed on top of the scene
    following the scene geometry. If neighbouring cells have large
    enough height difference, they are not connected and the shape
    has a hole in that position.
 */
struct QQuick3DParticleSceneShape::SceneNapkinMesh
{
    struct Triangle
    {
        QVector3D p0, p1, p2;
    };

    struct Cell
    {
        float x, z;
        float w, d;
        float y;
        int subcellCount = 0;
        int ccx, ccz;
        Cell *subcells = nullptr;
        static constexpr float c_maximumYDiff = 40.0f;

        ~Cell() {
            delete [] subcells;
        }
        QList<Triangle> triangles;

        void init(float x, float z, float w, float d, int ccx, int ccz, float y)
        {
            this->ccx = ccx;
            this->ccz = ccz;
            this->x = x;
            this->z = z;
            this->w = w;
            this->d = d;
            this->y = y;
            subcellCount = ccz * ccx;
            subcells = new Cell[subcellCount];
            initCells(subcells, x, z, w, d, ccx, ccz, y);
        }

        QVector3D center() const
        {
            return QVector3D(x + 0.5f * w, y, z + 0.5f * d);
        }

        void initCells(Cell *cells, float x, float z, float w, float d, int ccx, int ccz, float y)
        {
            float wccx = w / float(ccx - 1);
            float dccz = d / float(ccz - 1);
            for (int i = 0; i < ccx; i++) {
                for (int j = 0; j < ccz; j++) {
                    int idx = i * ccz + j;
                    cells[idx].x = x + i * wccx;
                    cells[idx].z = z + j * dccz;
                    cells[idx].w = wccx;
                    cells[idx].d = dccz;
                    cells[idx].subcellCount = 0;
                    cells[idx].subcells = nullptr;
                    cells[idx].y = y;
                    cells[idx].ccx = 0;
                    cells[idx].ccz = 0;
                }
            }
        }

        void update(const QVector3D &v1, const QVector3D &v2, const QVector3D &v3, bool append = true)
        {
            if (subcellCount == 0) {
                // Extents was empty so simply collect the triangles
                if (append)
                    triangles.append({v1, v2, v3});
                return;
            }
            // First get the rectangle this triangle forms ...
            float wccx = w / float(ccx - 1);
            float dccz = d / float(ccz - 1);
            int cx1 = (v1.x() - x) / wccx;
            int cz1 = (v1.z() - z) / dccz;
            int cx2 = (v2.x() - x) / wccx;
            int cz2 = (v2.z() - z) / dccz;
            int cx3 = (v3.x() - x) / wccx;
            int cz3 = (v3.z() - z) / dccz;
            cx1 = qBound(0, cx1, ccx - 1);
            cx2 = qBound(0, cx2, ccx - 1);
            cx3 = qBound(0, cx3, ccx - 1);
            cz1 = qBound(0, cz1, ccz - 1);
            cz2 = qBound(0, cz2, ccz - 1);
            cz3 = qBound(0, cz3, ccz - 1);
            int minx = qMin(qMin(cx1, cx2), cx3);
            int maxx = qMax(qMax(cx1, cx2), cx3);
            int minz = qMin(qMin(cz1, cz2), cz3);
            int maxz = qMax(qMax(cz1, cz2), cz3);

            // ... then test all cells inside this rectangle if they are inside the triangle.
            // If they are, adjust the height of the cell to the height of the triangle.
            for (int i = minx; i <= maxx; i++) {
                for (int j = minz; j <= maxz; j++) {
                    int idx = i * ccz + j;
                    QVector3D point = subcells[idx].center();
                    point.setY(this->y);
                    float py;
                    bool isIn = yOfTriangle(point, v1, v2, v3, py);
                    if (isIn) {
                        subcells[idx].y = qMax(subcells[idx].y, py);
                        if (append)
                            subcells[idx].triangles.append({v1, v2, v3});
                    } else if (minx == maxx && minz == maxz) { // Triangle is smaller than a cell so update anyway
                        subcells[idx].y = qMax(subcells[idx].y, qMax(v1.y(),qMax(v2.y(), v3.y())));
                        if (append)
                            subcells[idx].triangles.append({v1, v2, v3});
                    }
                }
            }
        }

        void updateSubcells()
        {
            for (const auto &triangle : std::as_const(triangles))
                update(triangle.p0, triangle.p1, triangle.p2, false);
        }

        QList<QVector3D> positions(float miny, float icp100)
        {
            QList<QVector3D> ret;
            Q_ASSERT(subcells);
            int pointCount = (ccx - 1) * (ccz - 1) * 6;
            ret.reserve(pointCount);

            // Form the mesh grid from the cells
            for (int i = 0; i < ccx - 1; i++) {
                for (int j = 0; j < ccz - 1; j++) {
                    int ca = i * ccz + j;
                    int cb = (i + 1) * ccz + j;
                    auto &sc = subcells[ca];
                    if (sc.subcells) {
                        ret.append(subcells[ca].positions(miny, icp100));
                    } else {
                        QVector3D a, b, c, d;
                        a = subcells[ca].center();
                        b = subcells[ca + 1].center();
                        c = subcells[cb].center();
                        d = subcells[cb + 1].center();
                        float ydiff1 = qMax(qMax(qAbs(a.y() - b.y()), qAbs(a.y() - c.y())), qAbs(b.y() - c.y()));
                        float ydiff2 = qMax(qMax(qAbs(d.y() - b.y()), qAbs(d.y() - c.y())), qAbs(b.y() - c.y()));
                        // Only add grid cell if the y is greated than the ground
                        if (a.y() > miny && b.y() > miny && c.y() > miny && d.y() > miny) {
                            // Do not add triangle if the height difference to neighbour is too big.
                            if (ydiff1 < c_maximumYDiff) {
                                ret.append(a);
                                ret.append(b);
                                ret.append(c);
                            }
                            if (ydiff2 < c_maximumYDiff) {
                                ret.append(c);
                                ret.append(b);
                                ret.append(d);
                            }
                        }
                    }
                }
            }
            return ret;
        }
    };

    Cell rootCell;
    float width = 0;
    float depth = 0;
    float x = 0;
    float z = 0;
    float icp100;
    QVector3D ext;
    QVector3D cent;
    QVector3D boundsMin, boundsMax;

    SceneNapkinMesh() {}
    ~SceneNapkinMesh() {}

    void init(const QVector3D &extents, const QVector3D &center, float initialCellsPer100)
    {
        ext = extents;
        cent = center;
        icp100 = initialCellsPer100;
        x = center.x()-extents.x();
        z = center.z()-extents.z();
        if (extents.isNull()) {
            // If the extents is empty, we must collect all the triangles
            // and calculate the bounds while collecting them
            constexpr auto maxVal = std::numeric_limits<float>().max();
            constexpr auto minVal = std::numeric_limits<float>().min();
            boundsMin = QVector3D(maxVal, maxVal, maxVal);
            boundsMax = QVector3D(minVal, minVal, minVal);
            return;
        }
        width = extents.x() * 2.0f;
        depth = extents.z() * 2.0f;
        float dwp = 100.0f / icp100;

        int ccx = width / dwp + 1;
        int ccz = depth / dwp + 1;

        rootCell.init(x, z, width, depth, ccx, ccz, center.y()-extents.y());
    }

    // Calculate y from top-down inside triangle [p0, p1, p2] based on point p, which
    // must be inside the triangle.
    static bool yOfTriangle(QVector3D p, QVector3D p0, QVector3D p1, QVector3D p2, float &y)
    {
        const auto scalarTriple = [](QVector3D a, QVector3D b, QVector3D c) {
            return QVector3D::dotProduct(QVector3D::crossProduct(a, b), c);
        };
        const auto sameSign = [](float a, float b) {
            return (a < 0.0f && b < 0.0f) || (a > 0.0f && b > 0.0f);
        };

        // Vectors from point to triangle vertices
        QVector3D pp0 = p0 - p;
        QVector3D pp1 = p1 - p;
        QVector3D pp2 = p2 - p;

        // Compute the barycentric coordinates (u, v, w) using parallelepiped volumes.
        QVector3D m = QVector3D::crossProduct(QVector3D(0, 1, 0), pp2);
        float u = QVector3D::dotProduct(pp1, m);
        float v = -QVector3D::dotProduct(pp0, m);
        if (!qFuzzyIsNull(u) && !qFuzzyIsNull(v) && !sameSign(u, v))
            return false;
        float w = scalarTriple(QVector3D(0, 1, 0), pp1, pp0);
        if (!qFuzzyIsNull(w) && !sameSign(u, w))
            return false;
        float den = 1.0f / (u + v + w);
        u *= den;
        v *= den;
        w *= den;
        y = p0.y() * u + p1.y() * v + p2.y() * w;
        return true;
    }
    void update(const QVector3D &v1, const QVector3D &v2, const QVector3D &v3)
    {
        if (ext.isNull()) {
            // Update bounds if extents is empty
            boundsMax.setX(qMax(boundsMax.x(), qMax(v1.x(), qMax(v2.x(), v3.x()))));
            boundsMax.setY(qMax(boundsMax.y(), qMax(v1.y(), qMax(v2.y(), v3.y()))));
            boundsMax.setZ(qMax(boundsMax.z(), qMax(v1.z(), qMax(v2.z(), v3.z()))));

            boundsMin.setX(qMin(boundsMin.x(), qMin(v1.x(), qMin(v2.x(), v3.x()))));
            boundsMin.setY(qMin(boundsMin.y(), qMin(v1.y(), qMin(v2.y(), v3.y()))));
            boundsMin.setZ(qMin(boundsMin.z(), qMin(v1.z(), qMin(v2.z(), v3.z()))));
        }
        rootCell.update(v1, v2, v3);
    }
    QList<QVector3D> positions()
    {
        float miny = cent.y() - ext.y();
        if (ext.isNull()) {
            // If extends was empty, rootCell simply collected all triangles
            // Now it needs to update subcells
            if (rootCell.triangles.isEmpty())
                return {};
            QVector3D center = (boundsMin + boundsMax) * 0.5;
            QVector3D extents = (boundsMax - boundsMin) * 0.5;
            init(extents, center, icp100);
            rootCell.updateSubcells();
            miny = center.y() - extents.y();
        }
        return rootCell.positions(miny, icp100);
    }
};

void QQuick3DParticleSceneShape::updateGeometry()
{
    // Only update geometry if the geometry is used
    static const QMetaMethod geometryChangedSignal = QMetaMethod::fromSignal(&QQuick3DParticleSceneShape::geometryChanged);
    if (!isSignalConnected(geometryChangedSignal))
        return;

    if (m_geometry)
        delete m_geometry;
    m_geometry = new QQuick3DGeometry();
    QVector3D boxMin, boxMax;
    if (m_sceneExtents.isNull()) {
        boxMin = m_sceneMesh->boundsMin;
        boxMax = m_sceneMesh->boundsMax;
    } else {
        boxMin = m_sceneCenter - m_sceneExtents;
        boxMax = m_sceneCenter + m_sceneExtents;
    }
    m_geometry->setBounds(boxMin, boxMax);
    QByteArray data((char *)m_sceneData.vertexPositions.data(), m_sceneData.vertexPositions.size() * sizeof(QVector3D));
    m_geometry->setVertexData(data);
    m_geometry->setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    m_geometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0, QQuick3DGeometry::Attribute::F32Type);
    m_geometry->setStride(sizeof(float) * 3);
    emit geometryChanged();
}

void QQuick3DParticleSceneShape::createShapeData()
{
    if (!m_scene)
        return;

    m_root = m_scene;

    if (m_root) {
        auto models = m_root->findChildren<QQuick3DModel*>();
        auto rootModel = qobject_cast<QQuick3DModel*>(m_root);
        if (rootModel)
            models.append(rootModel);
        int index = 0;
        for (const auto &model : std::as_const(models)) {
            ShapeData data;
            if ((model->geometry() != nullptr && model->geometry() == m_geometry))
                continue;
            data.model = model;
            data.sceneTransformConnection = QObject::connect(data.model, &QQuick3DNode::sceneTransformChanged, this, [&,index](){
                m_data[index].dirty = true;
            });
            data.visibilityConnection = QObject::connect(data.model, &QQuick3DNode::visibleChanged, this, [&,index](){
                m_data[index].dirty = true;
            });
            m_data.append(data);
            index++;
        }
    }
}

QVector3D QQuick3DParticleSceneShape::randomPositionModel(int particleIndex)
{
    if (m_root) {
        calculateModelVertexPositions();

        float totalArea = 0.0;
        if (m_sceneData.vertexPositions.isEmpty())
            return {};
        if (m_sceneData.modelTriangleAreas.size() == 0) {
            const auto & positions = m_sceneData.vertexPositions;
            m_sceneData.modelTriangleAreas.reserve(positions.size() / 3);
            m_sceneData.modelTriangleAreasSum = 0.0f;
            m_sceneData.modelTriangleCenter = {};
            for (int i = 0; i + 2 < positions.size(); i += 3) {
                const QVector3D &v1 = positions[i];
                const QVector3D &v2 = positions[i + 1];
                const QVector3D &v3 = positions[i + 2];
                const float area = QVector3D::crossProduct(v1 - v2, v1 - v3).length() * 0.5f;
                m_sceneData.modelTriangleAreasSum += area;
                m_sceneData.modelTriangleAreas.append(m_sceneData.modelTriangleAreasSum);
                m_sceneData.modelTriangleCenter += v1 + v2 + v3;
            }
            m_sceneData.modelTriangleCenter /= positions.size();
        }
        totalArea += m_sceneData.modelTriangleAreasSum;

        auto rand = m_system->rand();
        const float rndWeight = rand->get(particleIndex, QPRand::Shape1) * totalArea;
        // Use binary search to find the weighted random index
        int index = std::lower_bound(m_sceneData.modelTriangleAreas.begin(), m_sceneData.modelTriangleAreas.end(), rndWeight) - m_sceneData.modelTriangleAreas.begin();

        const QVector<QVector3D> &positions = m_sceneData.vertexPositions;

        const QVector3D &v1 = positions[index * 3];
        const QVector3D &v2 = positions[index * 3 + 1];
        const QVector3D &v3 = positions[index * 3 + 2];
        const float a = rand->get(particleIndex, QPRand::Shape2);
        const float b = rand->get(particleIndex, QPRand::Shape3);
        const float aSqrt = qSqrt(a);

        // Calculate a random point from the selected triangle
        QVector3D pos = (1.0 - aSqrt) * v1 + (aSqrt * (1.0 - b)) * v2 + (b * aSqrt) * v3;
        m_cachedIndex = particleIndex;
        m_cachedNormal = QVector3D::crossProduct(v2 - v1, v3 - v2).normalized();

        auto *parent = parentNode();
        if (parent) {
            QMatrix4x4 mat;
            mat.rotate(parent->rotation());
            m_cachedNormal = mat.mapVector(m_cachedNormal);
            return mat.mapVector(pos * parent->sceneScale());
        }

    }
    return QVector3D(0, 0, 0);
}

void QQuick3DParticleSceneShape::clearModelVertexPositions()
{
    for (const auto &model : std::as_const(m_data)) {
        QObject::disconnect(model.sceneTransformConnection);
        QObject::disconnect(model.visibilityConnection);
    }
    m_data.clear();
}

QList<QVector3D> QQuick3DParticleSceneShape::excludeTriangles(const QList<QVector3D> &list)
{
    QList<QVector3D> ret;
    QVector3D boxMin, boxMax;
    boxMin = m_sceneCenter - m_sceneExtents;
    boxMax = m_sceneCenter + m_sceneExtents;
    for (int i = 0; i < list.size(); i+=3) {
        QVector3D v0 = list[i];
        QVector3D v1 = list[i + 1];
        QVector3D v2 = list[i + 2];
        QVector3D normal = QVector3D::crossProduct(v1 - v0, v2 - v1).normalized();
        // The normal must point upwards
        if (normal.y() < 0.0f)
            continue;
        if (!m_sceneExtents.isNull()) {
            // Check if all v0-v2 are outside scene extents and don't add them if they are
            if (!(qAbs(v0.x() - m_sceneCenter.x()) < m_sceneExtents.x() &&
                  qAbs(v0.y() - m_sceneCenter.y()) < m_sceneExtents.y() &&
                  qAbs(v0.z() - m_sceneCenter.z()) < m_sceneExtents.z() &&
                  qAbs(v1.x() - m_sceneCenter.x()) < m_sceneExtents.x() &&
                  qAbs(v1.y() - m_sceneCenter.y()) < m_sceneExtents.y() &&
                  qAbs(v1.z() - m_sceneCenter.z()) < m_sceneExtents.z() &&
                  qAbs(v2.x() - m_sceneCenter.x()) < m_sceneExtents.x() &&
                  qAbs(v2.y() - m_sceneCenter.y()) < m_sceneExtents.y() &&
                  qAbs(v2.z() - m_sceneCenter.z()) < m_sceneExtents.z())) {
                // If so make sure they are not crossing the extents
                if (v0.x() < boxMin.x() && v1.x() < boxMin.x() && v2.x() < boxMin.x())
                    continue;
                if (v0.x() > boxMax.x() && v1.x() > boxMax.x() && v2.x() > boxMax.x())
                    continue;
                if (v0.y() < boxMin.y() && v1.y() < boxMin.y() && v2.y() < boxMin.y())
                    continue;
                if (v0.y() > boxMax.y() && v1.y() > boxMax.y() && v2.y() > boxMax.y())
                    continue;
                if (v0.z() < boxMin.z() && v1.z() < boxMin.z() && v2.z() < boxMin.z())
                    continue;
                if (v0.z() > boxMax.z() && v1.z() > boxMax.z() && v2.z() > boxMax.z())
                    continue;
            }
        }
        ret.append(v0);
        ret.append(v1);
        ret.append(v2);
    }
    return ret;
}

bool QQuick3DParticleSceneShape::isVisibleRecursive(QQuick3DNode *node)
{
    while (node) {
        if (!node->visible())
            return false;
        node = node->parentNode();
    }
    return true;
}

bool QQuick3DParticleSceneShape::isExcluded(QQuick3DModel *model)
{
    // Check if model parent is in the excluded list
    for (const QQuick3DNode *node : std::as_const(m_excludeList)) {
        QQuick3DNode *m = model;
        while (m) {
            if (m == node)
                return true;
            m = m->parentNode();
        }
    }
    return false;
}

void QQuick3DParticleSceneShape::calculateModelVertexPositions()
{
    bool update = false;
    for (const auto &modelData : std::as_const(m_data)) {
        update |= modelData.dirty || modelData.excludedDirty;
        if (update)
            break;
    }
    if (update) {
        if (m_sceneMesh) {
            delete m_sceneMesh;
            m_sceneData.clear();
        }
        m_sceneMesh = new QQuick3DParticleSceneShape::SceneNapkinMesh();
        m_sceneMesh->init(m_sceneExtents, m_sceneCenter, m_shapeResolution / 10.0f);
    } else {
        return;
    }

    for (auto &modelData : m_data) {
        if (modelData.excludedDirty) {
            modelData.excludedDirty = false;
            modelData.excluded = isExcluded(modelData.model);
        }
        modelData.visible = isVisibleRecursive(modelData.model);
        if (!modelData.visible || modelData.excluded) {
            modelData.dirty = false;
            continue;
        }
        if (modelData.dirty || modelData.vertexPositions.empty()) {
            const QMatrix4x4 mat = modelData.model->sceneTransform();
            modelData.vertexPositions = excludeTriangles(positionsFromModel(modelData.model, &mat, qmlContext(this)));
            modelData.hasVertices = !modelData.vertexPositions.isEmpty();
            modelData.modelTriangleAreas.clear();
        }
        modelData.dirty = false;
        if (modelData.hasVertices && modelData.visible) {
            int count = modelData.vertexPositions.size();
            for (int k = 0; k < count; k+=3)
                m_sceneMesh->update(modelData.vertexPositions[k], modelData.vertexPositions[k+1], modelData.vertexPositions[k+2]);
        }
    }
    if (update) {
        m_sceneData.vertexPositions = m_sceneMesh->positions();
        updateGeometry();
    }
}

QT_END_NAMESPACE
