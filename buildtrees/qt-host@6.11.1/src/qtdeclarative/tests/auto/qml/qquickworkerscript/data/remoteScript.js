WorkerScript.onMessage = function(msg) {
    console.log("received")
    WorkerScript.sendMessage("remote script running")
}
