import QtQml
import StaticTest

QtObject {

    enum E { A, B, C }

    property int qmlAsScoped: EnumScoping.E.B
    property int qmlAsUnscoped: EnumScoping.B

    property int cppScopedAsScoped: CppEnum.Scoped.S2
    property int cppScopedAsUnscoped: CppEnum.S2

    property int cppUnscopedAsScoped: CppEnum.Unscoped.U2
    property int cppUnscopedAsUnscoped: CppEnum.U2

    property int nsScopedAsScoped: EnumNamespace.Scoped.S2
    property int nsScopedAsUnscoped: EnumNamespace.S2

    property int nsUnscopedAsScoped: EnumNamespace.Unscoped.U2
    property int nsUnscopedAsUnscoped: EnumNamespace.U2
}
