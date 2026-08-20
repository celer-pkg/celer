// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls

QtObject {
    id: root

    property var supportedStates: [
        [],
        ["disabled"],
    ]

    property ListModel model: ListModel {
        id: fruitModel

        ListElement {
            name: "Apple"
            color: "green"
        }
        ListElement {
            name: "Cherry"
            color: "red"
        }
        ListElement {
            name: "Banana"
            color: "yellow"
        }
        ListElement {
            name: "Orange"
            color: "orange"
        }
        ListElement {
            name: "WaterMelon"
            color: "pink"
        }
    }

    component BaseSearchField: SearchField {
        textRole: "name"
        enabled: !is("disabled")
        suggestionModel: root.model
    }

    property Component component: Column {
        spacing: 10

        BaseSearchField {}

        BaseSearchField {
            text: "Live SearchField"
        }

        BaseSearchField {
            text: "No search indicator"
            searchIndicator.indicator: null
        }

        BaseSearchField {
            text: "No clear indicator"
            clearIndicator.indicator: null
        }

        BaseSearchField {
            text: "No indicators"
            searchIndicator.indicator: null
            clearIndicator.indicator: null
        }
    }
}
