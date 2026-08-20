import QtQml

QtObject {
    property string result: ""
    property string expected: "{}"

    id: object

    signal hello(string arg)

    Component.onCompleted: {
        object.hello("world")
    }

    onHello: (arg) => {
        result = JSON.stringify(arguments)
    }
}
