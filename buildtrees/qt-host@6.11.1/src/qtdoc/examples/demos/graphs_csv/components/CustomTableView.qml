// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic
import qtgraphscsv

Item {
    id: tableviewItem

    property alias horizontalHeaderview: hHeaderView
    property alias verticalalHeaderview: vHeaderView
    property alias mainTableView: tv
    property alias tableViewModel: tv.model
    property alias horizontalHeaderViewModel: hHeaderView.model
    property alias title: titleLabel.text
    property alias dataSelectionModel: tv.selectionModel

    property color borderColor
    property color primaryTextColor
    property color secondaryTextColor
    property color selectionColor: "#E3E3E3"
    property color backgroundColor: "#FCFCFC"
    property color scrollbarBackgroundColor: "#BEBEBE"

    readonly property font titleFont: ({
            family: "Inter",
            weight: 700 * Units.px,
            pixelSize: 16 * Units.px,
            letterSpacing: 0 * Units.px,
            bold: false
        })

    function extractBarSetGategories(first, count) {
        let categories = [];
        const last = first + count;
        for (let i = first; i < last; ++i)
            categories.push(tv.model.headerData(i, Qt.Horizontal, Qt.DisplayRole) + " medals");
        return categories;
    }

    function fillHorizontalHeaderModel(rowLength) {
        hHeaderView.model.clear();
        for (let i = 0; i < rowLength; ++i) {
            let h = tv.model.headerData(i, Qt.Horizontal, Qt.DisplayRole);
            hHeaderView.model.append({
                "display": h
            });
        }
    }

    Text {
        id: titleLabel
        text: ""

        font: tableviewItem.titleFont
        color: tableviewItem.primaryTextColor
        width: 132 * Units.px
        height: 16 * Units.px
        anchors.top: tableviewItem.top
        anchors.left: vHeaderView.left
        anchors.right: tableviewItem.right
        anchors.rightMargin: 236 * Units.px
        anchors.leftMargin: 8 * Units.px
        verticalAlignment: Text.AlignBottom
    }

    VerticalHeaderView {
        id: vHeaderView

        implicitWidth: 91 * Units.px
        implicitHeight: 320 * Units.px
        clip: true
        anchors.top: hHeaderView.bottom
        anchors.bottom: tv.bottom
        boundsBehavior: Flickable.StopAtBounds
        syncView: tv
        delegate: VerticalHeaderDelegate {
            textColor: tableviewItem.primaryTextColor
            borderColor: tableviewItem.borderColor
        }
    }

    HorizontalHeaderView {
        id: hHeaderView
        implicitWidth: 250 * Units.px

        anchors.top: titleLabel.bottom
        anchors.topMargin: 25 * Units.px
        anchors.left: vHeaderView.right
        interactive: false
        columnWidthProvider: column => {
            return (column ? tv.implicitColumnWidth(column) : 0);
        }

        delegate: HorizontalHeaderDelegate {
            textColor: tableviewItem.secondaryTextColor
            borderColor: tableviewItem.borderColor
        }
    }

    TableView {
        id: tv

        anchors.top: hHeaderView.bottom
        anchors.left: vHeaderView.right

        implicitWidth: 255 * Units.px
        implicitHeight: 310 * Units.px
        reuseItems: false
        clip: true
        selectionBehavior: TableView.SelectCells
        selectionMode: TableView.ContiguousSelection
        interactive: false

        columnWidthProvider: column => {
            if (column === 0)
                return 0;
        }
        rowHeightProvider: row => {
            if (row === 0)
                return 0;
        }

        ScrollBar.vertical: MyScrollBar {
            parent: tv.parent
            anchors.top: tv.top
            anchors.bottom: tv.bottom
            anchors.left: tv.right
            anchors.leftMargin: 16 * Units.px
        }

        keyNavigationEnabled: false

        selectionModel: ItemSelectionModel {
            id: dataSelectionModelId
        }

        delegate: Rectangle {
            id: delegateRectangle
            implicitHeight: 31 * Units.px
            implicitWidth: 63 * Units.px

            required property bool selected
            required property bool current
            required property string display

            border.width: 1 * Units.px
            border.color: tableviewItem.borderColor
            color: selected ? tableviewItem.selectionColor : tableviewItem.backgroundColor

            Text {
                leftPadding: 38
                topPadding: 6
                bottomPadding: 5
                rightPadding: 11
                color: tableviewItem.primaryTextColor
                text: delegateRectangle.display
            }
        }
    }

    component MyScrollBar: ScrollBar {
        id: scrollBar

        background: Rectangle {
            implicitWidth: 8 * Units.px
            implicitHeight: 320 * Units.px
            radius: 8 * Units.px
            color: tableviewItem.selectionColor
        }

        contentItem: Rectangle {
            implicitWidth: 8 * Units.px
            implicitHeight: 91 * Units.px
            color: tableviewItem.scrollbarBackgroundColor
            radius: 8 * Units.px
        }
    }
}
