import QtQml

QtObject {
    property bool toggle: true
    property list<QtObject> data: toggle ?  [ a, b ] : [ c ]
    property int length: data.length

    property QtObject a: QtObject {}
    property QtObject b: QtObject {}
    property QtObject c: QtObject {}
}
