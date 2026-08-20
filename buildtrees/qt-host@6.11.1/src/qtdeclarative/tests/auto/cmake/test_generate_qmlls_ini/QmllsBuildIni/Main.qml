import QtQuick
import MyModule
import Hello.World.MyModule3

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    MyComponent {}
    MyComponent2 {} // note: imported via imports
    MyComponent3 {}
}
