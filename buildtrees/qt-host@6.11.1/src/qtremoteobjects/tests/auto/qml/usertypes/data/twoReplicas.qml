import QtQuick 2.0
import QtRemoteObjects 5.12
import usertypes 1.0

Item {
    property int result: replica.hour
    property int result2: replica2.hour

    Node {
        id: sharedNode
        property string local_socket: Qt.platform.os == "android" ? "localabstract" : "local"
        registryUrl: local_socket + ":testTwoReplicas"
    }

    SimpleClockReplica {
        id: replica
        node: sharedNode
    }

    SimpleClockReplica {
        id: replica2
    }

    Timer {
        running: true
        interval: 200
        onTriggered: replica2.node = sharedNode
    }
}

