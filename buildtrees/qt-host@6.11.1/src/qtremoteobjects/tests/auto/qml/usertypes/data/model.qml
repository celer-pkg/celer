import QtQuick 2.0
import QtRemoteObjects 5.12
import usertypes 1.0

TypeWithModelReplica {
    node: Node {
        property string local_socket: Qt.platform.os == "android" ? "localabstract" : "local"
        registryUrl: local_socket + ":testModel"
    }
}
