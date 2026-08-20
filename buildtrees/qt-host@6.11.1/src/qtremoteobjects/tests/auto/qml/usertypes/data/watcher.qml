import QtQuick 2.0
import QtRemoteObjects 5.14
import usertypes 1.0

TypeWithReplyReplica {
    property variant result
    property bool hasError

    node: Node {
        property string local_socket: Qt.platform.os == "android" ? "localabstract" : "local"
        registryUrl: local_socket + ":testWatcher"
    }
    onStateChanged: function(state) {
        if (state != TypeWithReplyReplica.Valid)
            return;
        QtRemoteObjects.watch(uppercase("hello"), 1000)
                       .then(function(value) { hasError = false; result = value },
                             function(error) { hasError = true })
    }

    function callSlowFunction() {
        result = 0
        hasError = false

        QtRemoteObjects.watch(slowFunction(), 300)
                       .then(function(value) { hasError = false; result = value },
                             function(error) { hasError = true })
    }

    function callComplexFunction() {
        result = null
        hasError = false

        QtRemoteObjects.watch(complexReturnType(), 300)
                       .then(function(value) { hasError = false; result = value },
                             function(error) { hasError = true })
    }
}
