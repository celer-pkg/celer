pragma ComponentBehavior: Bound
import QtQml
import QtQuick3D

Node {
    id: root

    Loader3D {
        sourceComponent: Node {
            Component.onCompleted: root.objectName = "loaded"
        }
    }
}
