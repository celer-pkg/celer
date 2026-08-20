import QtQuick

TableView {
    id: root

    width: 100
    height: 100

    property TableView tableView: root

    // useful object as model, int as modelData
    property ListModel singularModel: ListModel {
        ListElement {
            x: 11
        }
    }

    // same, useful, object as model and modelData
    property ListModel listModel: ListModel {
        ListElement {
            x: 13
            y: 14
        }
    }

    // useful but different objects as modelData and model
    // This is how the array accessor works. We can live with it.
    property var array: [ {x: 17, y: 18} ]

    // useful but different objects as modelData and model
    // This is how the object accessor works. We can live with it.
    property QtObject object: QtObject {
        property int x: 21
        property int y: 22
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
        required property point modelData
        required property QtObject model

        property real modelX: model.x
        property real modelDataX: modelData.x
        property point modelSelf: model
        property point modelDataSelf: modelData
        property point modelModelData: model.modelData
        property point modelAnonymous: model[""]
    }
}
