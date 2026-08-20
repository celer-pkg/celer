import QtQml
import Test

Instantiator {
    id: root
    objectName: "hundred"

    property Component typedDelegate: QtObject {
        objectName: "ten"

        required property QtObject model

        required property real a

        property real immediateX: a
        property real modelX: model.a

        function writeImmediate() {
            a = 1;
        }

        function writeThroughModel() {
            model.a = 3;
        }
    }

    property Component untypedDelegate: QtObject {
        objectName: "ten"

        property real immediateX: a
        property real modelX: model.a

        function writeImmediate() {
            a = 1;
        }

        function writeThroughModel() {
            model.a = 3;
        }
    }

    property ListModel singularModel: ListModel {
        ListElement {
            a: 11
        }
        ListElement {
            a: 12
        }
    }

    property ListModel listModel: ListModel {
        ListElement {
            a: 11
            y: 12
        }
        ListElement {
            a: 15
            y: 16
        }
    }

    property var array: [
        {a: 11, y: 12}, {a: 19, y: 20}
    ]

    property QtObject object: QtObject {
        property int a: 11
        property int y: 12
    }

    property int modelIndex: Model.None
    property int delegateIndex: Delegate.None

    model: {
        switch (modelIndex) {
        case Model.Singular: return singularModel
        case Model.List: return listModel
        case Model.Array: return array
        case Model.Object: return object
        }
        return undefined;
    }

    delegate: {
        switch (delegateIndex) {
        case Delegate.Untyped: return untypedDelegate
        case Delegate.Typed: return typedDelegate
        }
        return null
    }
}
