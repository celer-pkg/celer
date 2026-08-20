import QtQml

QtObject {
    property string result: ""
    property string expected: "{\"0\":\"world\"}"

    id: root

    property var myfunc: function() {  root.result = JSON.stringify(arguments)  }

    Component.onCompleted: myfunc("world")
}
