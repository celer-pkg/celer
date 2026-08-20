// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtGraphs
import QtTest 1.0

Item {
    id: top
    height: 150
    width: 150

    LogValue3DAxisFormatter {
        id: initial
    }

    LogValue3DAxisFormatter {
        id: initialized
        autoSubGrid: false
        base: 0.1
        edgeLabelsVisible: false
    }

    LogValue3DAxisFormatter {
        id: change
    }

    LogValue3DAxisFormatter {
        id: invalid
    }

    TestCase {
        name: "LogValue3DAxisFormatter Initial"

        function test_initial() {
            compare(initial.autoSubGrid, true)
            compare(initial.base, 10)
            compare(initial.edgeLabelsVisible, true)
        }
    }

    TestCase {
        name: "LogValue3DAxisFormatter Initialized"

        function test_initialized() {
            compare(initialized.autoSubGrid, false)
            compare(initialized.base, 0.1)
            compare(initialized.edgeLabelsVisible, false)
        }
    }

    TestCase {
        name: "LogValue3DAxisFormatter Change"

        function test_change() {
            change.autoSubGrid = false
            change.base = 0.1
            change.edgeLabelsVisible = false

            compare(change.autoSubGrid, false)
            compare(change.base, 0.1)
            compare(change.edgeLabelsVisible, false)

            //signals
            compare(baseSpy.count, 1)
            compare(autoSubGridSpy.count, 1)
            compare(edgeLabelVisibilitySpy.count, 1)
        }
    }

    TestCase {
        name: "LogValue3DAxisFormatter Invalid"

        function test_invalid() {
            invalid.base = 1
            compare(invalid.base, 10)
            invalid.base = -1
            compare(invalid.base, 10)
        }
    }

    SignalSpy {
        id: baseSpy
        target: change
        signalName: "baseChanged"
    }

    SignalSpy {
        id: autoSubGridSpy
        target: change
        signalName: "autoSubGridChanged"
    }

    SignalSpy {
        id: edgeLabelVisibilitySpy
        target: change
        signalName: "edgeLabelsVisibleChanged"
    }
}
