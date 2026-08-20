import QtQuick
import QtQuick.Controls.Basic

Item {
    id: root
    property StackView stackView

    Connections {
        function onClicked() {
            root.stackView.pop();
        }
    }
}
