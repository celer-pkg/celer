// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick3D

Node {
    id: root

    property int cameraIndex: 0
    property list<vector3d> cameraPositions: []
    property list<vector3d> cameraRotations: []

    property Lightmapper lightmapper: null
    property int backgroundMode: 0
    property Texture lightProbe: null
    property bool enableLightmaps: false
    property real ssgiIndirectLightBoost: 1.0
    property real probeExposure: 1.0
}
