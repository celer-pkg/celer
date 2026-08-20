// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QQEMLib 1.0

Item {
    id: rootItem
    property alias font: textArea.font
    property alias text: textArea.text
    property alias textDocument: textArea.textDocument
    property alias lineColumn: lineNumbers
    property alias textArea: textArea

    // 0 = glsl, 1 = qml
    property int contentType: 0
    // 0 = QML, 1 = VERT, 2 = FRAG
    property int editorIndex: 0

    property int rowHeight: Math.ceil(fontMetrics.lineSpacing)
    property int marginsTop: 0
    property int marginsBottom: 0
    property int marginsLeft: 4
    property int lineCountWidth: 40
    property bool showLineNumbers: true
    property bool editable: true

    // Scroll to show lineNumber
    function scrollToLine(lineNumber) {
        const lineCount = textArea.lineCount;
        let pos = lineNumber / lineCount;
        const padding = codeFlickable.height / 2;
        const maxCY = codeFlickable.contentHeight - codeFlickable.height;
        let cY = codeFlickable.contentHeight * pos - padding;
        cY = Math.max(0, Math.min(maxCY, cY));
        codeFlickable.contentY = cY;
    }

    clip: true
    onWidthChanged: {
        // This is required so texts are not clipped when resizing the
        // split views or switching between design/code.
        textArea.update();
    }

    Flickable {
        id: lineNumberFlickable
        height: parent.height
        width: showLineNumbers ? lineNumbers.width + 2 * marginsLeft + 1 : 0
        contentY: codeFlickable.contentY
        interactive: false
        visible: showLineNumbers
        Column {
            id: lineNumbers
            anchors.left: parent.left
            anchors.leftMargin: marginsLeft
            anchors.topMargin: marginsTop
            y: marginsTop
            width: lineCountWidth
            Repeater {
                id: repeater
                model: textArea.text != "" ? textArea.lineCount : 0
                delegate: Label {
                    // UI combines QML error types
                    readonly property int errorType: (effectManager.effectError.type === 0 || effectManager.effectError.type === 3) ? 0 : effectManager.effectError.type
                    readonly property bool hasError: !editable && editorIndex == errorType && ((index + 1) === effectManager.effectError.line)
                    font: textArea.font
                    width: parent.width
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    height: rowHeight
                    text: (index + 1)
                    color: hasError ? "#e04040" : mainView.foregroundColor1
                }
            }
        }
        Rectangle {
            id: lineNumbersSeperator
            y: 4
            height: Math.max(lineNumbers.height, rootItem.height)
            anchors.left: lineNumbers.right
            anchors.leftMargin: marginsLeft
            width: 1
            color: mainView.foregroundColor1
        }
    }

    Flickable {
        id: codeFlickable
        anchors.left: lineNumberFlickable.right
        anchors.right: parent.right
        height: parent.height
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
            width: 15
            active: true
        }
        ScrollBar.horizontal: ScrollBar {
            width: 15
            active: true
        }

        FontMetrics {
            id: fontMetrics
            font: textArea.font
        }

        SyntaxHighlighter {
            id: syntaxHighlighter
            document: (contentType === 0) ? textArea.textDocument : null
        }

        // Tries to keep rectangle r area inside the visible area
        function ensureVisible(r) {
            if (contentX >= r.x)
                contentX = r.x;
            else if (contentX + width <= r.x + r.width)
                contentX = r.x + r.width - width;
            if (contentY >= r.y)
                contentY = r.y;
            else if (contentY + height <= r.y + r.height)
                contentY = r.y + r.height - height;
        }

        TextArea.flickable: TextArea {
            id: textArea

            property bool stringNotFound: false

            function findAction() {
                stringNotFound = false;
                findBar.setFindString(selectedText);
            }

            // Find and select strings, starting from (optional) position
            function findNext(findString, position) {
                const fString = findString.toLowerCase();
                let startSearchPos = cursorPosition;
                if (position !== undefined)
                    startSearchPos = position;
                let seachText = getText(startSearchPos, textArea.length);
                seachText = seachText.toLowerCase();
                const pos = seachText.indexOf(fString);
                if (pos >= 0) {
                    const startPos = startSearchPos + pos;
                    const endPos = startPos + fString.length;
                    select(startPos, endPos);
                    stringNotFound = false;
                } else if (startSearchPos !== 0) {
                    // If not found, try once for full text (so wrap around)
                    findNext(findString, 0);
                } else {
                    stringNotFound = true;
                }

                ensureSelectionVisible();
            }

            function findPrev(findString, position) {
                const fString = findString.toLowerCase();
                let startSearchPos = cursorPosition;
                if (position !== undefined)
                    startSearchPos = position;
                let seachText = getText(0, startSearchPos - 1);
                seachText = seachText.toLowerCase();
                const pos = seachText.lastIndexOf(fString);
                if (pos >= 0) {
                    const startPos = pos;
                    const endPos = startPos + fString.length;
                    select(startPos, endPos);
                    stringNotFound = false;
                } else if (startSearchPos !== textArea.length) {
                    // If not found, try once for full text (so wrap around)
                    findPrev(findString, textArea.length);
                } else {
                    stringNotFound = true;
                }

                ensureSelectionVisible();
            }

            // Keep selection in visible area manually, see QTBUG-105957
            function ensureSelectionVisible() {
                // Keep selection in visible area.
                // First try top-left and then selectionEnd, this way view is as
                // top & left as possible.
                var p1 = Qt.rect(0, 0, 0, 0);
                var p2 = positionToRectangle(textArea.selectionEnd);
                p2 = addMarginToRect(p2);
                codeFlickable.ensureVisible(p1);
                codeFlickable.ensureVisible(p2);
                codeFlickable.returnToBounds();
            }

            function addMarginToRect(rect) {
                const margin = 100;
                const maxWidth = codeFlickable.contentWidth - codeFlickable.width;
                const maxHeight = codeFlickable.contentHeight - codeFlickable.height;
                let marginRect = rect;
                marginRect.x = Math.max(marginRect.x - margin, 0);
                marginRect.y = Math.max(marginRect.y - margin, 0);
                marginRect.width = Math.min(marginRect.width + (2 * margin), maxWidth);
                marginRect.height = Math.min(marginRect.height + (2 * margin), maxHeight);
                return marginRect;
            }

            textFormat: Qt.PlainText
            focus: false
            selectByMouse: true
            leftPadding: rootItem.marginsLeft
            rightPadding: rootItem.marginsLeft
            topPadding: rootItem.marginsTop
            bottomPadding: rootItem.marginsBottom
            tabStopDistance: fontMetrics.averageCharacterWidth * 4;
            readOnly: !rootItem.editable
            // Override the default background
            background: Item {}
            font.family: codeFont ? codeFont.font.family : ""
            font.pointSize: effectManager.settings.codeFontSize

            Keys.onPressed:
                (event)=> {
                    if (!editable)
                        return;

                    if ((event.modifiers & Qt.ControlModifier) && (event.key === Qt.Key_I)) {
                        effectManager.autoIndentCurrentCode(tabBarEditors.currentIndex, textArea.text);
                        event.accepted = true;
                    } else {
                        event.accepted = effectManager.processKey(tabBarEditors.currentIndex, event.key, event.modifiers, textArea);
                    }
                }
            MouseArea {
                anchors.fill: parent
                onPressed: (mouse)=> {
                    effectManager.codeHelper.setCodeCompletionVisible(false);
                    mouse.accepted = false;
                }
            }
        }
    }

    ListView {
        id: codeCompletionListView
        // Position of cursor inside flickable
        readonly property point cursorPos: Qt.point(textArea.cursorRectangle.x - codeFlickable.contentX,
                                              textArea.cursorRectangle.y - codeFlickable.contentY)
        // Position of popup, to keep it visible
        readonly property real popupPosX: Math.min(cursorPos.x, rootItem.width - width);
        readonly property real popupPosY: cursorPos.y + rowHeight
        readonly property int maxItems: 10
        // Show 1 .. maxItems elements in list, depending how much space there is below cursor
        readonly property int showItems: Math.max(1, Math.min(maxItems, Math.floor((codeFlickable.height - cursorPos.y - 10) / rowHeight)))

        width: 260
        height: Math.min(rowHeight * showItems, rowHeight * count)
        x: popupPosX
        y: popupPosY
        model: effectManager.codeHelper.codeCompletionModel
        visible: effectManager.codeHelper.codeCompletionVisible
        currentIndex: effectManager.codeHelper.codeCompletionModel.currentIndex
        clip: true
        delegate: Item {
            width: codeCompletionListView.width
            height: rowHeight
            Rectangle {
                anchors.fill: parent
                color: mainView.highlightColor
                opacity: 0.2
                visible: codeCompletionListView.currentIndex === model.index
            }
            Rectangle {
                height: parent.height / 2
                width: height
                anchors.left: parent.left
                anchors.leftMargin: height / 2
                anchors.verticalCenter: parent.verticalCenter
                border.width: 1
                border.color: mainView.foregroundColor1
                color: {
                    let typeColor = "#ffffff";
                    if (model.type === 0)
                        typeColor = "#ff0000";
                    else if (model.type === 1)
                        typeColor  = "#00ff00";
                    else if (model.type === 2)
                        typeColor = "#0000ff";
                    return typeColor;
                }
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: rowHeight
                anchors.right: parent.right
                anchors.rightMargin: 10
                elide: Text.ElideRight
                text: model.name
                font: textArea.font
                color: mainView.foregroundColor2
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    effectManager.codeHelper.codeCompletionModel.currentIndex = model.index;
                    effectManager.codeHelper.applyCodeCompletion(textArea);
                }
            }
        }
        Rectangle {
            anchors.fill: parent
            z: -1
            color: mainView.backgroundColor1
            border.width: 1
            border.color: mainView.foregroundColor1
        }
    }
}
