import QtQuick

Window {
    flags: Qt.FramelessWindowHint
    width: 500; height: 500

    SafeArea.additionalMargins.right: 50

    signal safeAreaRightMarginChanged
    signal itemWidthChanged

    Rectangle {
        anchors.fill: parent

        onWidthChanged: itemWidthChanged()

        property var rightMargin: SafeArea.margins.right
        onRightMarginChanged: safeAreaRightMarginChanged()
    }
}
