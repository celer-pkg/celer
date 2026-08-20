import QtQuick

Text {
    property real explicitWidth: implicitWidth
    width: explicitWidth
    text: "Lorem ipsum sic dolor amet\nLorem ipsum dolor sit amet, consectetur adipiscing elit\nEtiam felis nisl, fringilla sed vestibulum a, pretium a massa"
    elide: Text.ElideRight
    maximumLineCount: 1
}
