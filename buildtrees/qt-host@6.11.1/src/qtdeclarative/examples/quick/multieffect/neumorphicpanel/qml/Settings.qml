// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

QtObject {
    id: rootItem

    property bool showSettingsView: true
    property real settingsViewWidth: 150 + 150 * dp
    property bool showDebug: false
    property bool showCustomMaterial: false

    property real itemSize: 200
    property real blur: 40
    property real radius: 40
    property bool individualRadius: false
    property real radiustr: 40
    property real radiusbr: 40
    property real radiusbl: 40
    property real radiustl: 40
    property real spread: 0.0
    property real offsetX: -20
    property real offsetY: -20
    property real opacity: 0.5

    function resetSettings() {
        settings.itemSize = defaultSettings.itemSize;
        settings.blur = defaultSettings.blur;
        settings.radius = defaultSettings.radius;
        settings.individualRadius = defaultSettings.individualRadius;
        settings.radiustr = defaultSettings.radiustr;
        settings.radiusbr = defaultSettings.radiusbr;
        settings.radiusbl = defaultSettings.radiusbl;
        settings.radiustl = defaultSettings.radiustl;
        settings.spread = defaultSettings.spread;
        settings.offsetX = defaultSettings.offsetX;
        settings.offsetY = defaultSettings.offsetY;
        settings.opacity = defaultSettings.opacity;
        settings.showDebug = defaultSettings.showDebug;
        settings.showCustomMaterial = defaultSettings.showCustomMaterial;
        settingsView.resetPosition();
    }
}
