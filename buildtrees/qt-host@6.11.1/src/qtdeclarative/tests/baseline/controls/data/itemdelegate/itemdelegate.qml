import QtQuick.Controls

import "../shared"

FlowPane {
    ItemDelegate {
        text: "Style's icon color"
        icon.source: "../shared/heart.svg"
    }

    ItemDelegate {
        text: "Style's icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
        }
    }

    ItemDelegate {
        text: "Green icon color"
        icon.source: "../shared/heart.svg"
        icon.color: "green"
    }

    ItemDelegate {
        text: "Green icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
            icon.color: "green"
        }
    }

    ItemDelegate {
        text: "Original icon color"
        icon.source: "../shared/heart.svg"
        icon.color: "transparent"
    }

    ItemDelegate {
        text: "Original icon color (action)"
        action: Action {
            icon.source: "../shared/heart.svg"
            icon.color: "transparent"
        }
    }
}
