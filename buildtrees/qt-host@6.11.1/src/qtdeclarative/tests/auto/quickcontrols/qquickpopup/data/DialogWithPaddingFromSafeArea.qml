import QtQuick
import QtQuick.Controls

Window {
    width: 600
    height: 600
    flags: Qt.ExpandedClientAreaHint | Qt.NoTitleBarBackgroundHint

    SafeArea.additionalMargins.top: 50
    Dialog {
        id: popup
        popupType: Popup.Window
        topPadding: SafeArea.margins.top
        width: 300
        height: 300
        modal: true
    }
}
