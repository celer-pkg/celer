import QtQuick 2.0

Item {
    property int signalCounter: 0
    property list<QtObject> sourceList: [ QtObject{}, QtObject{}, QtObject{} ]
    property list<QtObject> targetList1: sourceList

    onTargetList1Changed: signalCounter++

    function assignList() {
        targetList1 = sourceList
    }

    function assignList2() {
        targetList1 = [sourceList[2], sourceList[1], sourceList[0]]
    }
}
