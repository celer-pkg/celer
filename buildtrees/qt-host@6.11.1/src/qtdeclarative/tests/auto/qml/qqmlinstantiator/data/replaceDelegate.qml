import QtQml

Instantiator {
    delegate: Qt.createComponent(source)
    property url source: "MyItem.qml"
}
