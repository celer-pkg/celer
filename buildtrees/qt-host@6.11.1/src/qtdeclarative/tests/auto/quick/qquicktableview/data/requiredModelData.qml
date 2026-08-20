import QtQuick

TableView {
    width: 100
    height: 100

    id: root
    property TableView tableView: root

    property ListModel singularModel: ListModel {
        ListElement {
            a: "a"
        }
    }

    property ListModel listModel: ListModel {
        ListElement {
            a: "a"
            b: "b"
        }
    }

    property var array: [ {a: "a", b: "b"} ]

    property QtObject object: QtObject {
        property string a: "a"
        property string b: "b"
    }

    property int n: -1

    model: {
        switch (n) {
        case 0: return singularModel
        case 1: return listModel
        case 2: return array
        case 3: return object
        }
        return undefined;
    }

    delegate: Item {
        implicitWidth: 10
        implicitHeight: 10
        required property string a
    }
}
