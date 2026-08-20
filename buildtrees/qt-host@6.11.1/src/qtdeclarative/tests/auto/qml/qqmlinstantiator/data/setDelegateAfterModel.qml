import QtQml
QtObject {
    property bool useObjectModel: false
    property ObjectModel objectModel: ObjectModel {
        id: objectModel
        QtObject {}
        QtObject {}
    }
    property ListModel listModel: ListModel {
        id: listModel
        ListElement { n: 1 }
        ListElement { n: 2 }
        ListElement { n: 3 }
    }
    property Instantiator view: Instantiator {
        id: view
        model: useObjectModel ? objectModel : listModel
    }
    property Component delegate: Component {
        id: delegate
        QtObject {
            objectName: "green"
        }
    }
    property int count: view.count
    // Set the delegate after the model
    Component.onCompleted: view.delegate = delegate
}
