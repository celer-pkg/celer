import QtQuick
import Test

Item {
    width: 100
    height: 100

    property alias tableView: root

    TableView {
        id: root
        width: 100
        height: 100

        property Component typedDelegate: Item {
            implicitWidth: 10
            implicitHeight: 10

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

        property Component untypedDelegate: Item {
            implicitWidth: 10
            implicitHeight: 10

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
        }

        property ListModel listModel: ListModel {
            ListElement {
                a: 11
                y: 12
            }
        }

        property var array: [ {a: 11, y: 12} ]

        property QtObject object: QtObject {
            property int a: 11
            property int y: 12
        }

        function aAt0() : real {
            switch (modelIndex) {
            case Model.Singular:
            case Model.List:
                return model.get(0).a
            case Model.Array:
                return model[0].a
            case Model.Object:
                return model.a
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
}
