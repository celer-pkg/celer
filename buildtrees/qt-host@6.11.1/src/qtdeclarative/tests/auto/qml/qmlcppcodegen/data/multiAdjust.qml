pragma Strict
import QtQml

QtObject {
    id: mainRow

    property int calledFoo: 0
    property int calledBar: 0

    function foo(column: int) : void { ++calledFoo }
    function bar(column: int) : void { ++calledBar }

    property int a: 0
    property int b: 0

    signal event(key: int)

    onEvent: key => {
        let forwardArrowKey = (key === Qt.Key_Right && !mainRow.objectName.length)
        if (2 > a)
            mainRow.foo(mainRow.b)
        if (forwardArrowKey)
            mainRow.bar(mainRow.b)
    }
}
