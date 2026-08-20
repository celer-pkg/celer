import QtQml

Instantiator {
    id: outer
    model: 5
    Instantiator {
        id: middle
        model: 5
        QtObject {
            id: inner
            objectName: "o"
        }
    }
}
