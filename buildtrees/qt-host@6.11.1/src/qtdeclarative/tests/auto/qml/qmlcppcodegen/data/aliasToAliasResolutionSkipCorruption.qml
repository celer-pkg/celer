import QtQuick

Item {
    property alias dummy: inner.i
    property alias aai: middle.ai
    Item {
        id: middle
        property alias ai: inner.i
        Item {
            id: inner
            property int i: 0
        }
    }
}
