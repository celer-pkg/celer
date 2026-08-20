// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton
import QtQuick
import QtLocation

QtObject {
    readonly property int map: MapType.StreetMap
    readonly property int globe: MapType.TerrainMap
}
