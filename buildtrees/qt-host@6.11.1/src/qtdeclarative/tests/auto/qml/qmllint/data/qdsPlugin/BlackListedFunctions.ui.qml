import QtQuick

Item {
    id: root

    function evil() {
        evil();
        root.evil();
        let x = function () {}
    }

    property int qualifiedCall: root.evil()
    property int unqualifiedCall: evil()
}
