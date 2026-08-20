import QtQml

Instantiator {
    objectName: object
    delegate: Qt.createComponent(source)
    property url source: "MyItem.qml"
}
