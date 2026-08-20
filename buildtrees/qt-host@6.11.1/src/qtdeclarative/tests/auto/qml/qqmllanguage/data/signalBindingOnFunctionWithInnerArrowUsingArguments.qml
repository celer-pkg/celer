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
        let inner = (innerarg) => {
            result = JSON.stringify(arguments)
        }

        inner("otherarg")
    }
}
