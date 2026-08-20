import QtQuick

Window {
    flags: Qt.FramelessWindowHint
    width: 500; height: 500

    SafeArea.additionalMargins {
        top: 200
        left: 50
    }

    Flickable {
        id: flickable
        objectName: "flickable"
        anchors.fill: parent

        contentWidth: contentChild.width
        contentHeight: contentChild.height

        contentItem.SafeArea.additionalMargins.top: 100

        Item {
            id: contentChild
            objectName: "contentChild"
            width: 1000; height: 1000
        }
    }
}
