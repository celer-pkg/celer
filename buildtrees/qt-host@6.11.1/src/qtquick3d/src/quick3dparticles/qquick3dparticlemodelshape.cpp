// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticlemodelshape_p.h"
#include "qquick3dparticlerandomizer_p.h"
#include "qquick3dparticlesystem_p.h"
#include "qquick3dparticleutils_p.h"
#include <QtQml/qqmlfile.h>
#include <QtQuick3D/private/qquick3dmodel_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>
#include <algorithm>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ParticleModelShape3D
    \inherits ParticleAbstractShape3D
    \inqmlmodule QtQuick3D.Particles3D
    \brief Offers particle shape from model for emitters and affectors.
    \since 6.2

    The ParticleModelShape3D element can be used to get particle shape from a 3D model.

    For example, to emit particles from outlines of a model shape:

    \qml
    Component {
        id: suzanneComponent
        Model {
            source: "meshes/suzanne.mesh"
            scale: Qt.vector3d(100, 100, 100)
        }
    }

    ParticleEmitter3D {
        shape: ParticleModelShape3D {
            model: suzanneComponent
            fill: false
        }
        ...
    }
    \endqml
*/

QQuick3DParticleModelShape::QQuick3DParticleModelShape(QObject *parent)
    : QQuick3DParticleAbstractShape(parent)
{

}

QQuick3DParticleModelShape::~QQuick3DParticleModelShape()
{
    delete m_model;
}

/*!
    \qmlproperty bool ParticleModelShape3D::fill

    This property defines if the shape should be filled or just use the shape outlines.

    The default value is \c true.
*/
bool QQuick3DParticleModelShape::fill() const
{
    return m_fill;
}

/*!
    \qmlproperty Component ParticleModelShape3D::delegate
    The delegate provides a template defining the model for the ParticleModelShape3D.
    For example, using the default sphere model with default material
    \qml
    Component {
        id: modelComponent
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(0.5, 0.5, 0.5)
            materials: DefaultMaterial { diffuseColor: "red" }
        }
    }
    ParticleModelShape3D {
        delegate: modelComponent
    }
    \endqml
*/
QQmlComponent *QQuick3DParticleModelShape::delegate() const
{
    return m_delegate;
}

void QQuick3DParticleModelShape::setFill(bool fill)
{
    if (m_fill == fill)
        return;

    m_fill = fill;
    Q_EMIT fillChanged();
}

QVector3D QQuick3DParticleModelShape::getPosition(int particleIndex)
{
    return randomPositionModel(particleIndex);
}

QVector3D QQuick3DParticleModelShape::getSurfaceNormal(int particleIndex)
{
    if (m_fill)
        return QVector3D();
    if (m_cachedIndex != particleIndex)
        getPosition(particleIndex);
    return m_cachedNormal;
}

void QQuick3DParticleModelShape::setDelegate(QQmlComponent *delegate)
{
    if (delegate == m_delegate)
        return;
    m_delegate = delegate;
    clearModelVertexPositions();
    createModel();
    Q_EMIT delegateChanged();
}

void QQuick3DParticleModelShape::createModel()
{
    delete m_model;
    m_model = nullptr;
    if (!m_delegate)
        return;
    auto *obj = m_delegate->create(m_delegate->creationContext());
    m_model = qobject_cast<QQuick3DModel *>(obj);
    if (!m_model)
        delete obj;
}

QVector3D QQuick3DParticleModelShape::randomPositionModel(int particleIndex)
{
    if (m_model) {
        calculateModelVertexPositions();

        const QVector<QVector3D> &positions = m_vertexPositions;
        if (positions.size() > 0) {
            auto rand = m_system->rand();

            // Calculate model triangle areas so that the random triangle selection can be weighted
            // by the area. This way particles are uniformly emitted from the whole model.
            if (m_modelTriangleAreas.size() == 0) {
                m_modelTriangleAreas.reserve(positions.size() / 3);
                for (int i = 0; i + 2 < positions.size(); i += 3) {
                    const QVector3D &v1 = positions[i];
                    const QVector3D &v2 = positions[i + 1];
                    const QVector3D &v3 = positions[i + 2];
                    const float area = QVector3D::crossProduct(v1 - v2, v1 - v3).length() * 0.5f;
                    m_modelTriangleAreasSum += area;
                    m_modelTriangleAreas.append(m_modelTriangleAreasSum);
                    m_modelTriangleCenter += v1 + v2 + v3;
                }
                m_modelTriangleCenter /= positions.size();
            }

            const float rndWeight = rand->get(particleIndex, QPRand::Shape1) * m_modelTriangleAreasSum;

            // Use binary search to find the weighted random index
            int index = std::lower_bound(m_modelTriangleAreas.begin(), m_modelTriangleAreas.end(), rndWeight) - m_modelTriangleAreas.begin();

            const QVector3D &v1 = positions[index * 3];
            const QVector3D &v2 = positions[index * 3 + 1];
            const QVector3D &v3 = positions[index * 3 + 2];
            const float a = rand->get(particleIndex, QPRand::Shape2);
            const float b = rand->get(particleIndex, QPRand::Shape3);
            const float aSqrt = qSqrt(a);

            // Calculate a random point from the selected triangle
            QVector3D pos = (1.0 - aSqrt) * v1 + (aSqrt * (1.0 - b)) * v2 + (b * aSqrt) * v3;

            if (m_fill) {
                // The model is filled by selecting a random point between a random surface point
                // and the center of the model. The random point selection is exponentially weighted
                // towards the surface so that particles aren't clustered in the center.
                const float uniform = rand->get(particleIndex, QPRand::Shape4);
                const float lambda = 5.0f;
                const float alpha = -qLn(1 - (1 - qExp(-lambda)) * uniform) / lambda;
                pos += (m_modelTriangleCenter - pos) * alpha;
            } else {
                m_cachedIndex = particleIndex;
                m_cachedNormal = QVector3D::crossProduct(v2 - v1, v3 - v2).normalized();
            }

            auto *parent = parentNode();
            if (parent) {
                QMatrix4x4 mat;
                mat.rotate(parent->rotation() * m_model->rotation());
                if (!m_fill)
                    m_cachedNormal = mat.mapVector(m_cachedNormal);
                return mat.mapVector(pos * parent->sceneScale() * m_model->scale());
            }
        }
    }
    return QVector3D(0, 0, 0);
}

void QQuick3DParticleModelShape::clearModelVertexPositions()
{
    m_vertexPositions.clear();
    m_modelTriangleAreas.clear();
    m_modelTriangleAreasSum = 0;
}

void QQuick3DParticleModelShape::calculateModelVertexPositions()
{
    if (m_vertexPositions.empty())
        m_vertexPositions = positionsFromModel(m_model, nullptr, qmlContext(this));
}

QT_END_NAMESPACE
