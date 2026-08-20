pragma ComponentBehavior: Bound
import QtQml

QtObject {
    id: root

    property string result
    property string text: "foo"
    function toggle() {
        text = (text === "foo") ? "bar" : "foo"
        repeater.model = [text]
    }

    property Instantiator instantiator: Instantiator {
        id: repeater
        model: 0
        delegate: QtObject {
            id: delegate
            required property string modelData
            property Binding binding: Binding {
                target: root
                property: "objectName"
                value: delegate.modelData
            }

            property Binding binding2: Binding {
                delegate.objectName: delegate.modelData
            }
        }
    }
}
