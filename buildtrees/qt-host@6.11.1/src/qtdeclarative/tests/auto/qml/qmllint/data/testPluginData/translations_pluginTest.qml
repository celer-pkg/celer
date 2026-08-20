import QtQuick 2.0

Item {
    property string translation: qsTr("Hello", "disambiguation", 23)
    property string translation2: qsTranslate("Context", "Hello", "disambiguation")
    property string idTranslation: qsTrId("Hello_id")
    property string translationInExpression: qsTr("Hello", "disambiguation", 23) + "World!"
    property string translation2InExpression: qsTranslate("Context", "Hello", "disambiguation") + "World!"
    property string idTranslationInExpression: qsTrId("Hello_id") + "World!"
}
