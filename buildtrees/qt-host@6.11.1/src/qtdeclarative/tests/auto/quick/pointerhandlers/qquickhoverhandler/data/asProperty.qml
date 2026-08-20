import QtQuick

Item {
    id: root
    width: 200
    height: 200

    property HoverHandler handler: HoverHandler {
        parent: root
    }
}

