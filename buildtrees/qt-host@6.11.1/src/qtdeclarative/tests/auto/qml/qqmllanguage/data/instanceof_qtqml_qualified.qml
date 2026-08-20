import QtQml 2.0 as QmlImport

QmlImport.QtObject {
    id: qtobjectInstance

    property QmlImport.Timer aTimer: QmlImport.Timer {
        id: timerInstance
    }

    property QmlImport.Connections aConnections: QmlImport.Connections {
        id: connectionsInstance
    }

    property QmlImport.string string: "a"
    property QmlImport.url url: "http://example.com"
    property QmlImport.date date: new Date(1996, 2, 3)
}
