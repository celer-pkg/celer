import QtQuick

Item {
    width: 400
    height: 400
    Accessible.name: "root"
    Accessible.role: Accessible.Client

    Text {
        id: label
        objectName: "label"
        x: 100
        y: 40
        width: 100
        height: 40
        text : "This is a text input"
        Accessible.role: Accessible.StaticText
        Accessible.name: "Label for the text input field"
        Accessible.description: "A label"
        Accessible.labelFor: textInput
    }

    TextInput {
        id: textInput
        objectName: "textInput"
        x: 100
        y: 80
        width: 200
        height: 40
        Accessible.role: Accessible.EditableText
    }
}
