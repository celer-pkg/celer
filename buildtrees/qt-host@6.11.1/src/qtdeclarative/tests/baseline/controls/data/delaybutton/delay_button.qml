import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl

import "../shared"

FlowPane {
    DelayButton {
        text: "Delay 3000"
        delay: 3000
        progress: 0.7
    }

    DelayButton {
        text: "Delay null"
        delay: 0
        progress: 0
    }

    DelayButton {
        text: "Default"
    }

    DelayButton {
        text: "Delay 0"
        delay: 0
        progress: 0.1
    }

    DelayButton {
        enabled: false
        text: "Delay 500"
        delay: 500
        progress: 1.0
    }
    DelayButton {
        text: "Delay 1000"
        delay: 1000
        down: true
        progress: 0.0
    }

    DelayButton {
        text: "Style's icon color"
        icon.source: "../shared/heart.svg"
        visible: contentItem instanceof IconLabel
    }

    DelayButton {
        text: "Style's icon color (action)"
        visible: contentItem instanceof IconLabel
        action: Action {
            icon.source: "../shared/heart.svg"
        }
    }

    DelayButton {
        text: "Green icon color"
        icon.source: "../shared/heart.svg"
        icon.color: "green"
        visible: contentItem instanceof IconLabel
    }

    DelayButton {
        text: "Green icon color (action)"
        visible: contentItem instanceof IconLabel
        action: Action {
            icon.source: "../shared/heart.svg"
            icon.color: "green"
        }
    }

    DelayButton {
        text: "Original icon color"
        icon.source: "../shared/heart.svg"
        icon.color: "transparent"
        visible: contentItem instanceof IconLabel
    }

    DelayButton {
        text: "Original icon color (action)"
        visible: contentItem instanceof IconLabel
        action: Action {
            icon.source: "../shared/heart.svg"
            icon.color: "transparent"
        }
    }
}
