import QtQuick
import QtQuick.Controls

// ApplicationWindow can't be resolved because it's in QtQuick.Controls ...
ApplicationWindow {
    // ... therefore we don't know whether "header" is a Component.
    // ... which means we cannot tell whether item1 is inside that "Component" or outside of it.
    header: Item { property Item item1: item1 }
    Item {
        objectName: "outer"
        id: item1
    }

    objectName: header.item1.objectName
}
