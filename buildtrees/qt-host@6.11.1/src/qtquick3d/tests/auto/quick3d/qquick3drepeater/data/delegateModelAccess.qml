import QtQml
import QtQuick3D
import Test

Node {
    id: root
    x: 100
    y: 100

    property Component typedDelegate: Node {
        x: 10
        y: 10

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

    property Component untypedDelegate: Node {
        x: 10
        y: 10

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

    property int delegateModelAccess: DelegateModel.Qt5ReadWrite
    property int modelIndex: Model.None
    property int delegateIndex: Delegate.None

    property Repeater3D repeater: repeater

    Repeater3D {
        id: repeater
        delegateModelAccess: root.delegateModelAccess

        model: {
            switch (root.modelIndex) {
            case Model.Singular: return singularModel
            case Model.List: return listModel
            case Model.Array: return array
            case Model.Object: return object
            }
            return undefined;
        }

        delegate: {
            switch (root.delegateIndex) {
            case Delegate.Untyped: return untypedDelegate
            case Delegate.Typed: return typedDelegate
            }
            return null
        }
    }
}
