// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

import xr_shared

pragma ComponentBehavior: Bound

Node {
    id: selectionBox
    property Model selectedObject: null

    property XrGadget activeGadget: null
    readonly property bool gadgetActive: !!activeGadget

    visible: !!selectedObject
    position: selectedObject?.scenePosition || Qt.vector3d(0, 0, 0)
    rotation: selectedObject?.sceneRotation || Qt.quaternion(1, 0, 0, 0)


    function selectGadget(gadget: XrGadget) {
        if (activeGadget && activeGadget !== gadget)
            activeGadget.selected = false
        if (gadget)
            gadget.selected = true
        activeGadget = gadget
    }

    function handleHover(obj: Model) {
        const gadget = obj as XrGadget
        selectGadget(gadget)
    }

    enum Type {
        Translate,
        Rotate,
        Resize
    }

    property int gadgetType: 0

    function handlePress(obj: Model, pos: vector3d, direction: vector3d) {
        const gadget = obj as XrGadget
        if (gadget) {
            selectGadget(gadget)
            gadget.handleControllerPress(pos, direction)
        } else if (obj === selectedObject) {
            // switch to next gadget type
            gadgetType = (gadgetType + 1) % 3
        } else {
            selectedObject = obj
        }
    }

    function handleMove(pos: vector3d, direction: vector3d) {
        if (activeGadget)
            activeGadget.handleControllerMove(pos, direction)
    }

    function handleRelease() {
    }

    Model {
        id: selectionModel
        source: "#Cube"
        property bounds b: selectionBox.selectedObject?.bounds || bounds // just to silence the warning when selectedObject is null
        property vector3d s: selectionBox.selectedObject?.scale || Qt.vector3d(0, 0, 0)
        property real d: 5
        property vector3d min: Qt.vector3d(b.minimum.x, b.minimum.y, b.minimum.z).times(s)
        property vector3d max: Qt.vector3d(b.maximum.x, b.maximum.y, b.maximum.z).times(s)
        scale: max.minus(min).plus(Qt.vector3d(d, d, d)).times(0.01)
        position: max.plus(min).times(0.5)
        materials: PrincipledMaterial {
            baseColor: "yellow"
            emissiveFactor: Qt.vector3d(0.1, 0.1, 0.1)
        }
        opacity: 0.3
    }

    Node {
        id: translaters
        TranslateGadget {
            id: translateGadgetX
            axis: 0
            x: selectionModel.max.x
            controlledObject: selectionBox.selectedObject
        }
        TranslateGadget {
            id: translateGadgetY
            axis: 1
            y: selectionModel.max.y
            controlledObject: selectionBox.selectedObject
        }
        TranslateGadget {
            id: translateGadgetZ
            axis: 2
            z: selectionModel.max.z
            controlledObject: selectionBox.selectedObject
        }
        visible: selectionBox.gadgetType === GadgetBox.Type.Translate
    }

    Node {
        id: rotaters
        RotateGadget {
            axis: 0
            x: selectionModel.max.x + 1.5
            controlledObject: selectionBox.selectedObject
        }
        RotateGadget {
            axis: 1
            y: selectionModel.max.y + 1.5
            controlledObject: selectionBox.selectedObject
        }
        RotateGadget {
            axis: 2
            z: selectionModel.max.z + 1.5
            controlledObject: selectionBox.selectedObject
        }
        visible: selectionBox.gadgetType === GadgetBox.Type.Rotate
    }

    Node {
        id: resizors
        ResizeGadget {
            axis: 0
            x: selectionModel.max.x
            controlledObject: selectionBox.selectedObject
        }
        ResizeGadget {
            axis: 1
            y: selectionModel.max.y
            controlledObject: selectionBox.selectedObject
        }
        ResizeGadget {
            axis: 2
            z: selectionModel.max.z
            controlledObject: selectionBox.selectedObject
        }
        visible: selectionBox.gadgetType === GadgetBox.Type.Resize
    }
}
