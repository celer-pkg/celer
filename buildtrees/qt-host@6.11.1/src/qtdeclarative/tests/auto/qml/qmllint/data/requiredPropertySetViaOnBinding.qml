import QtQuick

Window {
    id: window
    width: 640
    height: 480
    visible: true

    property int refValue: 0

    component TestObject: QtObject {
        required property int value
    }


    TestObject {
        // consistent with runtime
        Behavior on value {}
    }

    TestObject {
        Binding on value {
            value: 42
        }
    }
}

