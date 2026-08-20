import QtQml

QtObject {
    enum E { A, B, C }
    property int e1: RedundantOptionalChainingEnums?.B
    property int e2: Qt?.Horizontal
}
