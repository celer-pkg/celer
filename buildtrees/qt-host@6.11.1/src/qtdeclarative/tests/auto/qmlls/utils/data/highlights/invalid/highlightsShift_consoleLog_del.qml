import QtQuick

HasEnumAndAttachedType {
    property var enumValue: 1
    Component.onCompleted: 
        console.log(enumValue)
    }
}
