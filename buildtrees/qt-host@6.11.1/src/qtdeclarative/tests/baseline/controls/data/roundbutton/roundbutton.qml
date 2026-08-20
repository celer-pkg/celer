import QtQuick
import QtQuick.Controls

import "../shared"

FlowPane {
    RoundButton {
    }

    RoundButton {
        text: qsTr("2")
        enabled: false
    }

    RoundButton {
        text: qsTr("3")
        down: true
    }

    RoundButton {
        text: qsTr("4")
        checked: true
    }

    RoundButton {
        text: qsTr("5")
        checkable: true
    }

    RoundButton {
        text: qsTr("6")
        focus: true
    }

    RoundButton {
        text: qsTr("7")
        highlighted: true
    }

    RoundButton {
        text: qsTr("8")
        flat: true
    }

    RoundButton {
        text: qsTr("9")
        LayoutMirroring.enabled: true
    }

    // Style's icon color
    RoundButton {
        icon.source: "../shared/heart.svg"
    }

    // Style's icon color (action)
    RoundButton {
        action: Action {
            icon.source: "../shared/heart.svg"
        }
    }

    // Green icon color
    RoundButton {
        icon.source: "../shared/heart.svg"
        icon.color: "green"
    }

    // Green icon color (action)
    RoundButton {
        action: Action {
            icon.source: "../shared/heart.svg"
            icon.color: "green"
        }
    }

    // Original icon color
    RoundButton {
        icon.source: "../shared/heart.svg"
        icon.color: "transparent"
    }

    // Original icon color (action)
    RoundButton {
        action: Action {
            icon.source: "../shared/heart.svg"
            icon.color: "transparent"
        }
    }
}
