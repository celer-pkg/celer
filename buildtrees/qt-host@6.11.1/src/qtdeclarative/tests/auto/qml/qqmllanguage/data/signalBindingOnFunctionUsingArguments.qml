import QtQml

QtObject {
    property string result: ""
    property string expected: "{\"0\":\"world\"}"

    id: object

    signal hello(string arg)

    Component.onCompleted: {
        object.hello("world")
    }

    onHello: function (arg) {
        result = JSON.stringify(arguments)
    }
}
