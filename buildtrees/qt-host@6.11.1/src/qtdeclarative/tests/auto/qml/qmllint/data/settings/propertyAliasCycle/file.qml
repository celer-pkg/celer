import QtQuick

Item {
    id: root
    property alias cycle1: root.cycle2
    property alias cycle2: root.cycle1
}
