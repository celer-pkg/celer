import QtQuick

QtObject {
    readonly property real linearValue: Easing.valueForProgress(Easing.Linear, 0.5)
    readonly property real inQuadValue: Easing.valueForProgress(Easing.InQuad, 0.5)
}
