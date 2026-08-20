// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtGraphs

Bars3D {
    width: 300
    height: 300
    Bar3DSeries {
        ItemModelBarDataProxy {
            itemModel: ListModel {
                ListElement{ row: "row 1"; column: "column 1"; value: "1"; }
                ListElement{ row: "row 1"; column: "column 2"; value: "2"; }
                ListElement{ row: "row 1"; column: "column 3"; value: "3"; }
            }

            rowRole: "row"
            columnRole: "column"
            valueRole: "value"
        }
    }
}
