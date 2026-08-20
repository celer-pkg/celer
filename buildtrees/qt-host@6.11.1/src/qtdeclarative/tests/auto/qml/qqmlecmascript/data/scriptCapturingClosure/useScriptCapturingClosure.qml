import QtQml

QtObject {
    id: root
    property var func
    property int state: -1

    function run() {
        root.state = root.func() ? 1 : 0;
    }

    Component.onCompleted: {
        const comp = Qt.createComponent("./ClosureMaker.qml");
        let o = comp.createObject();
        root.func = o.getClosure();
        o.destroy();
    }
}
