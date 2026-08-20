import QtQuick

Item {
    property string qsTrIdInExpression: "Hello" + qsTrId("World") + "!"
    property string qsTrInExpression: "Hello" + qsTr("World") + "!"
    property string qsTranslateInExpression: "Hello" + qsTranslate("context", "World") + "!"
}
