import QtQuick

Item {
    id: root
    width: 640
    height: 480
    visible: true

    Item {

        width: parent.width
        height: width
    }

    property string cPropDef: "value"
    property var aPropDef: ({})

    // This comment belongs to activeService
    readonly property string bPropDef: "value"

    signal bSignal
    // This signal should go before any function
    signal aSignal
    objectName: "myRoot"
    function bFunc(arg) {
        // pick a child object from the main model using the service name passed as a key
        return arg * 2;
    }

    function aFunc(key) {
        switch (key) {
        case "0":
            return 0;
        case "11":
            return 11;
        }
        return -1;
    }

    enum Benum {
        Black = 0, White = 1, Grey = 2, Gray = 2
    }

    enum Aenum {
        Black = 0, White = 1, Grey = 2, Gray = 2
    }

    onPaint: context => {
        context.lineTo(width / 2, height);
        context.closePath();
        context.fillStyle = control.pressed ? control.defaultColor : control.defaultColor;
        context.fill();
    }
}
