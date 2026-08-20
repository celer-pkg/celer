import QtQuick

Item {
    id: root
    objectName: "test"
    property var myroot: root
    property bool notNan: isNaN(42)
    property bool stringTest: "foo".endsWith("oo") && myString.indexOf("t") === 0
    property rect myrect: Qt.rect()
    property bool myMath: Math.abs(Math.sin(Math.max(Math.log(42), Math.min(20, 30)))) > 0
    property string myArg: "hello %1".arg("World")
    property color myColor
    property var myColorDarker: Qt.darker(myColor, 0.5)
    property var myQuaternion: Qt.quaternion(1, 2, 3, 4)
    property string myString: (1 + 1).toString()
    // qmllint disable translation-function-mismatch
    property string myTranslatedString: qsTr("hello", "greeting")
    property string myTranslatedStringId: qsTrId("hello-id")
    property string myTranslatedString2: QT_TR_NOOP("world")
    // qmllint enable translation-function-mismatch
}
