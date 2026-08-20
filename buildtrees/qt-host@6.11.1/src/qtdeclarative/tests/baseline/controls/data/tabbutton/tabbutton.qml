import QtQuick
import QtQuick.Controls

import "../shared"

FlowPane {
    TabButton {
        text: qsTr("Button 1")
    }

    TabButton {
        text: qsTr("Button 2")
        enabled: false
    }

    TabButton {
        text: qsTr("Button 3")
        focus: true
    }

    TabButton {
        text: qsTr("Button 5")
        down: true
    }

    TabButton {
        text: qsTr("Button 6")
        checked: true
    }

    TabButton {
        text: qsTr("Button 7")
        LayoutMirroring.enabled: true
    }

    TabButton {
        text: "Style's icon color"
        icon.source: "../shared/heart.svg"
    }

    TabButton {
        text: "Style's icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
        }
    }

    TabButton {
        text: "Green icon color"
        icon.source: "../shared/heart.svg"
        icon.color: "green"
    }

    TabButton {
        text: "Green icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
            icon.color: "green"
        }
    }

    TabButton {
        text: "Original icon color"
        icon.source: "../shared/heart.svg"
        icon.color: "transparent"
    }

    TabButton {
        text: "Original icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
            icon.color: "transparent"
        }
    }
}
