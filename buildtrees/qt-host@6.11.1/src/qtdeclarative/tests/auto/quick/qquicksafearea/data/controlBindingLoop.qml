import QtQuick
import QtQuick.Controls

Window {
    flags: Qt.FramelessWindowHint
    width: windowSize.width; height: windowSize.height
    color: "white"

    required property size windowSize

    required property var safeAreaMargins
    SafeArea.additionalMargins: safeAreaMargins

    required property point anchorPosition
    required property var explicitSize

    required property string currentDataTag

    required property var extraPadding

    Control {
        objectName: "control"

        anchors {
            top: anchorPosition.y == 0 ? parent.top : undefined
            bottom: anchorPosition.y == windowSize.height ? parent.bottom : undefined
            left: anchorPosition.x == 0 ? parent.left : undefined
            right: anchorPosition.x == windowSize.width ? parent.right : undefined
        }

        width: explicitSize ? explicitSize.width : undefined
        height: explicitSize ? explicitSize.height : undefined

        topPadding: SafeArea.margins.top + extraPadding.top
        bottomPadding: SafeArea.margins.bottom + extraPadding.bottom
        leftPadding: SafeArea.margins.left + extraPadding.left
        rightPadding: SafeArea.margins.right + extraPadding.right

        background: Rectangle {
            gradient: Gradient.WinterNeva
        }

        contentItem: Rectangle {
            objectName: "controlContentItem"
            gradient: Gradient.DustyGrass
            implicitWidth: 100
            implicitHeight: 100
        }
    }

    Rectangle {
        color: "transparent"
        border.color: "magenta"
        anchors {
            fill: parent
            topMargin: parent.SafeArea.margins.top
            bottomMargin: parent.SafeArea.margins.bottom
            leftMargin: parent.SafeArea.margins.left
            rightMargin: parent.SafeArea.margins.right
        }
    }

    Text {
        text: currentDataTag + " (" + (explicitSize ?
            "explicit" : "implicit") + " size)"
        anchors.centerIn: parent
    }
}
