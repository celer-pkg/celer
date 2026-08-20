// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    width: 400; height: 200
    color: "white"

    Grid {
        anchors.centerIn: parent
        columns: 4; rows: 2; spacing: 10

        Rectangle {
            color: "steelblue"; width: 80; height: 80
            rotation: 10
        }
        Rectangle {
            color: "#F0A080"; width: 80; height: 80
            border.color: "gray"; rotation: 10
        }
        Rectangle {
            color: "steelblue"; width: 80; height: 80
            radius: 10; rotation: 10
        }
        Rectangle {
            color: "#F0A080"; width: 80; height: 80
            radius: 10; border.color: "gray"
            rotation: 10
        }

        Rectangle {
            color: "steelblue"; width: 80; height: 80
            rotation: 10; smooth: true
        }
        Rectangle {
            color: "#F0A080"; width: 80; height: 80
            border.color: "gray"; rotation: 10; smooth: true
        }
        Rectangle {
            color: "steelblue"; width: 80; height: 80
            radius: 10; rotation: 10; smooth: true
        }
        Rectangle {
            color: "#F0A080"; width: 80; height: 80
            radius: 10; border.color: "gray"
            rotation: 10; smooth: true
        }
    }
}
