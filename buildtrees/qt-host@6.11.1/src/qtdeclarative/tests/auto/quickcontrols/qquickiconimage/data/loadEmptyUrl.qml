import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl

IconImage {
    name: "edit-copy"
    width: 32; height: 32

    // Setting fillMode will trigger load(), which shouldn't
    // end up calling loadEmptyUrl().
    fillMode: Image.PreserveAspectFit
}
