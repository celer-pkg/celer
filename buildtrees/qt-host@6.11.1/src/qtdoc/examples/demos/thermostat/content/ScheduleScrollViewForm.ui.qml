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
import QtQuick.Layouts
import Thermostat

ScrollView {
    id: scrollView

    required property Room room
    required property ScheduleViewForm scheduleViewRoot
    property bool isBackgroundVisible: false
    property int timeScheduleHeight: 361
    property int timeScheduleWidth: 1087
    property int tempSetterHeight: 427
    property int tempSetterWidth: 1087

    clip: true
    bottomPadding: 10
    contentWidth: availableWidth

    background: Rectangle {
        color: Constants.accentColor
        visible: scrollView.isBackgroundVisible
        radius: 12
    }

    ColumnLayout {
        id: layout
        anchors.horizontalCenter: parent.horizontalCenter

        TimeSchedule {
            id: timeSchedule
            Layout.preferredHeight: scrollView.timeScheduleHeight
            Layout.preferredWidth: scrollView.timeScheduleWidth
            scheduleViewRoot: scrollView.scheduleViewRoot
        }

        TemperatureSetter {
            Layout.preferredHeight: scrollView.tempSetterHeight
            Layout.preferredWidth: scrollView.tempSetterWidth
            scheduleViewRoot: scrollView.scheduleViewRoot
        }
    }

    states: [
        State {
            name: "bigDesktopLayout"
            when: Constants.isBigDesktopLayout
            PropertyChanges {
                target: scrollView
                timeScheduleHeight: 361
                timeScheduleWidth: layout.width
                tempSetterHeight: 350
                tempSetterWidth: layout.width
                isBackgroundVisible: false
            }
            PropertyChanges {
                target: layout
                width: 1100
            }
        },
        State {
            name: "smallDesktopLayout"
            when: Constants.isSmallDesktopLayout
            PropertyChanges {
                target: scrollView
                timeScheduleHeight: 361
                timeScheduleWidth: layout.width
                tempSetterHeight: 350
                tempSetterWidth: layout.width
                isBackgroundVisible: false
            }
            PropertyChanges {
                target: layout
                width: 918
            }
        },
        State {
            name: "mobileLayout"
            when: Constants.isMobileLayout
            PropertyChanges {
                target: scrollView
                timeScheduleHeight: 314
                timeScheduleWidth: layout.width
                tempSetterHeight: 450
                tempSetterWidth: layout.width
                isBackgroundVisible: false
            }
            PropertyChanges {
                target: layout
                width: 334
            }
        },
        State {
            name: "smallLayout"
            when: Constants.isSmallLayout
            PropertyChanges {
                target: scrollView
                timeScheduleHeight: 230
                timeScheduleWidth: layout.width
                tempSetterHeight: 300
                tempSetterWidth: layout.width
                isBackgroundVisible: true
            }
            PropertyChanges {
                target: layout
                width: 427
            }
        }
    ]
}
