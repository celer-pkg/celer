// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D.Helpers
import QtQuick3D
import QtQuick3D.Xr
import QtQuick.Layouts
import QtQuick.Controls

import xr_shared

XrView {
    id: xrView

    referenceSpace: XrView.ReferenceSpaceLocal

    passthroughEnabled: passthroughSupported
    environment: SceneEnvironment {
        clearColor: "skyblue"
        backgroundMode: xrView.passthroughEnabled ? SceneEnvironment.Transparent : SceneEnvironment.Color
    }

    xrOrigin: theOrigin
    XrOrigin {
        id: theOrigin
        TouchHand {
            id: rightHand
            view: xrView
            hand: XrHandModel.RightHand
            touchId: 1
            color: "green"
        }
        TouchHand {
            id: leftHand
            view: xrView
            hand: XrHandModel.LeftHand
            touchId: 2
            color: "red"
        }
    }

    DirectionalLight {
        eulerRotation.x: -30
        eulerRotation.y: -70
        ambientColor: "#777"
    }

    Scene {
        id: theScene
        //! [color change]
        y: xrView.referenceSpace === XrView.ReferenceSpaceLocalFloor ? 130 : 0
        onColorChange: (col, id) => {
            if (id === 1)
                rightHand.color = col
            else
                leftHand.color = col
        }
        //! [color change]
    }
}
