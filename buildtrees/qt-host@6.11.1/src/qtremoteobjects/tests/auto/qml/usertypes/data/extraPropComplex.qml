import QtQuick 2.0
import QtRemoteObjects 5.12
import usertypes 1.0

ComplexTypeReplica {
    // test that the existence of this property doesn't cause issues
    property QtObject object: QtObject {}

    node: Node {
        property string local_socket: Qt.platform.os == "android" ? "localabstract" : "local"
        registryUrl: local_socket + ":testExtraComplex"
    }
}

