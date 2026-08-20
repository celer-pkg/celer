// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//![file]
import QtQuick
import QtQuick.Controls
import Qt.labs.qmlmodels

ApplicationWindow {
    visible: true
    width: 500
    height: 500

    TreeView {
        id: treeView
        anchors.fill: parent

        selectionModel: ItemSelectionModel {}

        model: TreeModel {
            id: treeModel

            TableModelColumn {
                display: "checked"
            }
            TableModelColumn {
                display: "size"
            }
            TableModelColumn {
                display: "type"
            }
            TableModelColumn {
                display: "name"
            }
            TableModelColumn {
                display: "lastModified"
            }

            rows: [{
                    checked: false,
                    size: "—",
                    type: "folder",
                    name: "Documents",
                    lastModified: "2025-07-01",
                    rows: [{
                            checked: true,
                            size: "24 KB",
                            type: "file",
                            name: "Resume.pdf",
                            lastModified: "2025-06-20",
                        }, {
                            checked: false,
                            size: "2 MB",
                            type: "folder",
                            name: "Reports",
                            lastModified: "2025-06-10",
                            rows: [{
                                    checked: true,
                                    size: "850 KB",
                                    type: "file",
                                    name: "Q2_Report.docx",
                                    lastModified: "2025-06-15",
                                }, {
                                    checked: false,
                                    size: "1.2 MB",
                                    type: "file",
                                    name: "Q3_Plan.xlsx",
                                    lastModified: "2025-06-18",
                                }]
                        }]
                },
//![rows]
                {
                    checked: false,
                    size: "—",
                    type: "folder",
                    name: "Pictures",
                    lastModified: "2025-05-30",
                    rows: [{
                            checked: true,
                            size: "3.5 MB",
                            type: "file",
                            name: "Vacation.jpg",
                            lastModified: "2025-05-15",
                        }, {
                            checked: false,
                            size: "2.1 MB",
                            type: "file",
                            name: "Family.png",
                            lastModified: "2025-05-20",
                        }]
                }
//![rows]
            ]
        }

        delegate: TreeViewDelegate {}
    }
}
//![file]
