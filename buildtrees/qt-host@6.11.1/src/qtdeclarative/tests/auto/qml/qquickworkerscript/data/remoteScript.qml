import QtQml

QtObject {
    id: root
    property bool ready: script.ready
    property WorkerScript script: WorkerScript {
        source: "remoteScript.js"
        onMessage: function (messageObject) { root.objectName = messageObject }
        onReadyChanged: {
            if (ready)
                sendMessage(1);
        }
    }
}
