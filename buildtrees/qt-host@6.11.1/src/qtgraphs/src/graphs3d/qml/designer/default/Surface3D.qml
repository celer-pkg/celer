// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtGraphs

Surface3D {
    width: 300
    height: 300
    Surface3DSeries {
        ItemModelSurfaceDataProxy {
            itemModel: ListModel {
                ListElement{ row: "1"; column: "1"; y: "1"; }
                ListElement{ row: "1"; column: "2"; y: "2"; }
                ListElement{ row: "2"; column: "1"; y: "3"; }
                ListElement{ row: "2"; column: "2"; y: "4"; }
            }

            rowRole: "row"
            columnRole: "column"
            yPosRole: "y"
        }
    }
}
