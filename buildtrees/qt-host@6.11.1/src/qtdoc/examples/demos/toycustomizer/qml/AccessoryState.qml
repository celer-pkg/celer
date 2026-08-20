// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton
pragma ComponentBehavior: Bound
import QtQuick

QtObject {
    id: accessoryState

    property bool beanieVisible: false
    property bool capVisible: false
    property bool partyHatVisible: false
    property bool headphonesVisible: false
    property bool wizardHatVisible: false
    property bool whiskersVisible: false
    property bool bandanaVisible: false

    property bool eyePatchVisible: false
    property bool incognitoVisible: false
    property bool monacleVisible: false
    property bool nvGogglesVisible: false
    property bool sunglassesVisible: false
    property bool roundGlassesVisible: false

    property bool smallEyesVisible: true
    property bool cuteEyesVisible: false
    property bool annoyedEyesVisible: false
    property bool surprisedEyesVisible: false
    property bool confusedEyesVisible: false
    property bool powerpuffEyesVisible: false
    property bool wideEyesVisible: false

    property bool butterflyWingsVisible: false
    property bool angelWingsVisible: false
    property bool bowtieVisible: false
    property bool backpackVisible: false
    property bool necktieVisible: false
    property bool metalBracelet_LeftVisible: false
    property bool metalBracelet_RightVisible: false
}
