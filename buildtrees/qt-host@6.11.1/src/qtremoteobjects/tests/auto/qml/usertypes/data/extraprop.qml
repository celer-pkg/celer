import QtQuick 2.0
import QtRemoteObjects 5.12
import usertypes 1.0

SimpleClockReplica {
    property string result: hour // test that the existence of this property doesn't cause issues
    node: Node {
        property string local_socket: Qt.platform.os == "android" ? "localabstract" : "local"
        registryUrl: local_socket + ":test"
    }
    onStateChanged: if (state == SimpleClockReplica.Valid) pushHour(10)
}
