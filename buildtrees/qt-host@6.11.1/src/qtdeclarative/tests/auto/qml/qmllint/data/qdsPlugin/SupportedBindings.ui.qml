import QtQuick

Item {
    Item {
        property int magic: parent.x // ok: accessing parent from non-root component
    }

    property int notInBlacklist
    property int zxcv: notInBlacklist -= 3 // this property is not in the blacklist
}
