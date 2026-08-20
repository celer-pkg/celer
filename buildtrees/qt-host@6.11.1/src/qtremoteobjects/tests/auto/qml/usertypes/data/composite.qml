import QtQuick 2.0
import "../data" as MyComponents //for staric build resource path should be set

QtObject {
    property QtObject myTypeOk: MyComponents.MyType {} // this works
    property MyComponents.MyType myType: MyComponents.MyType {} // this crashes
    property MyComponents.MyType myType2 // this crashes (ensure solution works with null object)
}
