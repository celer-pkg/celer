import QtQuick
import Test 1.0

MyTypeObject {
    easeCurve.type: if (true) Easing.BezierSpline
    // The next three bindings should have no effect since we're not a bounce or elastic curve.
    easeCurve.amplitude: if (true) 0.9
    easeCurve.overshoot: if (true) 0.8
    easeCurve.period: if (true) 0.7
    easeCurve.bezierCurve: if (true) [0.3, 0.3, 0.7, 0.7, 1.1, 1.1]
}
