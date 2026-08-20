import QtQml

QtObject {
    property list<int> intList: [0, 1, 2, 3]
    property list<int> spliced
    Component.onCompleted: spliced = intList.splice(2)
}
