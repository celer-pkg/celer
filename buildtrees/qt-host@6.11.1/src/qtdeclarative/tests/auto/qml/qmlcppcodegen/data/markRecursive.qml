import QtQml
import TestTypes

Timer {
    objectName: currentObject?.sub.name ?? "0"

    property RecursiveObject recursiveObject: RecursiveObject {}

    property var currentObject
    onCurrentObjectChanged: gc()
    function setObject(obj: var): void { currentObject = obj; }

    interval: 1
    repeat: true
    running: true
    onTriggered: setObject(recursiveObject.getObject());
}
