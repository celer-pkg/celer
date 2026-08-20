// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls

import Spreadsheets

HorizontalHeaderView {
    id: root

    property alias enableShowHideAction: showHideMenuItem.enabled
    required property SpreadSelectionModel spreadSelectionModel

    signal resetReorderingRequested()
    signal hideRequested(int column)
    signal showRequested()

    selectionBehavior: HorizontalHeaderView.SelectionDisabled
    movableColumns: true
    onColumnMoved: (index, old_column, new_column) => SpreadModel.mapColumn(index, new_column)

    selectionModel: HeaderSelectionModel {
        id: headerSelectionModel
        selectionModel: root.spreadSelectionModel
        orientation: Qt.Horizontal
    }

    textRole: "columnName"
    delegate: HorizontalHeaderViewDelegate {
        id: headerDelegate

        required property var index
        required property int column
        required property bool containsDrag
        readonly property bool visibleBorder: ((columnMenu.column === column)
                                               || containsDrag)

        function rightClicked() {
            columnMenu.column = index
            const menu_pos = mapToItem(root, -anchors.margins, height + anchors.margins)
            columnMenu.popup(menu_pos)
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
                root.spreadSelectionModel.toggleColumn(headerDelegate.index)
                headerSelectionModel.setCurrent()
            }
            onSelectRequested: {
                root.spreadSelectionModel.selectColumn(headerDelegate.index)
                headerSelectionModel.setCurrent()
            }
            onContextMenuRequested: headerDelegate.rightClicked()
        }
    }

    Menu {
        id: columnMenu

        property int column: -1

        onOpened: {
            headerSelectionModel.setCurrent(column)
        }

        onClosed: {
            headerSelectionModel.setCurrent()
            column = -1
        }

        MenuItem {
            text: qsTr("Insert 1 column left")
            icon {
                source: "icons/insert_column_left.svg"
                color: palette.highlightedText
            }

            onClicked: {
                if (columnMenu.column < 0)
                    return
                SpreadModel.insertColumn(columnMenu.column)
            }
        }

        MenuItem {
            text: qsTr("Insert 1 column right")
            icon {
                source: "icons/insert_column_right.svg"
                color: palette.highlightedText
            }

            onClicked: {
                if (columnMenu.column < 0)
                    return
                SpreadModel.insertColumn(columnMenu.column + 1)
            }
        }

        MenuItem {
            text: root.selectionModel.hasSelection ? qsTr("Remove selected columns")
                                              : qsTr("Remove column")
            icon {
                source: "icons/remove_column.svg"
                color: palette.text
            }

            onClicked: {
                if (root.selectionModel.hasSelection)
                    SpreadModel.removeColumns(root.spreadSelectionModel.selectedColumns())
                else if (columnMenu.column >= 0)
                    SpreadModel.removeColumn(columnMenu.column)
            }
        }

        MenuItem {
            text: root.selectionModel.hasSelection ? qsTr("Hide selected columns")
                                              : qsTr("Hide column")
            icon {
                source: "icons/hide.svg"
                color: palette.text
            }

            onClicked: {
                if (root.selectionModel.hasSelection) {
                    let columns = root.spreadSelectionModel.selectedColumns()
                    columns.sort(function(lhs, rhs){ return rhs.column - lhs.column })
                    for (let i in columns)
                        root.hideRequested(columns[i].column)
                    root.spreadSelectionModel.clearSelection()
                } else {
                    root.hideRequested(columnMenu.column)
                }
            }
        }

        MenuItem {
            id: showHideMenuItem
            text: qsTr("Show hidden column(s)")
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
            text: qsTr("Reset column reordering")
            icon {
                source: "icons/reset_reordering.svg"
                color: palette.text
            }

            onClicked: root.resetReorderingRequested()
        }
    }
}
