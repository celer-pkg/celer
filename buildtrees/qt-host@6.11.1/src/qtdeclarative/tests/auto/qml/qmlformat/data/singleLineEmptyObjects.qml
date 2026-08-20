import QtQuick

Item {
    id: test

    property var test: QtObject {

    }

    QtObject {
    }

    QtObject {

        property var test: QtObject {

            property int a

        }
    }
}
