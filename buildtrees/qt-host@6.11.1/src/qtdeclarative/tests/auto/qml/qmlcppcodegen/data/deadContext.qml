pragma ComponentBehavior: Bound
import QtQuick

Item {
    id: root

    property Component a: Item {
        Timer {
            interval: 1
            running: root.choice < 4

            function doit() {}
            onTriggered: {
                ++root.choice
                doit()
            }
        }
    }

    property Component b: Item {
        Timer {
            interval: 1
            running: root.choice < 4

            function doit() {}
            onTriggered: {
                ++root.choice
                doit()
            }
        }
    }

    property int choice: 0

    Loader {
        sourceComponent: switch (root.choice % 2) {
            case 0: return root.a
            case 1: return root.b
        }
    }
}
