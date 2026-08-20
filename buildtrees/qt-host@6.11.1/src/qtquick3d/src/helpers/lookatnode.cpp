// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "lookatnode_p.h"

#include <QtMath>

/*!
    \qmltype LookAtNode
    \inqmlmodule QtQuick3D.Helpers
    \inherits Node
    \brief A helper node that is automatically kept pointing at another node.
    \since 6.4

    This helper implements a node that automatically rotates so that it is always
    pointed towards a specified node. The rotation only happens over X and Y axes.

    For example, the following snippet keeps the cylinder pointed at the cube.

    \badcode
        View3D {
            anchors.fill: parent
            camera: camera

            PerspectiveCamera {
                id: camera
            }

            DirectionalLight {}

            LookAtNode {
                target: cube

                Model {
                    id: cylinder
                    source: "#Cone"
                    eulerRotation.x: -90
                    materials: [
                        PrincipledMaterial {}
                    ]
                }
            }

            Model {
                id: cube
                position: Qt.vector3d(300, 300, 0);
                source: "#Cube"
                materials: [ PrincipledMaterial {} ]
            }
        }
    \endcode
*/

/*! \qmlproperty Node LookAtNode::target
    Specifies the target node to look at. The default value is \c{null}.
*/

LookAtNode::LookAtNode()
    : QQuick3DNode()
{
}

LookAtNode::~LookAtNode()
{
}

QQuick3DNode *LookAtNode::target() const
{
    return m_target;
}

void LookAtNode::setTarget(QQuick3DNode *node)
{
    if (node == m_target)
        return;

    if (m_target) {
        disconnect(m_target, &QQuick3DNode::scenePositionChanged, this, &LookAtNode::updateLookAt);
        disconnect(this, &QQuick3DNode::scenePositionChanged, this, &LookAtNode::updateLookAt);
    }

    m_target = node;

    if (m_target) {
        connect(m_target, &QQuick3DNode::scenePositionChanged, this, &LookAtNode::updateLookAt);
        connect(this, &QQuick3DNode::scenePositionChanged, this, &LookAtNode::updateLookAt);
    }

    emit targetChanged();
    updateLookAt();
}

void LookAtNode::updateLookAt()
{
    if (m_target) {
        // Note: This code was originally copied from QQuick3DCamera::lookAt method.

        // Assumption: we never want the node to roll.
        // We use Euler angles here to avoid roll to sneak in through numerical instability.

        const QVector3D targetPosition = m_target->scenePosition();
        auto sourcePosition = scenePosition();

        QVector3D targetVector = sourcePosition - targetPosition;

        float yaw = qRadiansToDegrees(atan2(targetVector.x(), targetVector.z()));

        QVector2D p(targetVector.x(), targetVector.z()); // yaw vector projected to horizontal plane
        float pitch = qRadiansToDegrees(atan2(p.length(), targetVector.y())) - 90;

        const float previousRoll = eulerRotation().z();
        setEulerRotation(QVector3D(pitch, yaw, previousRoll));
    }
}
