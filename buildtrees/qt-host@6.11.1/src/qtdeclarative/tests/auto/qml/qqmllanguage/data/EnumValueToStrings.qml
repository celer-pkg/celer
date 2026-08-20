import QtQml
import StaticTest

QtObject {
    enum QmlEnum { A, B = 1, C = 1 }

    property int a: EnumValueToStrings.QmlEnum.A
    property int b: EnumValueToStrings.QmlEnum.B
    property int cpps1: CppEnum.Scoped.S1
    property int cpps3: CppEnum.Scoped.S3
    property int cppu1: CppEnum.Unscoped.U1
    property int cppu3: CppEnum.Unscoped.U3
    property int nss1: EnumNamespace.Scoped.S1
    property int nss3: EnumNamespace.Scoped.S3
    property int nsu1: EnumNamespace.Unscoped.U1
    property int nsu3: EnumNamespace.Unscoped.U3

    property list<string> p1: Qt.enumValueToStrings(EnumValueToStrings.QmlEnum, a)
    property list<string> p2: Qt.enumValueToStrings(EnumValueToStrings.QmlEnum, b)

    property list<string> p3: Qt.enumValueToStrings(CppEnum.Scoped, cpps1)
    property list<string> p4: Qt.enumValueToStrings(CppEnum.Scoped, cpps3)
    property list<string> p5: Qt.enumValueToStrings(CppEnum.Unscoped, cppu1)
    property list<string> p6: Qt.enumValueToStrings(CppEnum.Unscoped, cppu3)

    property list<string> p7: Qt.enumValueToStrings(EnumNamespace.Scoped, nss1)
    property list<string> p8: Qt.enumValueToStrings(EnumNamespace.Scoped, nss3)
    property list<string> p9: Qt.enumValueToStrings(EnumNamespace.Unscoped, nsu1)
    property list<string> p10: Qt.enumValueToStrings(EnumNamespace.Unscoped, nsu3)


    property var p11: capture(() => Qt.enumValueToStrings("NOPE", 9))
    property var p12: capture(() => Qt.enumValueToStrings(EnumValueToStrings.QmlEnum, 14))
    property var p13: capture(() => Qt.enumValueToStrings(EnumValueToStrings.QmlEnum, EnumValueToStrings.NOPE))

    property var p14: Qt.enumValueToStrings(ConflictingEnums.E1, ConflictingEnums.E1.A /* 1 */)
    property var p15: Qt.enumValueToStrings(ConflictingEnums.E1, ConflictingEnums.A /* 2 */)

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
