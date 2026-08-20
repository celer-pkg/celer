import QtQuick

Item {
    property int myInt: 123
    property int block: { return 123; }
    property int alsoABlock: {42}
    property var lambda: function() {}
    property var namedFunction: function hello() {}
    property var arrow: x => x
    property var myUndefined: undefined
    property var emptyBlock: {}
    property var emptyObject: ({})
}
