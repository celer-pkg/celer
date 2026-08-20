import QtQuick

Window {
    flags: Qt.FramelessWindowHint
    width: 500; height: 500

    property var margins: SafeArea.margins

    Item {
        objectName: "fillItem"
        anchors.fill: parent
        property var margins: SafeArea.margins
    }

    Item {
        objectName: "topItem"
        height: 100; width: 100;
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
        property var margins: SafeArea.margins

        Item {
            objectName: "topChildItem"
            height: 50; width: 50;
            anchors {
                top: parent.top; topMargin: 3
            }
            property var margins: SafeArea.margins
        }
    }

    Item {
        objectName: "leftItem"
        height: 100; width: 100;
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }
        property var margins: SafeArea.margins

        Item {
            objectName: "leftChildItem"
            height: 50; width: 50;
            anchors {
                left: parent.left; leftMargin: 3
            }
            property var margins: SafeArea.margins
        }
    }

    Item {
        objectName: "rightItem"
        height: 100; width: 100;
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        property var margins: SafeArea.margins

        Item {
            objectName: "rightChildItem"
            height: 50; width: 50;
            anchors {
                right: parent.right; rightMargin: 3
            }
            property var margins: SafeArea.margins
        }
    }

    Item {
        objectName: "bottomItem"
        height: 100; width: 100;
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
        property var margins: SafeArea.margins

        Item {
            objectName: "bottomChildItem"
            height: 50; width: 50;
            anchors {
                bottom: parent.bottom; bottomMargin: 3
            }
            property var margins: SafeArea.margins
        }
    }

    Item {
        objectName: "centerItem"
        height: 100; width: 100;
        anchors {
            centerIn: parent
        }
        property var margins: SafeArea.margins

        Item {
            objectName: "centerChildItem"
            height: 50; width: 50;
            anchors {
                centerIn: parent
            }
            property var margins: SafeArea.margins
        }
    }
}
