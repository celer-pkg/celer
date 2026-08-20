import QtQuick

Connections {
    property int ok: 123
    property int evilBlock: { 1 }
    property var evilLambda: function() {}
    property var evilFunction: function hello() {}
    property var evilArrow: x => x
}
