// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

ListSelection {
    id: listMaterial
    width: 580
    height: 952
    property alias silverBtn: silverBtn

    selection: "Silver"

    State_Idle {
        id: silverBtn
        x: 0
        y: 0
        width: 580
        height: 88
        item_nameText: "Silver"
        checkboxImagesState: "state_type_Materials_Number_10"
        selection: listMaterial
    }

    State_Idle {
        id: brushedSteelBtn
        x: 0
        y: 96
        width: 580
        height: 88
        item_nameText: "Brushed Steel"
        checkboxImagesState: "state_type_Materials_Number_9"
        selection: listMaterial
    }

    State_Idle {
        id: copperBtn
        x: 0
        y: 192
        width: 580
        height: 88
        item_nameText: "Copper"
        checkboxImagesState: "state_type_Materials_Number_8"
        selection: listMaterial
    }

    State_Idle {
        id: glassBtn
        x: 0
        y: 288
        width: 580
        height: 88
        item_nameText: "Wax"
        checkboxImagesState: "state_type_Materials_Number_7"
        selection: listMaterial
    }

    State_Idle {
        id: woodBtn
        x: 0
        y: 384
        width: 580
        height: 88
        item_nameText: "Wood"
        checkboxImagesState: "state_type_Materials_Number_6"
        selection: listMaterial
    }

    State_Idle {
        id: stoneBtn
        x: 0
        y: 480
        width: 580
        height: 88
        item_nameText: "Stone"
        checkboxImagesState: "state_type_Materials_Number_5"
        selection: listMaterial
    }

    State_Idle {
        id: carbonFiberBtn
        x: 0
        y: 576
        width: 580
        height: 88
        item_nameText: "Carbon Fiber"
        checkboxImagesState: "state_type_Materials_Number_4"
        selection: listMaterial
    }

    State_Idle {
        id: plasticBtn
        x: 0
        y: 672
        width: 580
        height: 88
        item_nameText: "Plastic"
        checkboxImagesState: "state_type_Materials_Number_3"
        selection: listMaterial
    }

    State_Idle {
        id: fabricBtn
        x: 0
        y: 768
        width: 580
        height: 88
        item_nameText: "Fabric"
        checkboxImagesState: "state_type_Materials_Number_2"
        selection: listMaterial
    }

    State_Idle {
        id: leatherBtn
        x: 0
        y: 864
        width: 580
        height: 88
        item_nameText: "Leather"
        checkboxImagesState: "state_type_Materials_Number_1"
        selection: listMaterial
    }
}

/*##^##
Designer {
    D{i:0;height:952;width:580}D{i:1;uuid:"48276ca8-dc8a-53e9-91d0-ff5e4d5eb722"}
}
##^##*/

