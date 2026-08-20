import QtQuick
import QtQuick.Controls

import "../shared"

FlowPane {
    Repeater {
        model: ListModel {
            ListElement {
                text: qsTr("Option 1")
                down: true
                focus: false
                highlighted: false
                enabled: true
                checked: false
            }
            ListElement {
                text: qsTr("Option 2")
                down: false
                focus: true
                highlighted: false
                enabled: true
                checked: false
            }
            ListElement {
                text: qsTr("Option 3")
                down: false
                focus: false
                highlighted: true
                enabled: true
                checked: false
            }
            ListElement {
                text: qsTr("Option 4")
                down: false
                focus: false
                highlighted: false
                enabled: false
                checked: false
            }
            ListElement {
                text: qsTr("Option 5")
                down: false
                focus: false
                highlighted: false
                enabled: true
                checked: true
            }
            ListElement {
                text: qsTr("Option 6")
                down: false
                focus: false
                highlighted: false
                enabled: true
                checked: false
                tristate: false
                checkState: Qt.PartiallyChecked
            }
        }
        delegate: CheckDelegate {
            text: model.text
            down: model.down
            focus: model.focus
            highlighted: model.highlighted
            enabled: model.enabled
            checked: model.checked
        }
    }

    CheckDelegate {
        text: "Partially checked"
        tristate: true
        checkState: Qt.PartiallyChecked
    }

    CheckDelegate {
        text: "Style's icon color"
        icon.source: "../shared/heart.svg"
    }

    CheckDelegate {
        text: "Style's icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
        }
    }

    CheckDelegate {
        text: "Green icon color"
        icon.source: "../shared/heart.svg"
        icon.color: "green"
    }

    CheckDelegate {
        text: "Green icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
            icon.color: "green"
        }
    }
}
