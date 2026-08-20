import QtQml

Timer {
    id: root

    property var arr
    property var obj: ({})
    readonly property var copy1: copy(obj)
    readonly property var copy2: copy(copy1)

    function test(): void {
        arr = ["x"];
        obj = { key: { sub: {foo: 5} } };
    }

    function copy(o: var): var {
        gc();
        return JSON.parse(JSON.stringify(o));
    }

    interval: 1
    repeat: true
    running: true
    onTriggered: {
        ++count;
        test();
    }

    property int count: 0
}
