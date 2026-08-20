import QtQml

QtObject {
    default final property string s
    final property int i1: 1
    final property int i2
    final readonly property int i3: 1
    final readonly property int i5: 5

    final required property int i6
    final required property int i7

    final property QtObject o1
    final property QtObject o2: QtObject {}
}
