import QtQuick

Item {
    function evilFunction() {}

    Connections {
        function okFunction() {}
    }
    ScriptAction {
        function okFunction() {}
    }
    Item {
        function evilFunction() {}
    }
    Connections {
        property var okArrow: x => x
    }
}
