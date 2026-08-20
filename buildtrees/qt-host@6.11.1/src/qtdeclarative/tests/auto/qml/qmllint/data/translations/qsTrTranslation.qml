import QtQuick

Item {
    property string qtTrNoopHello: QT_TR_NOOP("Hello")
    property string qsTrHello: qsTr("hello")
    property string qsTranslateHello: qsTranslate("context", "hello")
}
