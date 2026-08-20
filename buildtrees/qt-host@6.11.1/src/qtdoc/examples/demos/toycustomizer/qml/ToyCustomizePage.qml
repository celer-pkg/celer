// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: toyCustomizePage

    property alias toyIndex: toyView.toyIndex
    property bool reset: false
    required property AccessoryModel accessoryModel

    signal cancelled
    signal confirmed
    signal showMaximizeViewRequested(page: Component)
    signal hideMaximizeViewRequested

    Component.onCompleted: {
        if (reset)
            accessoryModel.resetAllAccessories()
    }

    ColumnLayout {
        id: portraitGridLayout
        visible: ApplicationConfig.isPortrait
        anchors.fill: parent

        LayoutItemProxy {
            target: toyView
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: ApplicationConfig.responsiveSize(1051)
            Layout.preferredHeight: ApplicationConfig.responsiveSize(1181)
            Layout.leftMargin: ApplicationConfig.responsiveSize(100)
            Layout.rightMargin: ApplicationConfig.responsiveSize(100)
        }

        LayoutItemProxy {
            target: accessoryView
            Layout.fillWidth: true
        }
    }

    RowLayout {
        id: landscapeLayout
        visible: !ApplicationConfig.isPortrait
        anchors {
            fill: parent
            leftMargin: ApplicationConfig.responsiveSize(100)
            rightMargin: ApplicationConfig.responsiveSize(100)
        }
        spacing: ApplicationConfig.responsiveSize(100)

        LayoutItemProxy {
            target: toyView
            implicitHeight: ApplicationConfig.responsiveSize(1602)
            Layout.fillWidth: true
        }

        LayoutItemProxy {
            target: accessoryView
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumHeight: ApplicationConfig.responsiveSize(1366)
        }
    }

    Component {
        id: maximizeView
        MaximizeView {
            accessoryModel: toyCustomizePage.accessoryModel
            toyIndex: toyCustomizePage.toyIndex
            onHideRequested: toyCustomizePage.hideMaximizeViewRequested()
        }
    }

    AccessoryModel {
        id: __accessoryModel
    }

    ToyView {
        id: toyView
        implicitWidth: ApplicationConfig.responsiveSize(2080)
        accessoryModel: toyCustomizePage.accessoryModel
        onHideRequested: toyCustomizePage.cancelled()
        onShowRequested: toyCustomizePage.showMaximizeViewRequested(maximizeView)
        onConfirmRequested: toyCustomizePage.confirmed()
    }

    AccessoryView {
        id: accessoryView
        implicitWidth: ApplicationConfig.responsiveSize(2080)
        target: toyView.toy
        model: toyCustomizePage.accessoryModel
    }
}
