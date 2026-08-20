// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    Surface3DSeries {
        dataProxy: ItemModelSurfaceDataProxy {
            id: initial
        }
    }

    Surface3DSeries {
        dataProxy: ItemModelSurfaceDataProxy {
            id: initialized

            autoColumnCategories: false
            autoRowCategories: false
            columnCategories: ["colcat1", "colcat2"]
            columnRole: "col"
            columnRolePattern: /^.*-(\d\d)$/
            columnRoleReplace: "\\1"
            itemModel: ListModel { objectName: "model1" }
            multiMatchBehavior: ItemModelSurfaceDataProxy.MultiMatchBehavior.Average
            rowCategories: ["rowcat1", "rowcat2"]
            rowRole: "row"
            rowRolePattern: /^(\d\d\d\d).*$/
            rowRoleReplace: "\\1"
            xPosRole: "x"
            xPosRolePattern: /^.*-(\d\d)$/
            xPosRoleReplace: "\\1"
            yPosRole: "y"
            yPosRolePattern: /^(\d\d\d\d).*$/
            yPosRoleReplace: "\\1"
            zPosRole: "z"
            zPosRolePattern: /-/
            zPosRoleReplace: "\\1"
        }
    }

    Surface3DSeries {
        dataProxy: ItemModelSurfaceDataProxy {
            id: rowcolumnreplace

            autoColumnCategories: true
            autoRowCategories: true
            itemModel: ListModel {
                ListElement{ rowcol: "1st,AA"; data: "0/1/2"; }
                ListElement{ rowcol: "2nd,BB"; data: "1/2/3"; }
                ListElement{ rowcol: "3rd,CC"; data: "2/3/4"; }
                ListElement{ rowcol: "4th,DD"; data: "3/4/5"; }
            }
            columnRole: "rowcol"
            columnRolePattern: /(\d)\w+,(\w+)/
            columnRoleReplace: "\\2"
            rowRole: "rowcol"
            rowRolePattern: /(\d\w+),(\w+)/
            rowRoleReplace: "\\1"
            xPosRole: "data"
            xPosRolePattern: /^([^\/]*)\/([^\/]*)\/(.*)$/
            xPosRoleReplace: "\\1"
            yPosRole: "data"
            yPosRolePattern: /^([^\/]*)\/([^\/]*)\/(.*)$/
            yPosRoleReplace: "\\2"
            zPosRole: "data"
            zPosRolePattern: /^([^\/]*)\/([^\/]*)\/(.*)$/
            zPosRoleReplace: "\\3"
        }
    }

    Surface3DSeries {
        dataProxy: ItemModelSurfaceDataProxy {
            id: change
        }
    }

    TestCase {
        name: "ItemModelSurfaceDataProxy Initial"

        function test_initial() {
            compare(initial.autoColumnCategories, true)
            compare(initial.autoRowCategories, true)
            compare(initial.columnCategories, [])
            compare(initial.columnRole, "")
            verify(initial.columnRolePattern)
            compare(initial.columnRoleReplace, "")
            verify(!initial.itemModel)
            compare(initial.multiMatchBehavior, ItemModelSurfaceDataProxy.MultiMatchBehavior.Last)
            compare(initial.rowCategories, [])
            compare(initial.rowRole, "")
            verify(initial.rowRolePattern)
            compare(initial.rowRoleReplace, "")
            compare(initial.useModelCategories, false)
            compare(initial.xPosRole, "")
            verify(initial.xPosRolePattern)
            compare(initial.xPosRoleReplace, "")
            compare(initial.yPosRole, "")
            verify(initial.yPosRolePattern)
            compare(initial.yPosRoleReplace, "")
            compare(initial.zPosRole, "")
            verify(initial.zPosRolePattern)
            compare(initial.zPosRoleReplace, "")

            compare(initial.columnCount, 0)
            compare(initial.rowCount, 0)
            verify(initial.series)

            compare(initial.type, AbstractDataProxy.DataType.Surface)
        }
    }

    TestCase {
        name: "ItemModelSurfaceDataProxy Initialized"

        function test_initialized() {
            verify(initialized.series)
            compare(initialized.autoColumnCategories, false)
            compare(initialized.autoRowCategories, false)
            compare(initialized.columnCategories.length, 2)
            compare(initialized.columnCategories[0], "colcat1")
            compare(initialized.columnCategories[1], "colcat2")
            compare(initialized.columnRole, "col")
            compare(initialized.columnRolePattern, /^.*-(\d\d)$/)
            compare(initialized.columnRoleReplace, "\\1")
            compare(initialized.itemModel.objectName, "model1")
            compare(initialized.multiMatchBehavior, ItemModelSurfaceDataProxy.MultiMatchBehavior.Average)
            compare(initialized.rowCategories.length, 2)
            compare(initialized.rowCategories[0], "rowcat1")
            compare(initialized.rowCategories[1], "rowcat2")
            compare(initialized.rowRole, "row")
            compare(initialized.rowRolePattern, /^(\d\d\d\d).*$/)
            compare(initialized.rowRoleReplace, "\\1")
            compare(initialized.xPosRole, "x")
            compare(initialized.xPosRolePattern, /^.*-(\d\d)$/)
            compare(initialized.xPosRoleReplace, "\\1")
            compare(initialized.yPosRole, "y")
            compare(initialized.yPosRolePattern, /^(\d\d\d\d).*$/)
            compare(initialized.yPosRoleReplace, "\\1")
            compare(initialized.zPosRole, "z")
            compare(initialized.zPosRolePattern, /-/)
            compare(initialized.zPosRoleReplace, "\\1")

            compare(initialized.columnCount, 2)
            compare(initialized.rowCount, 2)
        }
    }

    TestCase {
        name: "ItemModelSurfaceDataProxy RoleReplace"

        function test_initialized() {
            verify(rowcolumnreplace.series)
            compare(rowcolumnreplace.columnCategories.length, 4)
            compare(rowcolumnreplace.columnCategories[0], "AA")
            compare(rowcolumnreplace.columnCategories[1], "BB")
            compare(rowcolumnreplace.columnCategories[2], "CC")
            compare(rowcolumnreplace.columnCategories[3], "DD")
            compare(rowcolumnreplace.rowCategories.length, 4)
            compare(rowcolumnreplace.rowCategories[0], "1st")
            compare(rowcolumnreplace.rowCategories[1], "2nd")
            compare(rowcolumnreplace.rowCategories[2], "3rd")
            compare(rowcolumnreplace.rowCategories[3], "4th")

            // TODO: Do we have a way to check what the x, y, and z values are after the replace (in QML)?
            compare(rowcolumnreplace.xPosRole, "data")
            compare(rowcolumnreplace.xPosRoleReplace, "\\1")
            compare(rowcolumnreplace.yPosRole, "data")
            compare(rowcolumnreplace.yPosRoleReplace, "\\2")
            compare(rowcolumnreplace.zPosRole, "data")
            compare(rowcolumnreplace.zPosRoleReplace, "\\3")
        }
    }

    TestCase {
        name: "ItemModelSurfaceDataProxy Change"

        ListModel { id: model1; objectName: "model1" }

        function test_1_change() {
            change.autoColumnCategories = false
            change.autoRowCategories = false
            change.columnCategories = ["colcat1", "colcat2"]
            change.columnRole = "col"
            change.columnRolePattern = /^.*-(\d\d)$/
            change.columnRoleReplace = "\\1"
            change.itemModel = model1
            change.multiMatchBehavior = ItemModelSurfaceDataProxy.MultiMatchBehavior.Average
            change.rowCategories = ["rowcat1", "rowcat2"]
            change.rowRole = "row"
            change.rowRolePattern = /^(\d\d\d\d).*$/
            change.rowRoleReplace = "\\1"
            change.useModelCategories = true // Overwrites columnLabels and rowLabels
            change.xPosRole = "x"
            change.xPosRolePattern = /^.*-(\d\d)$/
            change.xPosRoleReplace = "\\1"
            change.yPosRole = "y"
            change.yPosRolePattern = /^(\d\d\d\d).*$/
            change.yPosRoleReplace = "\\1"
            change.zPosRole = "z"
            change.zPosRolePattern = /-/
            change.zPosRoleReplace = "\\1"
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
            compare(change.multiMatchBehavior, ItemModelSurfaceDataProxy.MultiMatchBehavior.Average)
            compare(change.rowCategories.length, 2)
            compare(change.rowCategories[0], "rowcat1")
            compare(change.rowCategories[1], "rowcat2")
            compare(change.rowRole, "row")
            compare(change.rowRolePattern, /^(\d\d\d\d).*$/)
            compare(change.rowRoleReplace, "\\1")
            compare(change.useModelCategories, true)
            compare(change.xPosRole, "x")
            compare(change.xPosRolePattern, /^.*-(\d\d)$/)
            compare(change.xPosRoleReplace, "\\1")
            compare(change.yPosRole, "y")
            compare(change.yPosRolePattern, /^(\d\d\d\d).*$/)
            compare(change.yPosRoleReplace, "\\1")
            compare(change.zPosRole, "z")
            compare(change.zPosRolePattern, /-/)
            compare(change.zPosRoleReplace, "\\1")

            compare(change.columnCount, 0)
            compare(change.rowCount, 0)

            // Signals
            compare(autoColumnCategoriesSpy.count, 1)
            compare(autoRowCategoriesSpy.count, 1)
            compare(columnCategoriesSpy.count, 1)
            compare(columnRoleSpy.count, 1)
            compare(columnPatternSpy.count, 1)
            compare(columnReplaceSpy.count, 1)
            compare(itemModelSpy.count, 1)
            compare(multiMatchSpy.count, 1)
            compare(rowCategoriesSpy.count, 1)
            compare(rowRoleSpy.count, 1)
            compare(rowPatternSpy.count, 1)
            compare(rowReplaceSpy.count, 1)
            compare(useModelCategoriesSpy.count, 1)
            compare(xPosSpy.count, 1)
            compare(xPosPatternSpy.count, 1)
            compare(xPosReplaceSpy.count, 1)
            compare(yPosSpy.count, 1)
            compare(yPosPatternSpy.count, 1)
            compare(yPosReplaceSpy.count, 1)
            compare(zPosSpy.count, 1)
            compare(zPosPatternSpy.count, 1)
            compare(zPosReplaceSpy.count, 1)
        }
    }

    TestCase {
        name: "ItemModelSurfaceDataProxy MultiMatchBehaviour"

        Surface3D {
            id: surface1
            Surface3DSeries {
                ItemModelSurfaceDataProxy {
                    id: surfaceProxy
                    itemModel: ListModel {
                        ListElement{ coords: "0,0"; data: "5"; }
                        ListElement{ coords: "0,0"; data: "15"; }
                        ListElement{ coords: "0,1"; data: "5"; }
                        ListElement{ coords: "0,1"; data: "15"; }
                        ListElement{ coords: "1,0"; data: "5"; }
                        ListElement{ coords: "1,0"; data: "15"; }
                        ListElement{ coords: "1,1"; data: "0"; }
                    }
                    rowRole: "coords"
                    columnRole: "coords"
                    yPosRole: "data"
                    rowRolePattern: /(\d),(\d)/
                    columnRolePattern: /(\d),(\d)/
                    rowRoleReplace: "\\1"
                    columnRoleReplace: "\\2"
                }
            }
        }

        function test_0_async_dummy() {
        }

        function test_1_test_multimatch() {
            compare(surface1.axisY.max, 15)
        }

        function test_2_multimatch() {
            surfaceProxy.multiMatchBehavior = ItemModelSurfaceDataProxy.MultiMatchBehavior.First
        }

        function test_3_test_multimatch() {
            compare(surface1.axisY.max, 5)
        }

        function test_4_multimatch() {
            surfaceProxy.multiMatchBehavior = ItemModelSurfaceDataProxy.MultiMatchBehavior.Last
        }

        function test_5_test_multimatch() {
            compare(surface1.axisY.max, 15)
        }

        function test_6_multimatch() {
            surfaceProxy.multiMatchBehavior = ItemModelSurfaceDataProxy.MultiMatchBehavior.Average
        }

        function test_7_test_multimatch() {
            compare(surface1.axisY.max, 10)
        }

        function test_8_multimatch() {
            surfaceProxy.multiMatchBehavior = ItemModelSurfaceDataProxy.MultiMatchBehavior.CumulativeY
        }

        function test_9_test_multimatch() {
            compare(surface1.axisY.max, 20)
        }
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
        id: xPosSpy
        target: change
        signalName: "xPosRoleChanged"
    }

    SignalSpy {
        id: yPosSpy
        target: change
        signalName: "yPosRoleChanged"
    }

    SignalSpy {
        id: zPosSpy
        target: change
        signalName: "zPosRoleChanged"
    }

    SignalSpy {
        id: rowCategoriesSpy
        target: change
        signalName: "rowCategoriesChanged"
    }

    SignalSpy {
        id: columnCategoriesSpy
        target: change
        signalName: "columnCategoriesChanged"
    }

    SignalSpy {
        id: useModelCategoriesSpy
        target: change
        signalName: "useModelCategoriesChanged"
    }

    SignalSpy {
        id: autoRowCategoriesSpy
        target: change
        signalName: "autoRowCategoriesChanged"
    }

    SignalSpy {
        id: autoColumnCategoriesSpy
        target: change
        signalName: "autoColumnCategoriesChanged"
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
        id: xPosPatternSpy
        target: change
        signalName: "xPosRolePatternChanged"
    }

    SignalSpy {
        id: yPosPatternSpy
        target: change
        signalName: "yPosRolePatternChanged"
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
        id: zPosPatternSpy
        target: change
        signalName: "zPosRolePatternChanged"
    }

    SignalSpy {
        id: xPosReplaceSpy
        target: change
        signalName: "xPosRoleReplaceChanged"
    }

    SignalSpy {
        id: yPosReplaceSpy
        target: change
        signalName: "yPosRoleReplaceChanged"
    }

    SignalSpy {
        id: zPosReplaceSpy
        target: change
        signalName: "zPosRoleReplaceChanged"
    }

    SignalSpy {
        id: multiMatchSpy
        target: change
        signalName: "multiMatchBehaviorChanged"
    }
}
