// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

import chat_example

ApplicationWindow {
    id: root
    width: 720
    height: 1280
    visible: true
    title: qsTr("OpenAI Chat example")

    Material.theme: Material.Dark
    Material.accent: Material.Blue
    color: "#121212"

    EntryPage {
        id: entryPage
        onChoiceIsDone: function() {
            if (OpenAIChatManager.characterId == OpenAIChatManager.UserCharacter)
                promtPage.opacity = 1
            else
                entryPage.opacity = 0
        }
    }

    ChatPage {
        id: chatPage
        opacity: 0
        visible: opacity > 0

        onGoBack: function() {
            chatPage.opacity = 0
        }
    }

    UserPromtPage {
        id: promtPage

        width: 450
        height: 450

        opacity: 0
        visible: opacity > 0
        onClosePage: function() {
            promtPage.opacity = 0
        }

        onOpenChatPage: function() {
            promtPage.opacity = 0
        }
    }

    SequentialAnimation {
        id: openChatPageAnim

        NumberAnimation {
            target: entryPage
            property: "opacity"
            to: 0
            duration: 300
        }

        NumberAnimation {
            target: chatPage
            property: "opacity"
            to: 1
            duration: 300
        }
    }

    SequentialAnimation {
        id: openEntryPageAnim

        NumberAnimation {
            target: chatPage
            property: "opacity"
            to: 0
            duration: 300
        }

        NumberAnimation {
            target: entryPage
            property: "opacity"
            to: 1
            duration: 300
        }
    }

    SequentialAnimation {
        id: closeUserPromtPage

        NumberAnimation {
            target: promtPage
            property: "opacity"
            to: 0
            duration: 300
        }
    }

    SequentialAnimation {
        id: openUserPromtPage

        NumberAnimation {
            target: promtPage
            property: "opacity"
            to: 1
            duration: 300
        }
    }

    Connections {
        target: promtPage
        function onClosePage() {
            closeUserPromtPage.start()
        }
    }

    Connections {
        target: promtPage
        function onOpenChatPage() {
            openChatPageAnim.start()
        }
    }

    Connections {
        target: entryPage
        function onChoiceIsDone() {
            if (OpenAIChatManager.characterId == OpenAIChatManager.UserCharacter)
                openUserPromtPage.start()
            else
                openChatPageAnim.start()
        }
    }

    Connections {
        target: chatPage
        function onGoBack() {
            openEntryPageAnim.start()
        }
    }
}

