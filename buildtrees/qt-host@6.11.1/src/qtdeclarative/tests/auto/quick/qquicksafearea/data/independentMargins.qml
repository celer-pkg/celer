import QtQuick

Window {
    flags: Qt.FramelessWindowHint
    width: 500; height: 500

    Item {
        width: 200; height: 200
        anchors.centerIn: parent
        SafeArea.additionalMargins.top: 50

        Item {
            objectName: "topMarginItem"
            width: 100; height: 100
            x: -30
            property var margins: SafeArea.margins
        }
    }

    Item {
        width: 200; height: 200
        anchors.centerIn: parent
        SafeArea.additionalMargins.left: 50

        Item {
            objectName: "leftMarginItem"
            width: 100; height: 100
            y: -30
            property var margins: SafeArea.margins
        }
    }

    Item {
        width: 200; height: 200
        anchors.centerIn: parent
        SafeArea.additionalMargins.right: 50

        Item {
            objectName: "rightMarginItem"
            width: parent.width; height: parent.height * 2
            property var margins: SafeArea.margins
        }
    }

    Item {
        width: 200; height: 200
        anchors.centerIn: parent
        SafeArea.additionalMargins.bottom: 50

        Item {
            objectName: "bottomMarginItem"
            width: parent.width * 2; height: parent.height
            property var margins: SafeArea.margins
        }
    }
}
