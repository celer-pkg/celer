// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

QtObject {
    property int rotation1Angle
    property int rotation2Angle
    property int rotation3Angle
    property int rotation4Angle
    property int clawsAngle
    readonly property string status: "Ready"

    Behavior on rotation1Angle {
        SmoothedAnimation {
            velocity: 100
        }
    }

    Behavior on rotation2Angle {
        SmoothedAnimation {
            velocity: 80
        }
    }

    Behavior on rotation3Angle {
        SmoothedAnimation {
            velocity: 60
        }
    }

    Behavior on rotation4Angle {
        SmoothedAnimation {
            velocity: 60
        }
    }

    Behavior on clawsAngle {
        SmoothedAnimation {}
    }
}
