import QtQuick
import QtQuick.Controls

import "../shared"

FlowPane {
    Repeater {
        model: ListModel {
            ListElement {
                down: true
                focus: false
                highlighted: false
                enabled: true
                checked: false
            }
            ListElement {
                down: false
                focus: true
                highlighted: false
                enabled: true
                checked: false
            }
            ListElement {
                down: false
                focus: false
                highlighted: true
                enabled: true
                checked: false
            }
            ListElement {
                down: false
                focus: false
                highlighted: false
                enabled: false
                checked: false
            }
            ListElement {
                down: false
                focus: false
                highlighted: false
                enabled: true
                checked: true
            }
            ListElement {
                down: false
                focus: false
                highlighted: false
                enabled: true
                checked: false
            }
        }
        delegate: SwitchDelegate {
            down: model.down
            focus: model.focus
            highlighted: model.highlighted
            enabled: model.enabled
            checked: model.checked
        }
    }

    SwitchDelegate {
        text: "Style's icon color"
        icon.source: "../shared/heart.svg"
    }

    SwitchDelegate {
        text: "Style's icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
        }
    }

    SwitchDelegate {
        text: "Green icon color"
        icon.source: "../shared/heart.svg"
        icon.color: "green"
    }

    SwitchDelegate {
        text: "Green icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
            icon.color: "green"
        }
    }

    SwitchDelegate {
        text: "Original icon color"
        icon.source: "../shared/heart.svg"
        icon.color: "transparent"
    }

    SwitchDelegate {
        text: "Original icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
            icon.color: "transparent"
        }
    }
}
