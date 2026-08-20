// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

ListSelection {
    id: listEffects
    width: 580
    height: 1050

    selection: "None"

    State_Idle {
        id: none
        x: 0
        y: 0
        width: 580
        height: 88
        item_nameText: "None"
        checkboxImagesState: "state_type_Effects_Number_8"
        selection: listEffects
    }

    State_Idle {
        id: rain
        x: 0
        y: 96
        width: 580
        height: 88
        item_nameText: "Rain"
        checkboxImagesState: "state_type_Effects_Number_6"
        selection: listEffects
    }

    State_Idle {
        id: snow
        x: 0
        y: 192
        width: 580
        height: 88
        item_nameText: "Snow"
        checkboxImagesState: "state_type_Effects_Number_5"
        selection: listEffects
    }

    State_Idle {
        id: smole
        x: 0
        y: 288
        width: 580
        height: 88
        item_nameText: "Smoke"
        checkboxImagesState: "state_type_Effects_Number_4"
        selection: listEffects
    }

    State_Idle {
        id: heatwave
        x: 0
        y: 384
        width: 580
        height: 88
        item_nameText: "Heatwave"
        checkboxImagesState: "state_type_Effects_Number_3"
        selection: listEffects
    }

    State_Idle {
        id: other1
        x: 0
        y: 480
        width: 580
        height: 88
        item_nameText: "Sparks"
        checkboxImagesState: "state_type_Effects_Number_2"
        selection: listEffects
    }

    State_Idle {
        id: other2
        x: 0
        y: 576
        width: 580
        height: 88
        item_nameText: "Flames"
        checkboxImagesState: "state_type_Effects_Number_1"
        selection: listEffects
    }

    State_Idle {
        id: other21
        x: 0
        y: 672
        width: 580
        height: 88
        item_nameText: "Dust"
        checkboxImagesState: "state_type_Effects_Number_7"
        selection: listEffects
    }

    State_Idle {
        id: other22
        x: 0
        y: 768
        width: 580
        height: 88
        item_nameText: "Clouds"
        checkboxImagesState: "state_type_Effects_Number_9"
        selection: listEffects
    }

    State_Idle {
        id: other23
        x: 0
        y: 865
        width: 580
        height: 88
        item_nameText: "Explosion"
        checkboxImagesState: "state_type_Effects_Number_10"
        selection: listEffects
    }

    State_Idle {
        id: other24
        x: 0
        y: 962
        width: 580
        height: 88
        item_nameText: "Bubbles"
        checkboxImagesState: "state_type_Effects_Number_11"
        selection: listEffects
    }
}

/*##^##
Designer {
    D{i:0;height:1050;width:580}D{i:1;uuid:"1a64e024-c630-5dad-a09f-be30d0edc445"}D{i:2;uuid:"e80a93e7-1251-5ba2-9545-47409bf0be1c"}
D{i:3;uuid:"476cae96-dacf-5b5d-a34f-40ca069814a1"}D{i:4;uuid:"c6959992-fe57-5f49-8a74-c5e1c2404921"}
D{i:5;uuid:"e495756c-e8df-58cb-8c3c-ce71f0b14c4f"}D{i:6;uuid:"9ab1c75c-f885-51dd-a7ad-4d3fee391b09"}
D{i:7;uuid:"5552fc87-577d-5949-bb60-280a608ba544"}D{i:8;uuid:"281906eb-de7b-5ea1-a3bd-afdcaaf4e5f9"}
D{i:9;uuid:"aaca4b05-0a5d-53ca-876e-806142e3e720"}D{i:10;uuid:"bde4ea91-d841-5df7-b133-0b69dc180cb9"}
D{i:11;uuid:"0ea1def8-1ad9-5b73-91c5-d5a53d3ec553"}
}
##^##*/

