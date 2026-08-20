import QtQml
import StaticTest

QtObject {
    enum QmlEnum { A, B }

    property string p1: Qt.enumValueToString(EnumValueToString.QmlEnum, EnumValueToString.QmlEnum.B)
    property string p2: Qt.enumValueToString(EnumValueToString.QmlEnum, EnumValueToString.B)

    property string p3: Qt.enumValueToString(CppEnum.Scoped, CppEnum.Scoped.S2)
    property string p4: Qt.enumValueToString(CppEnum.Scoped, CppEnum.S2)
    property string p5: Qt.enumValueToString(CppEnum.Unscoped, CppEnum.Unscoped.U2)
    property string p6: Qt.enumValueToString(CppEnum.Unscoped, CppEnum.U2)

    property string p7: Qt.enumValueToString(EnumNamespace.Scoped, EnumNamespace.Scoped.S2)
    property string p8: Qt.enumValueToString(EnumNamespace.Scoped, EnumNamespace.S2)
    property string p9: Qt.enumValueToString(EnumNamespace.Unscoped, EnumNamespace.Unscoped.U2)
    property string p10: Qt.enumValueToString(EnumNamespace.Unscoped, EnumNamespace.U2)


    property var p11: capture(() => Qt.enumValueToString(1, 2))
    property var p12: capture(() => Qt.enumValueToString(EnumValueToString.QmlEnum, 2))
    property var p13: capture(() => Qt.enumValueToString(EnumValueToString.QmlEnum, EnumValueToString.NOPE))

    property var p14: Qt.enumValueToString(ConflictingEnums.E1, ConflictingEnums.E1.A /* 1 */)
    property var p15: Qt.enumValueToString(ConflictingEnums.E1, ConflictingEnums.A /* 2 */)

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
