import QtQuick
import QtQuick.Controls

import "../shared"

FlowPane {
    Button {
        text: "Normal"
    }

    Button {
        text: "Focused"
        focus: true
    }

    Button {
        text: "Flat"
        flat: true
    }

    Button {
        text: "Highlighted"
        highlighted: true
    }

    Button {
        text: "Disabled"
        enabled: false
    }

    Button {
        text: "Down"
        down: true
    }

    Button {
        text: "Style's icon color"
        icon.source: "../shared/heart.svg"
    }

    Button {
        text: "Style's icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
        }
    }

    Button {
        text: "Green icon color"
        icon.source: "../shared/heart.svg"
        icon.color: "green"
    }

    Button {
        text: "Green icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
            icon.color: "green"
        }
    }

    Button {
        text: "Original icon color"
        icon.source: "../shared/heart.svg"
        icon.color: "transparent"
    }

    Button {
        text: "Original icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
            icon.color: "transparent"
        }
    }
}
