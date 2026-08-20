// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: nameTumbler
    implicitWidth: ApplicationConfig.responsiveSize(1152)
    implicitHeight: ApplicationConfig.responsiveSize(775)
    color: "transparent"
    radius: ApplicationConfig.responsiveSize(16)

    property int rowHeight: adjectiveTumbler.height / adjectiveTumbler.visibleItemCount
    property var sourceModel

    SortFilterProxyModel {
        id: adjectivesProxy
        model: nameTumbler.sourceModel
        filters: [
            FunctionFilter {
                component AdjRoleData: QtObject { property string group }
                function filter(d: AdjRoleData) : bool {
                    return d.group === "adjectives"
                }
            }
        ]
        function setSelected(index_) {
            let proxyIndex = -1
            for (let i = 0; i < model.count; ++i) {
                const it = model.get(i)
                if (it.group === "adjectives") {
                    ++proxyIndex
                    if (proxyIndex === index_)
                        it.selected = true
                    else if (it.selected)
                        it.selected = false
                }
            }
        }
    }

    SortFilterProxyModel {
        id: nounProxy
        model: nameTumbler.sourceModel
        filters: [
            FunctionFilter {
                component NounRoleData: QtObject { property string group }
                function filter(d: NounRoleData) : bool {
                    return d.group === "noun"
                }
            }
        ]
        function setSelected(index_) {
            let proxyIndex = -1
            for (let i = 0; i < model.count; ++i) {
                const it = model.get(i)
                if (it.group === "noun") {
                    ++proxyIndex
                    if (proxyIndex === index_)
                        it.selected = true
                    else if (it.selected)
                        it.selected = false
                }
            }
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        y: adjectiveTumbler.y + (adjectiveTumbler.height - nameTumbler.rowHeight) / 2
        width: adjectiveTumbler.width * 2
        height: 1
        color: "#2269EE"
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        y: adjectiveTumbler.y + (adjectiveTumbler.height + nameTumbler.rowHeight) / 2
        width: adjectiveTumbler.width * 2
        height: 1
        color: "#2269EE"
    }

    GridLayout {
        id: nameLayout
        anchors.fill: parent
        anchors.margins: ApplicationConfig.responsiveSize(32)
        rows: 2
        columns: 2
        rowSpacing: ApplicationConfig.responsiveSize(32)

        ToyLabel {
            id: nameTublerTitle
            text: qsTr("Pick a name for your toy!")
            wrapMode: Text.Wrap
            textStyle: ApplicationConfig.TextStyle.H2_Bold

            Layout.row: 0
            Layout.column: 0
            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignHCenter
        }

        Tumbler {
            id: adjectiveTumbler

            model: adjectivesProxy
            currentIndex: 5

            Layout.alignment: Qt.AlignHCenter
            Layout.row: 1
            Layout.column: 0
            Layout.fillWidth: true
            Layout.fillHeight: true

            delegate: Item {
                required property var name
                required property int index
                opacity: 1.0 - Math.abs(Tumbler.displacement) / (Tumbler.tumbler.visibleItemCount / 2)

                ToyLabel {
                    anchors.centerIn: parent
                    text: name
                    color: index === adjectiveTumbler.currentIndex ? "#2269EE" : "#6A6A8D"
                    font {
                        styleName: index === adjectiveTumbler.currentIndex ? "Bold" : "Regular"
                        pixelSize: index === adjectiveTumbler.currentIndex ? 18 : 12
                        family: "DynaPuff"
                    }
                }
            }

            onCurrentIndexChanged: adjectivesProxy.setSelected(currentIndex)
            Component.onCompleted: adjectivesProxy.setSelected(currentIndex)
        }

        Tumbler {
            id: nounTumbler

            model: nounProxy
            currentIndex: 5

            Layout.row: 1
            Layout.column: 1
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.fillHeight: true

            delegate: Item {
                required property var name
                required property int index
                opacity: 1.0 - Math.abs(Tumbler.displacement) / (Tumbler.tumbler.visibleItemCount / 2)

                ToyLabel {
                    anchors.centerIn: parent
                    text: name
                    color: index === nounTumbler.currentIndex ? "#2269EE" : "#6A6A8D"
                    font {
                        styleName: index === nounTumbler.currentIndex ? "Bold" : "Regular"
                        pixelSize: index === nounTumbler.currentIndex ? 18 : 12
                        family: "DynaPuff"
                    }
                }
            }

            onCurrentIndexChanged: nounProxy.setSelected(currentIndex)
            Component.onCompleted: nounProxy.setSelected(currentIndex)
        }
    }
}
