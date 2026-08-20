// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "infinitegrid_p.h"
#include <QtQuick3D/private/qquick3dviewport_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype InfiniteGrid
    \inqmlmodule QtQuick3D.Helpers
    \since 6.5
    \brief Shows an infinite grid.

    This helper implements an infinite grid in the horizontal plane.
    The grid fades out as the grid lines converge or at the far clip distance,
    whichever comes first.

    The grid needs to be a child of the \l{SceneEnvironment}.

    \qml
    View3D {
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.SkyBox
            lightProbe: Texture {
                textureData: ProceduralSkyTextureData{}
            }
            InfiniteGrid {
                gridInterval: 100
            }
        }
        //...
     }
    \endqml
*/

/*!
    \qmlproperty real InfiniteGrid::gridInterval

    This property defines the distance between grid lines. The default value is \c 1.0.
*/

/*!
    \qmlproperty bool InfiniteGrid::visible

    This property determines whether the grid is shown. The default value is \c true.
*/

/*!
    \qmlproperty bool InfiniteGrid::gridAxes

    This property determines whether the X and Y axes are marked. If \c true,
    the X-axis will be red and the Y-axis green. The default value is \c true.
*/

QQuick3DInfiniteGrid::QQuick3DInfiniteGrid()
{
}

QQuick3DInfiniteGrid::~QQuick3DInfiniteGrid()
{
}

bool QQuick3DInfiniteGrid::visible() const
{
    return m_visible;
}

void QQuick3DInfiniteGrid::setVisible(bool newVisible)
{
    if (m_visible == newVisible)
        return;
    m_visible = newVisible;
    emit visibleChanged();
    if (m_sceneEnv)
        m_sceneEnv->setGridEnabled(m_visible);
}

float QQuick3DInfiniteGrid::gridInterval() const
{
    return m_gridInterval;
}

void QQuick3DInfiniteGrid::setGridInterval(float newGridInterval)
{
    if (qFuzzyCompare(m_gridInterval, newGridInterval))
        return;
    m_gridInterval = newGridInterval;
    emit gridIntervalChanged();
    if (m_sceneEnv && !qFuzzyIsNull(m_gridInterval))
        m_sceneEnv->setGridScale(0.1 / m_gridInterval);
}

void QQuick3DInfiniteGrid::componentComplete()
{
    m_componentComplete = true;
    auto *p = parent();
    QQuick3DSceneEnvironment *sceneEnv = nullptr;
    while (p && !sceneEnv) {
        sceneEnv = qobject_cast<QQuick3DSceneEnvironment *>(p);
        p = p->parent();
    }
    if (sceneEnv) {
        m_sceneEnv = sceneEnv;
        Q_ASSERT(m_sceneEnv);
        m_sceneEnv->setGridEnabled(m_visible);
        if (!qFuzzyIsNull(m_gridInterval))
            m_sceneEnv->setGridScale(0.1 / m_gridInterval);
        updateGridFlags();
    } else {
        qWarning("InfiniteGrid needs to be a child of SceneEnvironment.");
    }
}

void QQuick3DInfiniteGrid::classBegin()
{
}

bool QQuick3DInfiniteGrid::gridAxes() const
{
    return m_gridAxes;
}

void QQuick3DInfiniteGrid::setGridAxes(bool newGridAxes)
{
    if (m_gridAxes == newGridAxes)
        return;
    m_gridAxes = newGridAxes;
    emit gridAxesChanged();
    if (m_sceneEnv)
        updateGridFlags();
}

void QQuick3DInfiniteGrid::updateGridFlags()
{
    enum GridFlags { NoFlag = 0, DrawAxis = 1 };
    uint newFlags = m_gridAxes ? DrawAxis : NoFlag;
    m_sceneEnv->setGridFlags(newFlags);
}

QT_END_NAMESPACE
