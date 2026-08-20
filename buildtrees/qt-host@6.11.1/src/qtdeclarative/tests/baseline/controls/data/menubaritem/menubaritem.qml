import QtQuick.Controls

import "../shared"

FlowPane {
    MenuBarItem {
        text: "Style's icon color"
        icon.source: "../shared/heart.svg"
    }

    MenuBarItem {
        text: "Style's icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
        }
    }

    MenuBarItem {
        text: "Green icon color"
        icon.source: "../shared/heart.svg"
        icon.color: "green"
    }

    MenuBarItem {
        text: "Green icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
            icon.color: "green"
        }
    }

    MenuBarItem {
        text: "Original icon color"
        icon.source: "../shared/heart.svg"
        icon.color: "transparent"
    }

    MenuBarItem {
        text: "Original icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
            icon.color: "transparent"
        }
    }
}
