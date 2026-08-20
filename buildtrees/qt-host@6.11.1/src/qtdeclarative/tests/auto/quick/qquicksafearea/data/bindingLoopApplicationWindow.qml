import QtQuick
import QtQuick.Controls

ApplicationWindow {
    flags: Qt.FramelessWindowHint
    width: 500; height: 500

    signal itemWidthChanged

    footer: Control {
        rightPadding: SafeArea.margins.right
    }

    onWidthChanged: itemWidthChanged()
}
