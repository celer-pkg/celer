import QtQuick

Item {
    id: root

    property bool isRectBlue: false
    property bool shouldHaveListItem: false
    property bool shouldHaveListInt: false
    property bool hasListItem: myList.length > 0
    property bool hasListInt: myIntList.length > 0
    property alias color: rect.color

    property list<int> myIntList

    Binding on myIntList {
        when: shouldHaveListInt
        value: 42
    }

    property list<Item> myList

    Binding on myList {
        when: root.shouldHaveListItem
        value: rect
    }

    width: 640
    height: 480

    Rectangle {
        id: rect
        anchors {
            top: parent.top
            left: parent.left
            right: parent.horizontalCenter
            bottom: parent.bottom
        }

        Binding on color {
            when: root.isRectBlue
            value: "blue"
        }
    }
}
