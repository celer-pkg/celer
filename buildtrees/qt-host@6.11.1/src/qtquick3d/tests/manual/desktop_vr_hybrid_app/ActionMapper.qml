// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQuick3D.Xr

Item {
    id: actionMapper

    required property XrInputAction.Hand hand
    property alias triggerPressed: trigger.pressed
    property bool button1Pressed: button1.pressed || trackpad.rightPressed
    property bool button2Pressed: button2.pressed || trackpad.leftPressed

    XrInputAction {
        id: trigger
        hand: actionMapper.hand
        actionId: [XrInputAction.TriggerValue, XrInputAction.TriggerPressed, XrInputAction.IndexFingerPinch]
    }

    XrInputAction {
        id: button1
        hand: actionMapper.hand
        actionId: [XrInputAction.Button1Pressed, XrInputAction.MiddleFingerPinch]
    }

    XrInputAction {
        id: button2
        hand: actionMapper.hand
        actionId: [XrInputAction.Button2Pressed, XrInputAction.ButtonMenuPressed, XrInputAction.RingFingerPinch]
    }

    XrInputAction {
        id: trackpadX
        hand: actionMapper.hand
        actionId: [XrInputAction.TrackpadX]
    }

    XrInputAction {
        id: trackpad
        hand: actionMapper.hand
        actionId: [XrInputAction.TrackpadPressed]
        property bool rightPressed: pressed && trackpadX > 0.1
        property bool leftPressed: pressed && trackpadX < -0.1
    }
}
