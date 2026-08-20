import QtQml

QtObject {
    property url foo
    property bool b: foo?.toString()
}
