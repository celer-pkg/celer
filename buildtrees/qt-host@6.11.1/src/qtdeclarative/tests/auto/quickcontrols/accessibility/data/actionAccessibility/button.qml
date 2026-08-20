import QtQuick
import QtQuick.Controls

Button {
    id: button
    property int pressCount: 0

    action: Action {
        id: anAction
        text: "Peaches"
        Accessible.name: "Peach"
        Accessible.description: "Show peaches some love"
    }
    text: Accessible.description
    Accessible.onPressAction: button.pressCount += 1
}
