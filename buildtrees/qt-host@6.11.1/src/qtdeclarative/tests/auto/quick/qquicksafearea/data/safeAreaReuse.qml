import QtQuick

Window {
    flags: Qt.FramelessWindowHint
    width: 500; height: 500

    property real first: SafeArea.margins.top
    property real second: contentItem.SafeArea.margins.top
}
