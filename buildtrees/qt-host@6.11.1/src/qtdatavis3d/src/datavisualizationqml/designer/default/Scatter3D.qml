// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtDataVisualization 1.2

Scatter3D {
    width: 300
    height: 300
    Scatter3DSeries {
        ItemModelScatterDataProxy {
            itemModel: ListModel {
                ListElement{ x: "1"; y: "2"; z: "3"; }
                ListElement{ x: "2"; y: "3"; z: "4"; }
                ListElement{ x: "3"; y: "4"; z: "1"; }
            }

            xPosRole: "x"
            yPosRole: "y"
            zPosRole: "z"
        }
    }
}
