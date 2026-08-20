// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

   Scatter3DSeries {
       dataProxy: ItemModelScatterDataProxy {
           id: initial
       }
   }

    Scatter3DSeries {
        dataProxy: ItemModelScatterDataProxy {
            id: initialized

            itemModel: ListModel { objectName: "model1" }
            rotationRole: "rot"
            rotationRolePattern: /-/
            rotationRoleReplace: "\\1"
            scaleRole: "scale"
            scaleRolePattern: /-/
            scaleRoleReplace: "\\1"
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

    Scatter3DSeries {
        dataProxy: ItemModelScatterDataProxy {
            id: change
        }
    }

    TestCase {
        name: "ItemModelScatterDataProxy Initial"

        function test_initial() {
            verify(!initial.itemModel)
            compare(initial.rotationRole, "")
            verify(initial.rotationRolePattern)
            compare(initial.rotationRoleReplace, "")
            compare(initial.scaleRole, "")
            verify(initial.scaleRolePattern)
            compare(initial.scaleRoleReplace, "")
            compare(initial.xPosRole, "")
            verify(initial.xPosRolePattern)
            compare(initial.xPosRoleReplace, "")
            compare(initial.yPosRole, "")
            verify(initial.yPosRolePattern)
            compare(initial.yPosRoleReplace, "")
            compare(initial.zPosRole, "")
            verify(initial.zPosRolePattern)
            compare(initial.zPosRoleReplace, "")

            compare(initial.itemCount, 0)
            verify(initial.series)

            compare(initial.type, AbstractDataProxy.DataType.Scatter)
        }
    }

    TestCase {
        name: "ItemModelScatterDataProxy Initialized"

        function test_initialized() {
            compare(initialized.itemModel.objectName, "model1")
            compare(initialized.rotationRole, "rot")
            compare(initialized.rotationRolePattern, /-/)
            compare(initialized.rotationRoleReplace, "\\1")
            compare(initialized.scaleRole, "scale")
            compare(initialized.scaleRolePattern, /-/)
            compare(initialized.scaleRoleReplace, "\\1")
            compare(initialized.xPosRole, "x")
            compare(initialized.xPosRolePattern, /^.*-(\d\d)$/)
            compare(initialized.xPosRoleReplace, "\\1")
            compare(initialized.yPosRole, "y")
            compare(initialized.yPosRolePattern, /^(\d\d\d\d).*$/)
            compare(initialized.yPosRoleReplace, "\\1")
            compare(initialized.zPosRole, "z")
            compare(initialized.zPosRolePattern, /-/)
            compare(initialized.zPosRoleReplace, "\\1")
        }
    }

    TestCase {
        name: "ItemModelScatterDataProxy Change"

        ListModel { id: model1; objectName: "model1" }

        function test_change() {
            change.itemModel = model1
            change.rotationRole = "rot"
            change.rotationRolePattern = /-/
            change.rotationRoleReplace = "\\1"
            change.scaleRole = "scale"
            change.scaleRolePattern = /-/
            change.scaleRoleReplace = "\\1"
            change.xPosRole = "x"
            change.xPosRolePattern = /^.*-(\d\d)$/
            change.xPosRoleReplace = "\\1"
            change.yPosRole = "y"
            change.yPosRolePattern = /^(\d\d\d\d).*$/
            change.yPosRoleReplace = "\\1"
            change.zPosRole = "z"
            change.zPosRolePattern = /-/
            change.zPosRoleReplace = "\\1"

            compare(change.itemModel.objectName, "model1")
            compare(change.rotationRole, "rot")
            compare(change.rotationRolePattern, /-/)
            compare(change.rotationRoleReplace, "\\1")
            compare(change.scaleRole, "scale")
            compare(change.scaleRolePattern, /-/)
            compare(change.scaleRoleReplace, "\\1")
            compare(change.xPosRole, "x")
            compare(change.xPosRolePattern, /^.*-(\d\d)$/)
            compare(change.xPosRoleReplace, "\\1")
            compare(change.yPosRole, "y")
            compare(change.yPosRolePattern, /^(\d\d\d\d).*$/)
            compare(change.yPosRoleReplace, "\\1")
            compare(change.zPosRole, "z")
            compare(change.zPosRolePattern, /-/)
            compare(change.zPosRoleReplace, "\\1")

            // Signals
            compare(itemModelSpy.count, 1)
            compare(rotationRoleSpy.count, 1)
            compare(rotationPatternSpy.count, 1)
            compare(rotationReplaceSpy.count, 1)
            compare(scaleRoleSpy.count, 1)
            compare(scalePatternSpy.count, 1)
            compare(scaleReplaceSpy.count, 1)
            compare(xPosRoleSpy.count, 1)
            compare(xPosPatternSpy.count, 1)
            compare(xPosReplaceSpy.count, 1)
            compare(yPosRoleSpy.count, 1)
            compare(yPosPatternSpy.count, 1)
            compare(yPosReplaceSpy.count, 1)
            compare(zPosRoleSpy.count, 1)
            compare(zPosPatternSpy.count, 1)
            compare(zPosReplaceSpy.count, 1)
        }
    }

    SignalSpy {
        id: itemModelSpy
        target: change
        signalName: "itemModelChanged"
    }

    SignalSpy {
        id: rotationRoleSpy
        target: change
        signalName: "rotationRoleChanged"
    }

    SignalSpy {
        id: rotationPatternSpy
        target: change
        signalName: "rotationRolePatternChanged"
    }

    SignalSpy {
        id: rotationReplaceSpy
        target: change
        signalName: "rotationRoleReplaceChanged"
    }

    SignalSpy {
        id: scaleRoleSpy
        target: change
        signalName: "scaleRoleChanged"
    }

    SignalSpy {
        id: scalePatternSpy
        target: change
        signalName: "scaleRolePatternChanged"
    }

    SignalSpy {
        id: scaleReplaceSpy
        target: change
        signalName: "scaleRoleReplaceChanged"
    }

    SignalSpy {
        id: xPosRoleSpy
        target: change
        signalName: "xPosRoleChanged"
    }

    SignalSpy {
        id: xPosPatternSpy
        target: change
        signalName: "xPosRolePatternChanged"
    }

    SignalSpy {
        id: xPosReplaceSpy
        target: change
        signalName: "xPosRoleReplaceChanged"
    }

    SignalSpy {
        id: yPosRoleSpy
        target: change
        signalName: "yPosRoleChanged"
    }

    SignalSpy {
        id: yPosPatternSpy
        target: change
        signalName: "yPosRolePatternChanged"
    }

    SignalSpy {
        id: yPosReplaceSpy
        target: change
        signalName: "yPosRoleReplaceChanged"
    }

    SignalSpy {
        id: zPosRoleSpy
        target: change
        signalName: "zPosRoleChanged"
    }

    SignalSpy {
        id: zPosPatternSpy
        target: change
        signalName: "zPosRolePatternChanged"
    }

    SignalSpy {
        id: zPosReplaceSpy
        target: change
        signalName: "zPosRoleReplaceChanged"
    }
}
