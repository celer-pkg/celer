import QtQuick

Item {
    property bool useObjectModel: false

    ObjectModel {
        id: objectModel
        Item {}
        Item {}
    }

    DelegateModel {
        id: delegateModel
        model: [1, 2, 3]
    }

    Repeater {
        id: view
        anchors.fill: parent
        model: useObjectModel ? objectModel : delegateModel
    }

    Component {
        id: delegate
        Rectangle {
            width: 100
            height: 100
            color: "green"
        }
    }

    Component {
        id: delegate2
        Rectangle {
            width: 100
            height: 100
            color: "blue"
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
