import QtQuick


Item {
    property string myString
    property date myDateTime: new Date(myString)
    property real myTime: myDateTime.getTime()
    property bool isValid: !Number.isNaN(myDateTime)
}