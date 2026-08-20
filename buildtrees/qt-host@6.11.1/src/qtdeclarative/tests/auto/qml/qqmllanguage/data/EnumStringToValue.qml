import QtQml
import StaticTest

QtObject {
    enum QmlEnum { A, B }

    property int p1: Qt.enumStringToValue(EnumStringToValue.QmlEnum, "B")
    property int p2: Qt.enumStringToValue(CppEnum.Scoped, "S2")
    property int p3: Qt.enumStringToValue(CppEnum.Unscoped, "U2")
    property int p4: Qt.enumStringToValue(EnumNamespace.Scoped, "S2")
    property int p5: Qt.enumStringToValue(EnumNamespace.Unscoped, "U2")


    property string p6: capture(() => Qt.enumStringToValue(undefined, "A"))
    property string p7: capture(() => Qt.enumStringToValue(CppEnum.Scoped, undefined))

    property int p8: Qt.enumStringToValue(ConflictingEnums.E1, "A")
    property int p9: Qt.enumStringToValue(ConflictingEnums.E2, "A")

    function capture(f) {
        let s = "<no exception>"
        try {
            f()
        } catch(e) {
            s = e.message
        }
        return s
    }
}
