// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import QtTest
import QtQuick.Controls
import Qt.test.controls

TestCase {
    id: testCase
    width: 400
    height: 400
    visible: true
    when: windowShown
    name: "SearchField"

    Component {
        id: signalSpy
        SignalSpy { }
    }

    Component {
        id: searchField
        SearchField { }
    }

    Component {
        id: searchText
        SearchField {
            TextField{ }
        }
    }

    function init() {
        failOnWarning(/.?/)
    }

    function test_defaults() {
        let control = createTemporaryObject(searchField, testCase)
        verify(control)

        compare(control.suggestionModel, undefined)
        compare(control.suggestionCount, 0)
        compare(control.currentIndex, -1)
        compare(control.highlightedIndex, -1)
        compare(control.text, "")
        compare(control.textRole, "")
        compare(control.live, true)
        verify(control.delegate)
        if (StyleInfo.styleName !== "iOS")
            verify(control.popup)
        else
            compare(StyleInfo.styleName, "iOS")
    }

    // TO-DO: Implement SFPM logic after 6.10
    // ListModel {
    //     id: specialCharModels
    //     ListElement { text: "こんにちは" }
    //     ListElement { text: "Pi: π (3.14)"; }
    //     ListElement { text: "Math: ∑ ∞ ≈"; }
    //     ListElement { text: "Emoji: 😃🎉🔥"; }
    //     ListElement { text: "Currency: € ¥ ₹ $"; }
    //     ListElement { text: "α β γ"; }
    //     ListElement { text: "Привет"; }
    //     ListElement { text: "مرحبًا"; }
    //     ListElement { text: "你好"; }
    //     ListElement { text: "שלום"; }
    //     ListElement { text: "Brackets: { [ ( < > ) ] }"; }
    // }

    // function test_specialCharacters() {
    //     let control = createTemporaryObject(searchField, testCase)
    //     verify(control)

    //     control.suggestionModel = specialCharModels
    //     let textItem = control.contentItem
    //     textItem.text = "e"

    //     compare(control.text, "e")
    //     compare(control.suggestionCount, 3)
    //     compare(control.currentIndex, 0)
    //     compare(control.popup.visible, true)

    //     textItem.text = "П"

    //     compare(control.text, "П")
    //     compare(control.suggestionCount, 1)
    //     compare(control.currentIndex, 0)
    //     compare(control.popup.visible, true)

    //     textItem.text = "🎉"

    //     compare(control.text, "🎉")
    //     compare(control.suggestionCount, 1)
    //     compare(control.currentIndex, 0)
    //     compare(control.popup.visible, true)
    // }

    ListModel {
        id : fruitModel
        ListElement { name: "Apple"; color: "green" }
        ListElement { name: "Cherry"; color: "red" }
        ListElement { name: "Banana"; color: "yellow" }
        ListElement { name: "Orange"; color: "orange" }
        ListElement { name: "WaterMelon"; color: "pink" }
    }

    function test_textRole() {
        if (StyleInfo.styleName !== "iOS")
            ignoreWarning(/Unable to assign QQmlDMAbstractItemModelData to QString/)

        let control = createTemporaryObject(searchField, testCase)
        verify(control)

        control.suggestionModel = fruitModel
        control.textRole = "name"

        let textItem = control.contentItem
        textItem.text = "a"

        compare(control.text, "a")
        compare(control.suggestionCount, 5)
        if (StyleInfo.styleName !== "iOS")
            compare(control.popup.visible,true)
        else
            compare(StyleInfo.styleName, "iOS")

        control.textRole = "color"

        textItem.text = "r"

        compare(control.text, "r")
        compare(control.suggestionCount, 5)
        if (StyleInfo.styleName !== "iOS")
            compare(control.popup.visible,true)
        else
            compare(StyleInfo.styleName, "iOS")
    }

    Component {
        id: suggestion
        SearchField {
            onTextEdited: {
                if (text === "a") {
                    suggestionModel = ["Apple", "Apricot"]
                } else if (text === "c") {
                    suggestionModel = ["Cherry", "Coconut", "Cranberry"]
                }
            }
        }
    }

    function test_suggestionPopup() {
        if (StyleInfo.styleName === "iOS")
            skip("iOS style does not provide a popup for SearchField.")

        let control = createTemporaryObject(suggestion, testCase)
        verify(control)

        compare(control.popup.visible, false)

        let textItem = control.contentItem

        textItem.text = "a"
        compare(control.suggestionCount, 2)
        compare(control.currentIndex, -1)
        compare(control.highlightedIndex, 0)
        compare(control.popup.visible, true)

        textItem.text = "c"
        compare(control.suggestionCount, 3)
        compare(control.currentIndex, -1)
        compare(control.highlightedIndex, 0)
        compare(control.popup.visible, true)
    }

    function test_textEdited() {
        let control = createTemporaryObject(searchField, testCase)
        verify(control)

        let textEditedSpy = signalSpy.createObject(control, {target: control, signalName: "textEdited"})
        verify(textEditedSpy.valid)

        let searchTriggeredSpy = signalSpy.createObject(control, {target: control, signalName: "searchTriggered"})
        verify(searchTriggeredSpy.valid)

        control.live = true
        let textItem = control.contentItem
        textItem.text = "a"

        compare(control.text, "a")
        compare(control.currentIndex, -1)
        compare(textEditedSpy.count, 1)

        compare(searchTriggeredSpy.count, 1)
    }

    function test_currentIndexResetsOnEdit() {
        if (StyleInfo.styleName === "iOS")
            skip("iOS style does not provide a popup for SearchField.")

        ignoreWarning(/Unable to assign QQmlDMAbstractItemModelData to QString/)

        let control = createTemporaryObject(searchField, testCase)
        verify(control)

        control.forceActiveFocus()
        verify(control.activeFocus)

        control.suggestionModel = fruitModel
        control.textRole = "name"

        let textItem = control.contentItem
        textItem.text = "a"

        compare(control.popup.visible, true)

        compare(control.currentIndex, -1)
        compare(control.highlightedIndex, 0)

        keyClick(Qt.Key_Down)
        compare(control.currentIndex, -1)
        compare(control.highlightedIndex, 1)

        keyClick(Qt.Key_Enter)
        compare(control.currentIndex, 1)
        compare(control.popup.visible, false)

        textItem.text = control.text + "x"
        compare(control.currentIndex, -1)
    }

    function test_arrowKeys() {
        if (StyleInfo.styleName === "iOS")
            skip("iOS style does not provide a popup for SearchField.")

        ignoreWarning(/Unable to assign QQmlDMAbstractItemModelData to QString/)

        let control = createTemporaryObject(searchField, testCase)
        verify(control)

        let activatedSpy = signalSpy.createObject(control, {target: control, signalName: "activated"})
        verify(activatedSpy.valid)

        let highlightedSpy = signalSpy.createObject(control, {target: control, signalName: "highlighted"})
        verify(highlightedSpy.valid)

        let openedSpy = signalSpy.createObject(control, {target: control.popup, signalName: "opened"})
        verify(openedSpy.valid)

        let closedSpy = signalSpy.createObject(control, {target: control.popup, signalName: "closed"})
        verify(closedSpy.valid)

        let acceptedSpy = signalSpy.createObject(control, {target: control, signalName: "accepted"})
        verify(closedSpy.valid)

        let searchTriggeredSpy = signalSpy.createObject(control, {target: control, signalName: "searchTriggered"})
        verify(searchTriggeredSpy.valid)

        control.forceActiveFocus()
        verify(control.activeFocus)

        compare(control.currentIndex, -1)
        compare(control.highlightedIndex, -1)

        control.suggestionModel = fruitModel
        control.textRole = "name"

        let textItem = control.contentItem
        textItem.text = "a"

        compare(control.popup.visible, true)

        keyClick(Qt.Key_Down)
        compare(control.currentIndex, -1)
        compare(control.highlightedIndex, 1)
        compare(activatedSpy.count, 0)
        compare(highlightedSpy.count, 1)
        compare(highlightedSpy.signalArguments[0][0], 1)
        highlightedSpy.clear()

        keyClick(Qt.Key_Down)
        compare(control.currentIndex, -1)
        compare(control.highlightedIndex, 2)
        compare(activatedSpy.count, 0)
        compare(highlightedSpy.count, 1)
        compare(highlightedSpy.signalArguments[0][0], 2)
        highlightedSpy.clear()

        keyClick(Qt.Key_Up)
        compare(control.currentIndex, -1)
        compare(control.highlightedIndex, 1)
        compare(activatedSpy.count, 0)
        compare(highlightedSpy.count, 1)
        compare(highlightedSpy.signalArguments[0][0], 1)
        highlightedSpy.clear()

        keyClick(Qt.Key_Enter)
        compare(control.text, "Cherry")
        compare(control.currentIndex, 1)
        compare(acceptedSpy.count, 1)
        compare(searchTriggeredSpy.count, 2)

        keyClick(Qt.Key_Back)
        compare(control.popup.visible, false)

        keyClick(Qt.Key_Escape)
        compare(control.text, "")
    }

    Component {
        id: delegateComponent1

        ItemDelegate {}
    }

    Component {
        id: delegateComponent2

        ItemDelegate {}
    }

    function test_dontDeleteDelegates() {
        let control = createTemporaryObject(searchField, testCase, { delegate: delegateComponent1 })
        verify(control)

        // When setting a new delegate, the old one shouldn't be destroyed.
        control.delegate = delegateComponent2
        verify(delegateComponent1)

        // The same goes for the new delegate: it shouldn't be destroyed when setting the old one.
        control.delegate = delegateComponent1
        verify(delegateComponent2)
    }
}
