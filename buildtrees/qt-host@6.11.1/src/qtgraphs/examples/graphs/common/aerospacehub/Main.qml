// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Universal
import AerospaceHubExample

ApplicationWindow {
    width: GlobalSettings.rootwidth
    height: GlobalSettings.rootheight
    visible: true
    title: "Wind Turbine Dashboard"

    FontLoader {
        id: interFont
        source: "./fonts/Inter-VariableFont_opsz,wght.ttf"
    }

    font.family: interFont.name

    // Data simulators
    PieSimulator {
        id: piesim
        max: 1000.0
        min: 1.0
        numberOfData: 3
        live: GlobalSettings.livedata
        deviation: 1
        updatePeriod: GlobalSettings.updateinterval * 3

        Component.onCompleted: {
            generateData()
        }
    }

    BarSimulator {
        id: barsim
        max: 50
        min: 1.0
        numberOfData: 3
        numberOfCategory: 12
        live: GlobalSettings.livedata
        deviation: 1
        updatePeriod: GlobalSettings.updateinterval * 50

        Component.onCompleted: {
            generateData()
        }
    }

    LineSimulator {
        id: linesim
        max: 2.0
        min: 0.0
        numberOfData: 25
        order: LineSimulator.SortingOrder.Ascending
        deviation: 2
        live: GlobalSettings.livedata
        updatePeriod: GlobalSettings.updateinterval * 10

        Component.onCompleted: {
            generateData()
            GlobalSettings.area2ddataready = true
            GlobalSettings.area2ddataready = false
        }

        onSimulateDataCompleted: {
            GlobalSettings.area2ddataready = true
            GlobalSettings.area2ddataready = false
        }
    }

    Data3DSimulator {
        id: barDataSimulator
        max: 10.0
        min: 1.0
        numberOfRows: GlobalSettings.turbineCountPerAxis
        numberOfColumns: GlobalSettings.turbineCountPerAxis
        dataModel: GlobalSettings.barDataModel
        live: GlobalSettings.livedata
        updatePeriod: GlobalSettings.updateinterval * 100
        deviation: 2

        Component.onCompleted: {
            generateData()
        }
    }

    Data3DSimulator {
        id: scatterDataSimulator
        max: 10.0
        min: 0.0
        numberOfRows: GlobalSettings.turbineCountPerAxis
        numberOfColumns: 5
        numberOfData: 5
        dataModel: GlobalSettings.scatterDataModel
        live: GlobalSettings.livedata
        updatePeriod: GlobalSettings.updateinterval * 100
        deviation: 10

        Component.onCompleted: {
            generateData()
        }
    }

    Data3DSimulator {
        id: surfaceDataSimulator
        max: 10.0
        min: 0.0
        numberOfRows: 20
        numberOfColumns: 20
        dataModel: GlobalSettings.surfaceDataModel
        order: Data3DSimulator.SortingOrder.Ascending
        deviation: 5
        live: GlobalSettings.livedata
        updatePeriod: GlobalSettings.updateinterval * 100

        Component.onCompleted: {
            generateData()
        }
    }

    Timer {
        // In case graph data cannot be live (as in Wasm), animate at least these values
        running: GlobalSettings.livedata || GlobalSettings.hidelivedata
        interval: GlobalSettings.updateinterval * 100
        repeat: true
        onTriggered: {
            GlobalSettings.winddir += Math.floor(Math.random() * 3) - 1
            GlobalSettings.windspeed += Math.floor(Math.random() * 3) - 1
            if (GlobalSettings.winddir > 360)
                GlobalSettings.winddir = 0
            else if (GlobalSettings.winddir < 0)
                GlobalSettings.winddir = 360
        }
    }

    // Root item
    Item {
        id: root
        anchors.fill: parent

        state: "ShowDashboard"

        property alias settingsstate: settingscontent.settingsstate

        // Update global settings that rely on window size
        onHeightChanged: {
            GlobalSettings.rootheight = height
        }

        onWidthChanged: {
            GlobalSettings.rootwidth = width
        }

        //! [rootitem]
        // Background image
        Image {
            anchors.fill: parent
            source: root.state === "ShowSettings" ? "images/settings-bg.png"
                                                  : "images/dashboard-bg.png"
            fillMode: Image.PreserveAspectCrop
        }

        // Everything except background image
        ColumnLayout {
            anchors.fill: parent
            //! [rootitem]
            anchors.leftMargin: GlobalSettings.doublespacing
            anchors.rightMargin: GlobalSettings.doublespacing
            anchors.bottomMargin: GlobalSettings.defaultspacing

            // Header; logos and title
            Item {
                id: header
                Layout.fillWidth: true
                Layout.preferredHeight: GlobalSettings.headerheight
                Layout.leftMargin: -GlobalSettings.halfspacing

                Image {
                    id: icon
                    anchors.verticalCenter: parent.verticalCenter
                    source: "images/turbine-logo.png"
                    height: GlobalSettings.imagesize110px
                    sourceSize.width: 110
                    sourceSize.height: 110
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    anchors.verticalCenter: icon.verticalCenter
                    anchors.left: icon.right
                    anchors.leftMargin: 30
                    text: "AeroScope Hub"
                    color: "#EFEFEF"
                    verticalAlignment: Qt.AlignVCenter
                    font.pixelSize: GlobalSettings.fontsize54px
                    font.bold: true
                }

                Image {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    source: "images/BuiltWithQtLogo.png"
                    width: GlobalSettings.qtlogowidth
                    sourceSize.width: 242
                    sourceSize.height: 72
                    fillMode: Image.PreserveAspectFit
                }
            }

            // Everything except header and background image
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: GlobalSettings.doublespacing

                //! [toolbar]
                // Vertical toolbar
                ToolBarItem {
                    Layout.preferredWidth: GlobalSettings.toolbarwidth
                    Layout.fillHeight: true
                }

                // Everything except vertical toolbar, header, and background image
                ColumnLayout {
                    id: maincontentview
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    //! [toolbar]

                    // Status row: back button (when applicable) and view name
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: GlobalSettings.statusheaderheight
                        spacing: GlobalSettings.defaultspacing

                        Button {
                            icon.source: "images/back-icon.png"
                            icon.color: "#EFEFEF"
                            background: Rectangle {
                                anchors.centerIn: parent
                                width: parent.width * 1.1
                                height: width
                                radius: height / 6
                                color: "#80181818"
                                border.width: parent.hovered ? 1 : 0
                                border.color: "#EFEFEF"
                            }
                            visible: root.state !== "ShowDashboard"
                            onClicked: root.state = "ShowDashboard"
                        }

                        Label {
                            id: viewtext
                            text: "Dashboard"
                            font.pixelSize: GlobalSettings.fontsize28px
                            color: "#EFEFEF"
                            visible: root.state !== "ShowDashboard"
                            background: Button {
                                anchors.centerIn: parent
                                width: parent.width * 1.2
                                height: parent.height * 1.2
                                onClicked: root.state = "ShowDashboard"
                                background: Rectangle {
                                    radius: parent.height / 5
                                    color: parent.hovered ? "#181818" : "transparent"
                                }
                            }
                        }

                        Label {
                            id: viewtextdesc
                            text: "Dashboard: Turbine status"
                            font.pixelSize: GlobalSettings.fontsize28px
                            font.bold: true
                            color: "#EFEFEF"
                        }
                    }

                    //! [maincontent]
                    // Main content
                    RowLayout {
                        id: maincontent
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        //! [maincontent]
                        spacing: GlobalSettings.doublespacing

                        //! [dashboard]
                        // Status area; turbine image, notification, status texts, and turbine name
                        StatusPane {
                            id: statuspane
                            Layout.preferredWidth: GlobalSettings.statusimagewidth
                            Layout.fillHeight: true
                        }

                        // Scrollable dashboard graph area, shown when in ShowDashboard state
                        DashboardPane {
                            id: dashboardpane
                            contentheight: statuspane.height * 1.5
                            Layout.preferredWidth: GlobalSettings.dashboardwidth
                            Layout.fillHeight: true
                        }
                        //! [dashboard]

                        //! [detail]
                        // Details graph, shown when in correct state
                        BigGraphItem {
                            id: detailspane
                            Layout.preferredWidth: GlobalSettings.dashboardwidth
                            Layout.fillHeight: true
                            state: root.state
                            radius: GlobalSettings.defaultradius
                            piesimulator: piesim
                            barsimulator: barsim
                            linesimulator: linesim
                            detailsview: true
                        }
                        //! [detail]
                    }
                }
            }
        }

        //! [settings]
        // Settings
        SettingsItem {
            id: settingscontent
            anchors.fill: parent
            //! [settings]
            anchors.leftMargin: GlobalSettings.settingsleftmargin
            anchors.rightMargin: GlobalSettings.defaultmargin
            anchors.topMargin: GlobalSettings.settingstopmargin
            anchors.bottomMargin: GlobalSettings.defaultmargin
            opacity: 0
            piesimulator: piesim
            barsimulator: barsim
            linesimulator: linesim
        }

        //! [states]
        states: [
            State {
                name: "ShowDashboard"
            },
            State {
                name: "ShowSettings"
                PropertyChanges {
                    maincontent.opacity: 0
                    settingscontent.opacity: 1
                }
                PropertyChanges {
                    viewtextdesc.text: "/  Settings"
                }
            },
            //! [states]
            State {
                name: "ShowArea2D"
                PropertyChanges {
                    dashboardpane.opacity: 0
                }
                PropertyChanges {
                    detailspane.opacity: 1
                }
                PropertyChanges {
                    viewtextdesc.text: "/  Power output over time"
                }
            },
            State {
                name: "ShowBar2D"
                PropertyChanges {
                    dashboardpane.opacity: 0
                }
                PropertyChanges {
                    detailspane.opacity: 1
                }
                PropertyChanges {
                    viewtextdesc.text: "/  Energy production per Month"
                }
            },
            State {
                name: "ShowSurface3D"
                PropertyChanges {
                    dashboardpane.opacity: 0
                }
                PropertyChanges {
                    detailspane.opacity: 1
                }
                PropertyChanges {
                    viewtextdesc.text: "/  Power curve visualization"
                }
            },
            State {
                name: "ShowScatter3D"
                PropertyChanges {
                    dashboardpane.opacity: 0
                }
                PropertyChanges {
                    detailspane.opacity: 1
                }
                PropertyChanges {
                    viewtextdesc.text: "/  Fleet-level performance distribution"
                }
            },
            State {
                name: "ShowPie2D"
                PropertyChanges {
                    dashboardpane.opacity: 0
                }
                PropertyChanges {
                    detailspane.opacity: 1
                }
                PropertyChanges {
                    viewtextdesc.text: "/  Component failure distribution"
                }
            },
            State {
                name: "ShowBar3D"
                PropertyChanges {
                    dashboardpane.opacity: 0
                }
                PropertyChanges {
                    detailspane.opacity: 1
                }
                PropertyChanges {
                    viewtextdesc.text: "/  Energy production breakdown x Turbine"
                }
            }
        ]

        //! [transitions]
        // Use transitions to bring the selected view to the front
        transitions: [
            Transition {
                NumberAnimation {
                    properties: "opacity"
                    easing.type: Easing.InOutCubic
                    duration: 1000
                }
            }
        ]
        //! [transitions]
    }
}
