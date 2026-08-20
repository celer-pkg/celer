// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

pragma ComponentBehavior: Bound

AxisGadget {
    source: "qrc:/qt/qml/xr_shared/meshes/arrow_mesh.mesh"
    //! [onMoved]
    onMoved: (pos, dir) => {
        let moveDirection = delta.normalized()
        let mapped_axis = controlledObject.mapDirectionToScene(axisDirection).normalized()
        let dot = mapped_axis.dotProduct(moveDirection)
        let offset = mapped_axis.times(delta.length() * dot)
        controlledObject.position = originalPos.plus(offset)
    }
    //! [onMoved]
}
