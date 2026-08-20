import QtQml
import TestTypes

StaleBindingPropertyRead {
    id: o
    property int i: 0
    readonly property int ro: 0

    // Warn
    property int p1: o.cppStale
    property int p2: o.cppReadonly

    // No Warn
    property int p3: o.cppConstant
    property int p4: o.cppNotifiable
    property int p5: o.cppConstantNotifiable
    property int p6: o.i
    property int p7: o.ro
}
