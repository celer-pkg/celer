// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property alias headingText: headingLabel.text
    property alias bodyText: bodyLabel.text
    property alias currentPageStep: pageStepper.currentStep
    property alias pageModel: pageStepper.model

    implicitHeight: headerItemLayout.implicitHeight

    signal exitRequested()

    ColumnLayout {
        id: headerItemLayout
        spacing: ApplicationConfig.responsiveSize(100)

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }

        RowLayout {
            Layout.fillWidth: true

            Item {
                implicitHeight: ApplicationConfig.responsiveSize(
                                    ApplicationConfig.isPortrait ? 120 : 240)
                implicitWidth: ApplicationConfig.responsiveSize(
                                    ApplicationConfig.isPortrait ? 384 : 768)

                ToyButton {
                    text: qsTr("Exit")
                    type: ToyButton.Type.Secondary
                    textStyle: ApplicationConfig.isPortrait ? ApplicationConfig.TextStyle.Button_S
                                                            : ApplicationConfig.TextStyle.Button_L

                    verticalPadding: {
                        const verPadding = ApplicationConfig.isPortrait ? 31 : 48
                        return Math.round(ApplicationConfig.responsiveSize(verPadding))
                    }
                    horizontalPadding: {
                        const horPadding = ApplicationConfig.isPortrait ? 40 : 80
                        return Math.round(ApplicationConfig.responsiveSize(horPadding))
                    }

                    anchors.verticalCenter: parent.verticalCenter

                    function __exitIconSize(portrait) {
                        return portrait ? ApplicationConfig.responsiveSize(32)
                                        : ApplicationConfig.responsiveSize(48)
                    }

                    icon {
                        source: "icons/exit.svg"
                        width: __exitIconSize(ApplicationConfig.isPortrait)
                        height: __exitIconSize(ApplicationConfig.isPortrait)
                    }

                    onClicked: root.exitRequested()
                }
            }
            Item {
                implicitHeight: 2
                Layout.fillWidth: true
            }
            ToyImage {
                implicitHeight: ApplicationConfig.responsiveSize(
                                    ApplicationConfig.isPortrait ? 164 : 240)
                implicitWidth: ApplicationConfig.responsiveSize(
                                    ApplicationConfig.isPortrait ? 356 : 768)
                source: "images/appLogo.svg"
            }
            Item {
                implicitHeight: 2
                Layout.fillWidth: true
            }
            ToyImage {
                implicitHeight: ApplicationConfig.responsiveSize(
                                    ApplicationConfig.isPortrait ? 120 : 240)
                implicitWidth: ApplicationConfig.responsiveSize(
                                    ApplicationConfig.isPortrait ? 384 : 768)
                source: "images/builtWithQt.svg"
                color: "black"
            }
        }
        PageStepper {
            id: pageStepper
            Layout.fillWidth: true
            Layout.minimumWidth: ApplicationConfig.responsiveSize(1760)
        }
        ColumnLayout {
            spacing: ApplicationConfig.responsiveSize(72)
            Layout.alignment: Qt.AlignCenter
            ToyLabel {
                id: headingLabel
                color: "#2269EE"
                font {
                    bold: true
                    family: "DynaPuff"
                    pixelSize: ApplicationConfig.responsiveSize(112)
                }
                Layout.alignment: Qt.AlignCenter
            }
            ToyLabel {
                id: bodyLabel
                color: "#6A6A8D"
                font {
                    bold: false
                    family: "DynaPuff"
                    pixelSize: ApplicationConfig.responsiveSize(50)
                }
                Layout.alignment: Qt.AlignCenter
            }
        }
    }
}
