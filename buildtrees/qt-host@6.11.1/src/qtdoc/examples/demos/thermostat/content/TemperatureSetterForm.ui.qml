// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls.Basic
import Thermostat

Pane {
    id: root

    required property ScheduleViewForm scheduleViewRoot
    property alias saveButtonDesktop: desktopView.saveButton
    property alias saveButtonMobile: mobileView.saveButton
    property alias saveButtonSmall: smallView.saveButton
    property alias cancelButtonDesktop: desktopView.cancelButton
    property alias cancelButtonMobile: mobileView.cancelButton
    property alias cancelButtonSmall: smallView.cancelButton

    width: 1087

    padding: 0

    background: Rectangle {
        color: Constants.accentColor
        radius: 12
    }

    TemperatureSetterDesktopView {
        id: desktopView
        scheduleViewRoot: root.scheduleViewRoot
        anchors.fill: parent
    }

    TemperatureSetterMobileView {
        id: mobileView
        visible: false
        scheduleViewRoot: root.scheduleViewRoot
    }

    TemperatureSetterSmallView {
        id: smallView
        visible: false
        scheduleViewRoot: root.scheduleViewRoot
    }

    states: [
        State {
            name: "desktopLayout"
            when: Constants.isBigDesktopLayout || Constants.isSmallDesktopLayout
            PropertyChanges {
                target: desktopView
                visible: true
            }
            PropertyChanges {
                target: mobileView
                visible: false
            }
            PropertyChanges {
                target: smallView
                visible: false
            }
        },
        State {
            name: "mobileLayout"
            when: Constants.isMobileLayout
            PropertyChanges {
                target: desktopView
                visible: false
            }
            PropertyChanges {
                target: mobileView
                visible: true
            }
            PropertyChanges {
                target: smallView
                visible: false
            }
        },
        State {
            name: "smallLayout"
            when: Constants.isSmallLayout
            PropertyChanges {
                target: desktopView
                visible: false
            }
            PropertyChanges {
                target: mobileView
                visible: false
            }
            PropertyChanges {
                target: smallView
                visible: true
            }
        }
    ]
}
