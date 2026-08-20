// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls

import Spreadsheets

VerticalHeaderView {
    id: root

    property alias enableShowHideAction: showHideMenuItem.enabled
    required property SpreadSelectionModel spreadSelectionModel

    signal resetReorderingRequested()
    signal hideRequested(int row)
    signal showRequested()

    selectionBehavior: VerticalHeaderView.SelectionDisabled
    movableRows: true
    onRowMoved: (index, old_row, new_row) => SpreadModel.mapRow(index, new_row)

    selectionModel: HeaderSelectionModel {
        id: headerSelectionModel
        selectionModel: root.spreadSelectionModel
        orientation: Qt.Vertical
    }

    textRole: "rowName"
    delegate: VerticalHeaderViewDelegate {
        id: headerDelegate

        required property bool containsDrag
        required property var index
        required property int row
        readonly property bool visibleBorder: ((rowMenu.row === row)
                                               || containsDrag)

        function rightClicked() {
            rowMenu.row = index
            const menu_pos = mapToItem(root, width + anchors.margins, -anchors.margins)
            rowMenu.popup(menu_pos)
        }

        Binding {
            target: headerDelegate.background
            property: "color"
            value: headerDelegate.palette.highlight
            when: headerDelegate.highlighted
        }

        Binding {
            target: headerDelegate.background
            property: "border.width"
            value: headerDelegate.visibleBorder ? 1 : 0
            when: (headerDelegate.background as Rectangle) ?? false
        }

        Binding {
            target: headerDelegate.background
            property: "border.color"
            value: headerDelegate.palette.highlight
            when: (headerDelegate.background as Rectangle) ?? false
        }

        HeaderViewTapHandler {
            anchors.fill: parent
            onToggleRequested: {
                root.spreadSelectionModel.toggleRow(headerDelegate.index)
                headerSelectionModel.setCurrent()
            }
            onSelectRequested: {
                root.spreadSelectionModel.selectRow(headerDelegate.index)
                headerSelectionModel.setCurrent()
            }
            onContextMenuRequested: headerDelegate.rightClicked()
        }
    }

    Menu {
        id: rowMenu

        property int row: -1

        onOpened: {
            headerSelectionModel.setCurrent(row)
        }

        onClosed: {
            headerSelectionModel.setCurrent()
            row = -1
        }

        MenuItem {
            text: qsTr("Insert 1 row above")
            icon {
                source: "icons/insert_row_above.svg"
                color: palette.highlightedText
            }

            onClicked: {
                if (rowMenu.row < 0)
                    return
                SpreadModel.insertRow(rowMenu.row)
            }
        }

        MenuItem {
            text: qsTr("Insert 1 row bellow")
            icon {
                source: "icons/insert_row_below.svg"
                color: palette.text
            }

            onClicked: {
                if (rowMenu.row < 0)
                    return
                SpreadModel.insertRow(rowMenu.row + 1)
            }
        }

        MenuItem {
            text: root.selectionModel.hasSelection ? qsTr("Remove selected rows")
                                              : qsTr("Remove row")
            icon {
                source: "icons/remove_row.svg"
                color: palette.text
            }

            onClicked: {
                if (root.selectionModel.hasSelection)
                    SpreadModel.removeRows(root.selectionModel.selectedRows())
                else if (rowMenu.row >= 0)
                    SpreadModel.removeRow(rowMenu.row)
            }
        }

        MenuItem {
            text: root.selectionModel.hasSelection ? qsTr("Hide selected rows")
                                              : qsTr("Hide row")
            icon {
                source: "icons/hide.svg"
                color: palette.text
            }

            onClicked: {
                if (root.selectionModel.hasSelection) {
                    let rows = root.selectionModel.selectedRows()
                    rows.sort(function(lhs, rhs){ return rhs.row - lhs.row })
                    for (let i in rows)
                        root.hideRequested(rows[i].row)
                    root.spreadSelectionModel.clearSelection()
                } else {
                    root.hideRequested(rowMenu.row)
                }
            }
        }

        MenuItem {
            id: showHideMenuItem
            text: qsTr("Show hidden row(s)")
            icon {
                source: "icons/show.svg"
                color: palette.text
            }

            onClicked: {
                root.showRequested()
                root.spreadSelectionModel.clearSelection()
            }
        }

        MenuItem {
            text: qsTr("Reset row reordering")
            icon {
                source: "icons/reset_reordering.svg"
                color: palette.text
            }

            onClicked: root.resetReorderingRequested()
        }
    }
}
