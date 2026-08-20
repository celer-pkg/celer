pragma Strict
import QML
import TestTypes as TT2

QtObject {
    property Component c: Component {
        id: cc
        QtObject {}
    }
        
    property bool ready: cc.status == Component.Ready
    property int enumFromGadget2: TT2.GadgetWithEnum.CONNECTED + 1
}
