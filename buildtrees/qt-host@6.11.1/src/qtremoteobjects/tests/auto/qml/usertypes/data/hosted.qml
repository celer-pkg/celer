import QtQuick 2.0
import QtRemoteObjects 5.15
import usertypes 1.0

Item {
    SimpleClockSimpleSource {
        id: clock
    }

    Host {
        property string local_socket: Qt.platform.os == "android" ? "localabstract" : "local"
        hostUrl: local_socket + ":testHost"
        Component.onCompleted: enableRemoting(clock)
    }

    Timer {
        interval: 500
        running: true
        onTriggered: clock.timeUpdated(10, 30)
    }
}
