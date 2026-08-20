// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

ListSelection {
    id: listModels
    width: 580
    height: 376
    property alias rabbit: rabbit

    selection: "Dragon"

    State_Idle {
        id: rabbit
        x: 0
        y: 96
        width: 580
        height: 88
        item_nameText: "Rabbit"
        checkboxImagesState: "state_type_3DModel_Number_2"
        selection: listModels
    }

    State_Idle {
        id: none
        x: 0
        y: 0
        width: 580
        height: 88
        item_nameText: "None"
        checkboxImagesState: "state_type_3DModel_Number_1"
        selection: listModels
    }

    State_Idle {
        id: headbust
        x: 0
        y: 288
        width: 580
        height: 88
        item_nameText: "Dragon"
        checkboxImagesState: "state_type_3DModel_Number_4"
        selection: listModels
    }

    State_Idle {
        id: qtMaterialBall
        x: 0
        y: 192
        width: 580
        height: 88
        item_nameText: "Qt material ball"
        checkboxImagesState: "state_type_3DModel_Number_3"
        selection: listModels
    }
}

/*##^##
Designer {
    D{i:0;height:376;width:580}D{i:1;uuid:"5655c12e-3314-5270-93f1-30393ca6bd10"}D{i:2;uuid:"0f43834f-bd09-5e54-9317-6683c9f3a480"}
D{i:3;uuid:"48016141-dc04-548b-9ba4-25ab1bf3ee95"}D{i:4;uuid:"db69947c-c4ea-5fde-a1a9-c3535ae3e73d"}
}
##^##*/

