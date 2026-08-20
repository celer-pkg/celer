import QtQml as QtQml
import QtQuick
import Test 1.0

MyTypeObject {
    property int r_type: easeCurve.type
    property real r_amplitude: easeCurve.amplitude
    property real r_overshoot: easeCurve.overshoot
    property real r_period: easeCurve.period
    property var r_bezierCurve: easeCurve.bezierCurve
    property variant copy: easeCurve
    readonly property easingCurve inOutQuadCurve: Easing.InOutQuad
    readonly property easingCurve outQuintCurve: new QtQml.easingCurve(Easing.OutQuint)
    readonly property easingCurve inElasticCurve: ({
        type: Easing.InElastic,
        amplitude: 4,
        period: 3
    })
}
