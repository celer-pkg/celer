// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import Thermostat

Window {
    id: window

    width: 1440
    height: 1080

    minimumHeight: 306
    minimumWidth: Constants.isMobileLayout ? 364 : 484

    visible: true
    title: "Thermostat"

    HomePage {
        id: mainScreen
        anchors.fill: parent
    }

    Component.onCompleted: {
        Constants.layout = Qt.binding(() => {
            let tall = window.height >= window.width
            if (window.width >= 1440 && window.height >= 520)
                return Constants.Layout.Desktop
            if (window.width >= 1024 && window.height >= 768)
                return Constants.Layout.SmallDesktop
            if (tall || (window.width >= 600 && window.height >= 380))
                return Constants.Layout.Mobile
            return Constants.Layout.Small
        })
    }
}
