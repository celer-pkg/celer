import QtQml
import Test

DelegateModel {
    id: root

    property Component typedDelegate: QtObject {
        required property QtObject model

        required property real x

        property real immediateX: x
        property real modelX: model.x

        function writeImmediate() {
            x = 1;
        }

        function writeThroughModel() {
            model.x = 3;
        }
    }

    property Component untypedDelegate: QtObject {
        property real immediateX: x
        property real modelX: model.x

        function writeImmediate() {
            x = 1;
        }

        function writeThroughModel() {
            model.x = 3;
        }
    }

    property ListModel singularModel: ListModel {
        ListElement {
            x: 11
        }
        ListElement {
            x: 12
        }
    }

    property ListModel listModel: ListModel {
        ListElement {
            x: 11
            y: 12
        }
        ListElement {
            x: 15
            y: 16
        }
    }

    property var array: [
        {x: 11, y: 12}, {x: 19, y: 20}
    ]

    property QtObject object: QtObject {
        property int x: 11
        property int y: 12
    }

    function xAt0() : real {
        switch (modelIndex) {
        case Model.Singular:
        case Model.List:
            return model.get(0).x
        case Model.Array:
            return model[0].x
        case Model.Object:
            return model.x
        }
        return -1;
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
