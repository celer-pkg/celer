import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 400
    height: 300
    title: "QTBUG-140018"

    Component {
        id: contentComponent

        Rectangle {
            color: "red"
        }
    }

    Component {
        id: stackViewComponent

        StackView {
            id: stackView
            anchors.fill: parent
        }
    }

    Loader {
        id: loader
        anchors.fill: parent

        sourceComponent: stackViewComponent
        active: true
    }

    Component.onCompleted: {
        let stackViewItem = loader.item
        loader.active = false
        stackViewItem.replace(contentComponent, {})
    }
}
