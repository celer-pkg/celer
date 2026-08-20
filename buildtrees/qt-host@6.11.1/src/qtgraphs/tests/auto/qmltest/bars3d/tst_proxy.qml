// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    Bar3DSeries {
        dataProxy: ItemModelBarDataProxy {
            id: initial
        }
    }

    Bar3DSeries {
        columnLabels: ["col1", "col2"]
        rowLabels: ["row1", "row2"]
        dataProxy: ItemModelBarDataProxy {
            id: initialized

            autoColumnCategories: false
            autoRowCategories: false
            columnCategories: ["colcat1", "colcat2"]
            columnRole: "col"
            columnRolePattern: /^.*-(\d\d)$/
            columnRoleReplace: "\\1"
            itemModel: ListModel { objectName: "model1" }
            multiMatchBehavior: ItemModelBarDataProxy.MultiMatchBehavior.Average
            rotationRole: "rot"
            rotationRolePattern: /-/
            rotationRoleReplace: "\\1"
            rowCategories: ["rowcat1", "rowcat2"]
            rowRole: "row"
            rowRolePattern: /^(\d\d\d\d).*$/
            rowRoleReplace: "\\1"
            valueRole: "val"
            valueRolePattern: /-/
            valueRoleReplace: "\\1"
        }
    }

    Bar3DSeries {
        dataProxy: ItemModelBarDataProxy {
            id: change
        }
    }

    TestCase {
        name: "ItemModelBarDataProxy Initial"

        function test_initial() {
            compare(initial.autoColumnCategories, true)
            compare(initial.autoRowCategories, true)
            compare(initial.columnCategories, [])
            compare(initial.columnRole, "")
            verify(initial.columnRolePattern)
            compare(initial.columnRoleReplace, "")
            verify(!initial.itemModel)
            compare(initial.multiMatchBehavior, ItemModelBarDataProxy.MultiMatchBehavior.Last)
            compare(initial.rotationRole, "")
            verify(initial.rotationRolePattern)
            compare(initial.rotationRoleReplace, "")
            compare(initial.rowCategories, [])
            compare(initial.rowRole, "")
            verify(initial.rowRolePattern)
            compare(initial.rowRoleReplace, "")
            compare(initial.useModelCategories, false)
            compare(initial.valueRole, "")
            verify(initial.valueRolePattern)
            compare(initial.valueRoleReplace, "")

            compare(initial.series.columnLabels.length, 0)
            compare(initial.rowCount, 0)
            compare(initial.series.rowLabels.length, 0)
            verify(initial.series)

            compare(initial.type, AbstractDataProxy.DataType.Bar)
        }
    }

    TestCase {
        name: "ItemModelBarDataProxy Initialized"

        function test_initialized() {
            compare(initialized.autoColumnCategories, false)
            compare(initialized.autoRowCategories, false)
            compare(initialized.columnCategories.length, 2)
            compare(initialized.columnCategories[0], "colcat1")
            compare(initialized.columnCategories[1], "colcat2")
            compare(initialized.columnRole, "col")
            compare(initialized.columnRolePattern, /^.*-(\d\d)$/)
            compare(initialized.columnRoleReplace, "\\1")
            compare(initialized.itemModel.objectName, "model1")
            compare(initialized.multiMatchBehavior, ItemModelBarDataProxy.MultiMatchBehavior.Average)
            compare(initialized.rotationRole, "rot")
            compare(initialized.rotationRolePattern, /-/)
            compare(initialized.rotationRoleReplace, "\\1")
            compare(initialized.rowCategories.length, 2)
            compare(initialized.rowCategories[0], "rowcat1")
            compare(initialized.rowCategories[1], "rowcat2")
            compare(initialized.rowRole, "row")
            compare(initialized.rowRolePattern, /^(\d\d\d\d).*$/)
            compare(initialized.rowRoleReplace, "\\1")
            compare(initialized.valueRole, "val")
            compare(initialized.valueRolePattern, /-/)
            compare(initialized.valueRoleReplace, "\\1")

            compare(initialized.series.columnLabels.length, 2)
            compare(initialized.rowCount, 2)
            compare(initialized.series.rowLabels.length, 2)
        }
    }

    TestCase {
        name: "ItemModelBarDataProxy Change"

        ListModel { id: model1; objectName: "model1" }

        function test_1_change() {
            change.autoColumnCategories = false
            change.autoRowCategories = false
            change.columnCategories = ["colcat1", "colcat2"]
            change.columnRole = "col"
            change.columnRolePattern = /^.*-(\d\d)$/
            change.columnRoleReplace = "\\1"
            change.itemModel = model1
            change.multiMatchBehavior = ItemModelBarDataProxy.MultiMatchBehavior.Average
            change.rotationRole = "rot"
            change.rotationRolePattern = /-/
            change.rotationRoleReplace = "\\1"
            change.rowCategories = ["rowcat1", "rowcat2"]
            change.rowRole = "row"
            change.rowRolePattern = /^(\d\d\d\d).*$/
            change.rowRoleReplace = "\\1"
            change.useModelCategories = true // Overwrites columnLabels and rowLabels
            change.valueRole = "val"
            change.valueRolePattern = /-/
            change.valueRoleReplace = "\\1"

            change.series.columnLabels = ["col1", "col2"]
            change.series.rowLabels = ["row1", "row2"]
        }

        function test_2_test_change() {
            // This test has a dependency to the previous one due to asynchronous item model resolving
            compare(change.autoColumnCategories, false)
            compare(change.autoRowCategories, false)
            compare(change.columnCategories.length, 2)
            compare(change.columnCategories[0], "colcat1")
            compare(change.columnCategories[1], "colcat2")
            compare(change.columnRole, "col")
            compare(change.columnRolePattern, /^.*-(\d\d)$/)
            compare(change.columnRoleReplace, "\\1")
            compare(change.itemModel.objectName, "model1")
            compare(change.multiMatchBehavior, ItemModelBarDataProxy.MultiMatchBehavior.Average)
            compare(change.rotationRole, "rot")
            compare(change.rotationRolePattern, /-/)
            compare(change.rotationRoleReplace, "\\1")
            compare(change.rowCategories.length, 2)
            compare(change.rowCategories[0], "rowcat1")
            compare(change.rowCategories[1], "rowcat2")
            compare(change.rowRole, "row")
            compare(change.rowRolePattern, /^(\d\d\d\d).*$/)
            compare(change.rowRoleReplace, "\\1")
            compare(change.useModelCategories, true)
            compare(change.valueRole, "val")
            compare(change.valueRolePattern, /-/)
            compare(change.valueRoleReplace, "\\1")

            compare(change.series.columnLabels.length, 1)
            compare(change.rowCount, 0)
            compare(change.series.rowLabels.length, 0)

            // signals
            compare(autoColumnCategoriesSpy.count, 1)
            compare(autoRowCategoriesSpy.count, 1)
            compare(columnCategoriesSpy.count, 1)
            compare(rowCategoriesSpy.count, 1)
            compare(columnRoleSpy.count, 1)
            compare(columnPatternSpy.count, 1)
            compare(columnReplaceSpy.count, 1)
            compare(rowRoleSpy.count, 1)
            compare(rowPatternSpy.count, 1)
            compare(rowReplaceSpy.count, 1)
            compare(valueRoleSpy.count, 1)
            compare(valuePatternSpy.count, 1)
            compare(valueReplaceSpy.count, 1)
            compare(rotationRoleSpy.count, 1)
            compare(rotationPatternSpy.count, 1)
            compare(rotationReplaceSpy.count, 1)
            compare(itemModelSpy.count, 1)
            compare(multiMatchSpy.count, 1)
        }

        function test_3_test_categoryIndex() {
            let rowIndex = change.rowCategoryIndex("rowcat1")
            let columnIndex = change.columnCategoryIndex("colcat2")
            compare(rowIndex, 0)
            compare(columnIndex, 1)

            rowIndex = change.rowCategoryIndex("rowcat2")
            columnIndex = change.columnCategoryIndex("colcat1")
            compare(rowIndex, 1)
            compare(columnIndex, 0)
        }
    }

    TestCase {
        name: "ItemModelBarDataProxy MultiMatchBehaviour"

        Bars3D {
            id: bars1

            Bar3DSeries {
                ItemModelBarDataProxy {
                    id: barProxy
                    itemModel: ListModel {
                        ListElement{ coords: "0,0"; data: "5"; }
                        ListElement{ coords: "0,0"; data: "15"; }
                    }
                    rowRole: "coords"
                    columnRole: "coords"
                    valueRole: "data"
                    rowRolePattern: /(\d),\d/
                    columnRolePattern: /(\d),(\d)/
                    rowRoleReplace: "\\1"
                    columnRoleReplace: "\\2"
                }
            }
        }

        function test_0_async_dummy() {
        }

        function test_1_test_multimatch() {
            compare(bars1.valueAxis.max, 15)
        }

        function test_2_multimatch() {
            barProxy.multiMatchBehavior = ItemModelBarDataProxy.MultiMatchBehavior.First
        }

        function test_3_test_multimatch() {
            compare(bars1.valueAxis.max, 5)
        }

        function test_4_multimatch() {
            barProxy.multiMatchBehavior = ItemModelBarDataProxy.MultiMatchBehavior.Last
        }

        function test_5_test_multimatch() {
            compare(bars1.valueAxis.max, 15)
        }

        function test_6_multimatch() {
            barProxy.multiMatchBehavior = ItemModelBarDataProxy.MultiMatchBehavior.Average
        }

        function test_7_test_multimatch() {
            compare(bars1.valueAxis.max, 10)
        }

        function test_8_multimatch() {
            barProxy.multiMatchBehavior = ItemModelBarDataProxy.MultiMatchBehavior.Cumulative
        }

        function test_9_test_multimatch() {
            compare(bars1.valueAxis.max, 20)
        }
    }

    SignalSpy {
        id: autoColumnCategoriesSpy
        target: change
        signalName: "autoColumnCategoriesChanged"
    }

    SignalSpy {
        id: autoRowCategoriesSpy
        target: change
        signalName: "autoRowCategoriesChanged"
    }

    SignalSpy {
        id: columnCategoriesSpy
        target: change
        signalName: "columnCategoriesChanged"
    }

    SignalSpy {
        id: rowCategoriesSpy
        target: change
        signalName: "rowCategoriesChanged"
    }

    SignalSpy {
        id: itemModelSpy
        target: change
        signalName: "itemModelChanged"
    }

    SignalSpy {
        id: rowRoleSpy
        target: change
        signalName: "rowRoleChanged"
    }

    SignalSpy {
        id: columnRoleSpy
        target: change
        signalName: "columnRoleChanged"
    }

    SignalSpy {
        id: valueRoleSpy
        target: change
        signalName: "valueRoleChanged"
    }

    SignalSpy {
        id: rotationRoleSpy
        target: change
        signalName: "rotationRoleChanged"
    }

    SignalSpy {
        id: useModelCategoriesSpy
        target: change
        signalName: "useModelCategoriesChanged"
    }

    SignalSpy {
        id: rowPatternSpy
        target: change
        signalName: "rowRolePatternChanged"
    }

    SignalSpy {
        id: columnPatternSpy
        target: change
        signalName: "columnRolePatternChanged"
    }

    SignalSpy {
        id: valuePatternSpy
        target: change
        signalName: "valueRolePatternChanged"
    }

    SignalSpy {
        id: rotationPatternSpy
        target: change
        signalName: "rotationRolePatternChanged"
    }

    SignalSpy {
        id: rowReplaceSpy
        target: change
        signalName: "rowRoleReplaceChanged"
    }

    SignalSpy {
        id: columnReplaceSpy
        target: change
        signalName: "columnRoleReplaceChanged"
    }

    SignalSpy {
        id: valueReplaceSpy
        target: change
        signalName: "valueRoleReplaceChanged"
    }

    SignalSpy {
        id: rotationReplaceSpy
        target: change
        signalName: "rotationRoleReplaceChanged"
    }

    SignalSpy {
        id: multiMatchSpy
        target: change
        signalName: "multiMatchBehaviorChanged"
    }
}
