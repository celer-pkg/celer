import QtQuick

HasEnumAndAttachedType {
    property var enumValue: 1
    Component.onCompleted: {
        console.log(enumValue)
    }
}

// This is a comment line
// Another comment line