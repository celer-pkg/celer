import QtQuick
import "./a1.mjs" as A1
import "./b1.mjs" as B1

Item {
    Component.onCompleted: {
        A1.process_a1()
        B1.read_from_b1()
    }
}
