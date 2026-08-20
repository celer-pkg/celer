import QtQuick

Item {
    id: rootRectangle
    width: 500
    height: 500

    readonly property color myBlue: "#1010FF"
    readonly property color myRed: "#ff1010"
    property string result: ""

    property var themes: {
        "blueTheme": {
            bgColor: rootRectangle.myBlue
        },
        "redTheme": {
            bgColor: rootRectangle.myRed
        }
    }

    Component.onCompleted: {
        result = JSON.stringify(rootRectangle.themes)
    }
}

