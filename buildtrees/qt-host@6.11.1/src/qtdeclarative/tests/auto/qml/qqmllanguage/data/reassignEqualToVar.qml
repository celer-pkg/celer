import QtQml

QtObject {
    property int changes: 0
    property bool discriminator: false
    property var varVal: discriminator ? 100 : 100
    onVarValChanged: ++changes
}
