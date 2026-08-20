import QtQml

QtObject {
    property string result: ""
    property string expected: "{\"0\":\"otherarg\"}"

    id: object

    signal hello(string arg)

    Component.onCompleted: {
        object.hello("world")
    }

    onHello: function (arg) {
        function inner (innerarg) {
            result = JSON.stringify(arguments)
        }

        inner("otherarg")
    }
}
