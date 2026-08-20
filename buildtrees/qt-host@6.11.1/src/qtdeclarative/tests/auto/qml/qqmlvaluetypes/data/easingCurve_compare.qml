import QtQml as QtQml
import QtQuick
import Test 1.0

MyTypeObject {
    property variant copy: easeCurve
    property string tostring: easeCurve.toString()

    property bool equalsString: (easeCurve == tostring)

    property bool equalsSelf: (easeCurve == easeCurve) // true
    property bool equalsOther: (easeCurve ==
        new QtQml.easingCurve(QtQml.Easing.OutQuart)) // false
}

