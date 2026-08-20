// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ToolBar {

    property bool busyIndicatorRunning : false
    property bool searchBarVisbile: true

    signal doSearch(string searchText)
    signal searchTextChanged(string searchText)
    signal showCategories()
    signal goBack()
    signal showMap()

    onSearchBarVisbileChanged: {
        searchBar.opacity = searchBarVisbile ? 1 : 0
        backBar.opacity = searchBarVisbile ? 0 : 1
    }

    function showSearch(text) {
        if (text !== null) {
            searchText.ignoreTextChange = true
            searchText.text = text
            searchText.ignoreTextChange = false
        }
    }

    height: searchBar.height
    RowLayout {
        id: searchBar
        width: parent.width
        Behavior on opacity { NumberAnimation{} }
        visible: opacity ? true : false
        TextField {
            id: searchText
            Behavior on opacity { NumberAnimation{} }
            visible: opacity ? true : false
            property bool ignoreTextChange: false
            placeholderText: qsTr("Type place...")
            Layout.fillWidth: true
            Layout.topMargin: 2
            Layout.bottomMargin: 2
            onTextChanged: {
                if (!ignoreTextChange)
                    searchTextChanged(text)
            }
            onAccepted: doSearch(searchText.text)
        }
        ToolButton {
            id: searchButton
            icon.source: Qt.resolvedUrl("../resources/search.png")
            onClicked: doSearch(searchText.text)
            Layout.topMargin: 2
            Layout.bottomMargin: 2
        }
        ToolButton {
            id: categoryButton
            icon.source: Qt.resolvedUrl("../resources/categories.png")
            onClicked: showCategories()
            Layout.topMargin: 2
            Layout.bottomMargin: 2
        }
    }

    RowLayout {
        id: backBar
        width: parent.width
        height: parent.height
        opacity: 0
        Behavior on opacity { NumberAnimation{} }
        visible: opacity ? true : false
        ToolButton {
            id: backButton
            icon.source: Qt.resolvedUrl("../resources/left.png")
            onClicked: goBack()
        }
        ToolButton {
            id: mapButton
            icon.source: Qt.resolvedUrl("../resources/search.png")
            onClicked: showMap()
        }
        Item {
             Layout.fillWidth: true
        }
    }
}
