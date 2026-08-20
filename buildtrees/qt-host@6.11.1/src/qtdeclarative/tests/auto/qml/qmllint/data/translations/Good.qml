import QtQuick

Item {
    property string translated: qsTrId("Hello")
    property string translatedNoOP: QT_TRID_NOOP("Hello")
    property string qsTrInExpression: "Hello" + qsTr("World") + "!"
}
