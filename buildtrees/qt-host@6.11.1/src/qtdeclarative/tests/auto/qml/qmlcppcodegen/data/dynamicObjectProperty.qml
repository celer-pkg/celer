import QtQml

QtObject {
    id: self

    readonly property var shorthand: ({
        answer(number) { return number; }
    })

    readonly property var getter: ({
        get answer() { return 42 }
    })

    readonly property var setter: ({
        set answer(number) { self.objectName += number }
    })

    objectName: shorthand.answer(20) + getter.answer;
    Component.onCompleted: setter.answer = 99
}
