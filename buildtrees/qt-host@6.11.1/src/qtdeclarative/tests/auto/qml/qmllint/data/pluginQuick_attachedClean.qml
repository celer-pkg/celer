import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: root
    // QtQuick
    Accessible.name: "Foo"
    LayoutMirroring.enabled: true
    EnterKey.type: Qt.EnterKeyGo

    //QtQuick.Layouts
    Layout.minimumHeight: 3
    property bool stackLayout: StackLayout.isCurrentItem // Read-only

    // QtQuick.Templates
    SplitView.fillWidth: true
    StackView.visible: true
    property int swipeView: SwipeView.index // Read only, enforceable but complicated
    Flickable {
        TextArea.flickable: TextArea {}
        ScrollIndicator.vertical: ScrollIndicator {}
        ScrollBar.vertical: ScrollBar {}
    }
    ToolTip.delay: 50
    TableView {
        delegate: Item {
            property bool tumbler: Tumbler.displacement // Read only, enforceable but complicated
        }
    }
    property bool swipeDelegate: SwipeDelegate.pressed // Read only

    PropertyChanges {
      // inside custom parser, can't be sensibly handled
      target: root
      Layout.leftMargin: 10 // qmllint disable Quick.property-changes-parsed
    }
}
