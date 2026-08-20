// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest

Rectangle {
    width: 320
    height: 240
    color: "#111"

    TextEdit {
        id: te
        wrapMode: TextEdit.WordWrap
        anchors.fill: parent
        color: "white"

        TextSelection {
            id: sel1
        }

        Component {
            id: selectionFactory
            TextSelection { }
        }
    }

    TestCase {
        name: "TextSelection"
        when: windowShown

        function init() {
            te.textDocument.modified = false // so that setting source will work
            te.textDocument.source = "planets.txt"
        }

        function test_selectDuplicateAndReplaceWords() {
            var sel = te.cursorSelection
            // Select the whole line and duplicate it
            sel.selectionStart = 0
            sel.moveSelectionEnd(TextSelection.NextBlock)

            sel.duplicate()

            // The visual selection is now the duplicated block.
            // Position the other selection instance at the beginning of the duplicate,
            // then select a word forward from there.
            let start = te.cursorSelection.selectionStart
            sel1.selectionStart = start
            sel1.selectionEnd = sel1.selectionStart
            sel1.moveSelectionEnd(TextSelection.EndOfWord)

            // Workaround: apostrophe is treated as a word delimiter apparently:
            // we have selected "Michael", and we need to get to the end of "Michael's"
            sel1.moveSelectionEnd(TextSelection.NextWord)
            sel1.moveSelectionEnd(TextSelection.EndOfWord)

            // Replace the words in the duplicate line, and set their colors
            let planets = [["Mercury", "orange"], ["Venus", "lavender"], ["Earth", "lightskyblue"],
                           ["Mars", "indianred"], ["Jupiter", "pink"], ["Saturn", "papayawhip"],
                           ["Uranus", "lightsteelblue"], ["Neptune", "dodgerblue"], ["Pluto", "burlywood"]]
            planets.forEach((planet) => {
                                sel1.text = planet[0]
                                sel1.color = planet[1]
                                sel1.moveSelectionStart(TextSelection.NextWord)
                                sel1.moveSelectionEnd(TextSelection.EndOfWord)
                            });

            // When we replaced the first word, selectionStart got changed.
            // Re-select the whole duplicate line and verify that all the words got changed.
            sel.selectionStart = start
            sel.moveSelectionEnd(TextSelection.EndOfBlock)
            compare(sel.text, "Mercury Venus Earth Mars Jupiter Saturn Uranus Neptune Pluto.")

            // Use the secondary selection to verify that Mercury is orange
            sel1.selectionStart = start
            sel1.selectionEnd = start
            sel1.moveSelectionEnd(TextSelection.EndOfWord)
            compare(sel1.text, "Mercury")
            compare(sel1.color, "#ffa500")
        }

        function test_duplicateMultipleBlocks() {
            te.textDocument.source = "quotes.md"
            te.selectAll()
            var sel = te.cursorSelection
            compare(sel.selectionStart, 0)
            compare(sel.selectionEnd, 221)
            sel.duplicate()
            compare(sel.selectionStart, 221)
            compare(sel.selectionEnd, 442)
        }

        function test_fontAttributeIndependence_data() {
            return [
                {tag: "declared selection", sel: sel1 },
                {tag: "cursorSelection", sel: te.cursorSelection },
            ]
        }

        function test_fontAttributeIndependence(data) {
            let sel = data.sel

            // Italicize the first word
            sel.selectionStart = 0
            sel.moveSelectionEnd(TextSelection.EndOfWord)
            let pxSize = sel.font.pixelSize
            let ptSize = sel.font.pointSize
            let fam = sel.font.family
            sel.font.italic = true

            // Verify that other font attributes don't change
            compare(sel.font.pixelSize, pxSize)
            compare(sel.font.pointSize, ptSize)
            compare(sel.font.family, fam)

            // Embiggen the first word
            sel.font.pointSize = 24

            // Verify that italic isn't lost
            compare(sel.font.italic, true)

            // Make it bold
            sel.font.bold = true

            // Verify that italic and point size are not lost
            compare(sel.font.italic, true)
            compare(sel.font.pointSize, 24)
        }

        function test_makeHyperlink() {
            var sel = te.cursorSelection
            let url = "https://en.wikipedia.org/wiki/Mercury_(planet)"

            // Link the first planet to its wikipedia article
            sel.selectionStart = 0
            sel.moveSelectionEnd(TextSelection.EndOfWord)
            let pxSize = sel.font.pixelSize
            let ptSize = sel.font.pointSize
            let fam = sel.font.family
            sel.linkTo(url)
            compare(te.linkAt(5, 5), url)

            // Verify that other attributes don't change
            compare(sel.font.pixelSize, pxSize)
            compare(sel.font.pointSize, ptSize)
            compare(sel.font.family, fam)

            // Change style of the first word, and verify that link isn't lost
            sel.font.bold = true
            compare(te.linkAt(5, 5), url)
        }

        function test_componentCreateObject() {
            // We must give the parent to createObject() because it can't be set afterwards
            let sel = selectionFactory.createObject(te)
            compare(sel.document, te.textDocument)

            // Italicize the first word
            sel.selectionStart = 0
            sel.selectionEnd = 9
            sel.font.italic = true

            // Right-align the block
            sel.alignment = Qt.AlignRight

            // Verify that it's still italic
            compare(sel.font.italic, true)

            // Move the cursor and verify block alignment, also via cursorSelection
            sel.selectionStart = 15
            sel.selectionEnd = 15
            compare(sel.alignment, Qt.AlignRight)
            te.cursorSelection.selectionStart = 20
            compare(te.cursorSelection.alignment, Qt.AlignRight)
        }
    }
}
