import QtQuick

Item {
    property int magic: parent.x

    id: root
    property int asdf: x /= 3
    property int qwer: root.opacity -= 3
}
