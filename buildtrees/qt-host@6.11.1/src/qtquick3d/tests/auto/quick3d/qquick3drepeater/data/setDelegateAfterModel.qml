import QtQml
import QtQuick3D

Node {
    property bool useObjectModel: false

    ObjectModel {
        id: objectModel
        Node {}
        Node {}
    }

    DelegateModel {
        id: delegateModel
        model: [1, 2, 3]
    }

    Repeater3D {
        id: view
        model: useObjectModel ? objectModel : delegateModel
    }

    Component {
        id: delegate
        Node {
            x: 100
            y: 100
        }
    }

    Component {
        id: delegate2
        Node {
            x: 200
            y: 200
        }
    }

    property int count: view.count

    // Set the delegate after the model
    Component.onCompleted: view.delegate = delegate

    function plantDelegate() {
        view.model = [1, 2, 3, 4]
        view.delegate = delegate
        delegateModel.delegate = delegate2
    }
}
