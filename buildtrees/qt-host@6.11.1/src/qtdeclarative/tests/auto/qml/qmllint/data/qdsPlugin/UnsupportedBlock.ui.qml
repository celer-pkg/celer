import QtQuick

Item {
    property int ok: 123
    property int evilBlock: {}
    property var evilLambda: function() {}
    property var evilFunction: function hello() {}
    property var evilArrow: x => x
}
