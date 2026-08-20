import QtQuick

Item {
    id: bindingTest

    property real initialSize: 5
    property real size: initialSize

    SequentialAnimation on size {
        id: animation

        running: false
    }

    SequentialAnimation on size {
        id: animation2

        running: false
    }

    Binding on y {
        when: objectName === "a"
        value: 2
    }
    Binding on y {
        when: objectName !== "a"
        value: 3
    }
}
