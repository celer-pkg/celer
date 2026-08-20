import QtQuick

Item {
    property int isRoot


    Item {
        property int isNotRoot
    }
    component IC: Item {
        property int isNotRoot
    }
}
