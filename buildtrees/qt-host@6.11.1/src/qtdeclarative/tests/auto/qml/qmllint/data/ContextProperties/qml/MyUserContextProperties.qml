import QtQuick

Item {
    property var p1: myIgnoredCP1.someProperty
    property var p2: myIgnoredCP2("asdf")
    property var p3: myIgnoredCP2
    property var p4: myUserCP1.someProperty1.someProperty2("123")
    property var p5: myUserCP2("asdf")
}
