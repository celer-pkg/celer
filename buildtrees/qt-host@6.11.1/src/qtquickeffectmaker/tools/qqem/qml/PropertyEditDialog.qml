// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

CustomPopup {
    id: rootItem

    property int propertyIndex: -1
    readonly property bool isNewProperty: (propertyIndex == -1)
    property variant defaultValue
    property variant minValue
    property variant maxValue
    property int initialType: 2
    property variant initialDefaultValue
    property variant initialMinValue
    property variant initialMaxValue
    property string initialName
    property string initialDescription
    property string initialCustomValue
    property bool initialUseCustomValue: false
    property bool initialEnableMipmap: false // For image properties
    property bool enableMipmap: false // For image properties
    property bool initialExportImage: true
    property bool exportImage: true
    property int selectedNodeId: nodeViewItem.selectedNodeId
    readonly property real componentSpacing: 10

    function insertUniform() {
        let newRow = -1
        if (effectManager.uniformModel.updateRow(selectedNodeId, newRow, typeComboBox.currentIndex, uniformNameTextInput.text,
                                                 rootItem.defaultValue, descriptionTextArea.text, customValueTextArea.text,
                                                 customValueSwitch.checked, rootItem.minValue, rootItem.maxValue, rootItem.enableMipmap,
                                                 rootItem.exportImage)) {
            rootItem.close();
        }
    }

    function updateUniform() {
        if (effectManager.uniformModel.updateRow(selectedNodeId, rootItem.propertyIndex, typeComboBox.currentIndex, uniformNameTextInput.text,
                                                 rootItem.defaultValue, descriptionTextArea.text, customValueTextArea.text,
                                                 customValueSwitch.checked, rootItem.minValue, rootItem.maxValue, rootItem.enableMipmap,
                                                 rootItem.exportImage)) {
            rootItem.close();
        }
    }

    function removeUniform() {
        effectManager.uniformModel.removeRow(propertyIndex, 1);
        rootItem.close();
    }

    function roundedNumber(num) {
        return num.toFixed(3).toString();
    }

    x: mainWindow.width / 2 - width / 2
    y: mainWindow.height / 2 - height / 2
    width: 540
    height: 640
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    onVisibleChanged: {
        if (visible) {
            typeComboBox.currentIndex = initialType;
            rootItem.enableMipmap = rootItem.initialEnableMipmap;
            rootItem.exportImage = rootItem.initialExportImage;
            if (initialType == 7) {
                // When editing an image property, update the correct image
                // preview & mipmap status
                valueItemLoader.item.loadSource(initialDefaultValue, initialEnableMipmap, initialExportImage);
            }
            uniformNameTextInput.text = initialName;
            descriptionTextArea.text = initialDescription;
            customValueTextArea.text = initialCustomValue;
            customValueSwitch.checked = initialUseCustomValue;
        }
    }

    CustomDialog {
        id: removeUniformDialog
        title: qsTr("Delete Selected Property")
        x: rootItem.width / 2 - width / 2
        y: rootItem.height / 2 - height / 2
        width: 320
        height: 200
        modal: true
        focus: true
        standardButtons: Dialog.Yes | Dialog.No
        closePolicy: Popup.NoAutoClose

        Text {
            width: parent.width
            text: "Are you sure you want to delete the property <b>" + uniformNameTextInput.text + "</b>?"
            font.pixelSize: 14
            color: mainView.foregroundColor2
            wrapMode: Text.WordWrap
        }
        onAccepted: {
            removeUniform();
        }
    }

    ColorDialog {
        id: colorPickerDialog
        options: ColorDialog.ShowAlphaChannel
        onAccepted: {
            var cItem = valueItemLoader.item;
            if (cItem)
                cItem.updateFromPicker(selectedColor);
        }
    }

    component ValueLabelComponent : Text {
        color: mainView.foregroundColor2
        font.pixelSize: 14
        anchors.verticalCenter: parent.verticalCenter
        width: text === "" ? 0 : 50
    }

    component SingleFloatComponent : Item {
        id: singleFloatComponent
        property string name: ""
        property alias text: textItem.text
        width: nameItem.width + textItem.width + 10
        height: 40
        Text {
            id: nameItem
            text: name
            color: mainView.foregroundColor2
            font.pixelSize: 14
            anchors.verticalCenter: parent.verticalCenter
        }
        CustomTextField {
            id: textItem
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: nameItem.right
            anchors.leftMargin: 5
            width: 50
            text: "0"
        }
    }

    Component {
        id: boolComponent
        CheckBox {
            checked: initialDefaultValue ? Boolean(initialDefaultValue) : false
            onToggled: {
                rootItem.defaultValue = checked;
            }
        }
    }

    Component {
        id: floatComponent
        Column {
            spacing: 10
            Row {
                spacing: rootItem.componentSpacing
                ValueLabelComponent {
                    text: "default"
                }
                SingleFloatComponent {
                    id: defValue
                    name: ""
                    text: initialDefaultValue ? initialDefaultValue.toString() : "0.0"
                    onTextChanged: {
                        rootItem.defaultValue = Number(text);
                    }
                }
            }
            Row {
                spacing: rootItem.componentSpacing
                ValueLabelComponent {
                    text: "min"
                }
                SingleFloatComponent {
                    id: minValue
                    name: ""
                    text: initialMinValue ? initialMinValue.toString() : "0.0"
                    onTextChanged: {
                        rootItem.minValue = Number(text);
                    }
                }
            }
            Row {
                spacing: rootItem.componentSpacing
                ValueLabelComponent {
                    text: "max"
                }
                SingleFloatComponent {
                    id: maxValue
                    name: ""
                    text: initialMaxValue ? initialMaxValue.toString() : "1.0"
                    onTextChanged: {
                        rootItem.maxValue = Number(text);
                    }
                }
            }
        }
    }

    Component {
        id: vec2Component
        Column {
            function updateDefaultValue() {
                rootItem.defaultValue = Qt.vector2d(xDefaultValue.text, yDefaultValue.text);
            }
            function updateMinValue() {
                rootItem.minValue = Qt.vector2d(xMinValue.text, yMinValue.text);
            }
            function updateMaxValue() {
                rootItem.maxValue = Qt.vector2d(xMaxValue.text, yMaxValue.text);
            }
            spacing: 10
            Row {
                spacing: rootItem.componentSpacing
                ValueLabelComponent {
                    text: "default"
                }
                SingleFloatComponent {
                    id: xDefaultValue
                    name: "x:"
                    text: initialDefaultValue && initialDefaultValue.x ? roundedNumber(initialDefaultValue.x) : "0.0"
                    onTextChanged: {
                        updateDefaultValue();
                    }
                }
                SingleFloatComponent {
                    id: yDefaultValue
                    name: "y:"
                    text: initialDefaultValue && initialDefaultValue.y ? roundedNumber(initialDefaultValue.y) : "0.0"
                    onTextChanged: {
                        updateDefaultValue();
                    }
                }
            }
            Row {
                spacing: rootItem.componentSpacing
                ValueLabelComponent {
                    text: "min"
                }
                SingleFloatComponent {
                    id: xMinValue
                    name: "x:"
                    text: initialMinValue && initialMinValue.x ? roundedNumber(initialMinValue.x) : "0.0"
                    onTextChanged: {
                        updateMinValue();
                    }
                }
                SingleFloatComponent {
                    id: yMinValue
                    name: "y:"
                    text: initialMinValue && initialMinValue.y ? roundedNumber(initialMinValue.y) : "0.0"
                    onTextChanged: {
                        updateMinValue();
                    }
                }
            }
            Row {
                spacing: rootItem.componentSpacing
                ValueLabelComponent {
                    text: "max"
                }
                SingleFloatComponent {
                    id: xMaxValue
                    name: "x:"
                    text: initialMaxValue && initialMaxValue.x ? roundedNumber(initialMaxValue.x) : "1.0"
                    onTextChanged: {
                        updateMaxValue();
                    }
                }
                SingleFloatComponent {
                    id: yMaxValue
                    name: "y:"
                    text: initialMaxValue && initialMaxValue.y ? roundedNumber(initialMaxValue.y) : "1.0"
                    onTextChanged: {
                        updateMaxValue();
                    }
                }
            }
        }
    }

    Component {
        id: vec3Component
        Column {
            function updateDefaultValue() {
                rootItem.defaultValue = Qt.vector3d(xDefaultValue.text, yDefaultValue.text, zDefaultValue.text);
            }
            function updateMinValue() {
                rootItem.minValue = Qt.vector3d(xMinValue.text, yMinValue.text, zMinValue.text);
            }
            function updateMaxValue() {
                rootItem.maxValue = Qt.vector3d(xMaxValue.text, yMaxValue.text, zMaxValue.text);
            }
            spacing: 10
            Row {
                spacing: rootItem.componentSpacing
                ValueLabelComponent {
                    text: "default"
                }
                SingleFloatComponent {
                    id: xDefaultValue
                    name: "x:"
                    text: initialDefaultValue && initialDefaultValue.x ? roundedNumber(initialDefaultValue.x) : "0.0"
                    onTextChanged: {
                        updateDefaultValue();
                    }
                }
                SingleFloatComponent {
                    id: yDefaultValue
                    name: "y:"
                    text: initialDefaultValue && initialDefaultValue.y ? roundedNumber(initialDefaultValue.y) : "0.0"
                    onTextChanged: {
                        updateDefaultValue();
                    }
                }
                SingleFloatComponent {
                    id: zDefaultValue
                    name: "z:"
                    text: initialDefaultValue && initialDefaultValue.z ? roundedNumber(initialDefaultValue.z) : "0.0"
                    onTextChanged: {
                        updateDefaultValue();
                    }
                }
            }
            Row {
                spacing: rootItem.componentSpacing
                ValueLabelComponent {
                    text: "min"
                }
                SingleFloatComponent {
                    id: xMinValue
                    name: "x:"
                    text: initialMinValue && initialMinValue.x ? roundedNumber(initialMinValue.x) : "0.0"
                    onTextChanged: {
                        updateMinValue();
                    }
                }
                SingleFloatComponent {
                    id: yMinValue
                    name: "y:"
                    text: initialMinValue && initialMinValue.y ? roundedNumber(initialMinValue.y) : "0.0"
                    onTextChanged: {
                        updateMinValue();
                    }
                }
                SingleFloatComponent {
                    id: zMinValue
                    name: "z:"
                    text: initialMinValue && initialMinValue.z ? roundedNumber(initialMinValue.z) : "0.0"
                    onTextChanged: {
                        updateMinValue();
                    }
                }
            }
            Row {
                spacing: rootItem.componentSpacing
                ValueLabelComponent {
                    text: "max"
                }
                SingleFloatComponent {
                    id: xMaxValue
                    name: "x:"
                    text: initialMaxValue && initialMaxValue.x ? roundedNumber(initialMaxValue.x) : "1.0"
                    onTextChanged: {
                        updateMaxValue();
                    }
                }
                SingleFloatComponent {
                    id: yMaxValue
                    name: "y:"
                    text: initialMaxValue && initialMaxValue.y ? roundedNumber(initialMaxValue.y) : "1.0"
                    onTextChanged: {
                        updateMaxValue();
                    }
                }
                SingleFloatComponent {
                    id: zMaxValue
                    name: "z:"
                    text: initialMaxValue && initialMaxValue.z ? roundedNumber(initialMaxValue.z) : "1.0"
                    onTextChanged: {
                        updateMaxValue();
                    }
                }
            }
        }
    }

    Component {
        id: vec4Component
        Column {
            function updateDefaultValue() {
                rootItem.defaultValue = Qt.vector4d(xDefaultValue.text, yDefaultValue.text, zDefaultValue.text, wDefaultValue.text);
            }
            function updateMinValue() {
                rootItem.minValue = Qt.vector4d(xMinValue.text, yMinValue.text, zMinValue.text, wMinValue.text);
            }
            function updateMaxValue() {
                rootItem.maxValue = Qt.vector4d(xMaxValue.text, yMaxValue.text, zMaxValue.text, wMaxValue.text);
            }
            spacing: 10
            Row {
                spacing: rootItem.componentSpacing
                ValueLabelComponent {
                    text: "default"
                }
                SingleFloatComponent {
                    id: xDefaultValue
                    name: "x:"
                    text: initialDefaultValue && initialDefaultValue.x ? roundedNumber(initialDefaultValue.x) : "0.0"
                    onTextChanged: {
                        updateDefaultValue();
                    }
                }
                SingleFloatComponent {
                    id: yDefaultValue
                    name: "y:"
                    text: initialDefaultValue && initialDefaultValue.y ? roundedNumber(initialDefaultValue.y) : "0.0"
                    onTextChanged: {
                        updateDefaultValue();
                    }
                }
                SingleFloatComponent {
                    id: zDefaultValue
                    name: "z:"
                    text: initialDefaultValue && initialDefaultValue.z ? roundedNumber(initialDefaultValue.z) : "0.0"
                    onTextChanged: {
                        updateDefaultValue();
                    }
                }
                SingleFloatComponent {
                    id: wDefaultValue
                    name: "w:"
                    text: initialDefaultValue && initialDefaultValue.w ? roundedNumber(initialDefaultValue.w) : "0.0"
                    onTextChanged: {
                        updateDefaultValue();
                    }
                }
            }
            Row {
                spacing: rootItem.componentSpacing
                ValueLabelComponent {
                    text: "min"
                }
                SingleFloatComponent {
                    id: xMinValue
                    name: "x:"
                    text: initialMinValue && initialMinValue.x ? roundedNumber(initialMinValue.x) : "0.0"
                    onTextChanged: {
                        updateMinValue();
                    }
                }
                SingleFloatComponent {
                    id: yMinValue
                    name: "y:"
                    text: initialMinValue && initialMinValue.y ? roundedNumber(initialMinValue.y) : "0.0"
                    onTextChanged: {
                        updateMinValue();
                    }
                }
                SingleFloatComponent {
                    id: zMinValue
                    name: "z:"
                    text: initialMinValue && initialMinValue.z ? roundedNumber(initialMinValue.z) : "0.0"
                    onTextChanged: {
                        updateMinValue();
                    }
                }
                SingleFloatComponent {
                    id: wMinValue
                    name: "w:"
                    text: initialMinValue && initialMinValue.w ? roundedNumber(initialMinValue.w) : "0.0"
                    onTextChanged: {
                        updateMinValue();
                    }
                }
            }
            Row {
                spacing: rootItem.componentSpacing
                ValueLabelComponent {
                    text: "max"
                }
                SingleFloatComponent {
                    id: xMaxValue
                    name: "x:"
                    text: initialMaxValue && initialMaxValue.x ? roundedNumber(initialMaxValue.x) : "1.0"
                    onTextChanged: {
                        updateMaxValue();
                    }
                }
                SingleFloatComponent {
                    id: yMaxValue
                    name: "y:"
                    text: initialMaxValue && initialMaxValue.y ? roundedNumber(initialMaxValue.y) : "1.0"
                    onTextChanged: {
                        updateMaxValue();
                    }
                }
                SingleFloatComponent {
                    id: zMaxValue
                    name: "z:"
                    text: initialMaxValue && initialMaxValue.z ? roundedNumber(initialMaxValue.z) : "1.0"
                    onTextChanged: {
                        updateMaxValue();
                    }
                }
                SingleFloatComponent {
                    id: wMaxValue
                    name: "w:"
                    text: initialMaxValue && initialMaxValue.w ? roundedNumber(initialMaxValue.w) : "1.0"
                    onTextChanged: {
                        updateMaxValue();
                    }
                }
            }
        }
    }

    Component {
        id: colorComponent
        Column {
            function updateDefaultValue() {
                rootItem.defaultValue = Qt.rgba(xDefaultValue.text, yDefaultValue.text, zDefaultValue.text, wDefaultValue.text);
                rootItem.minValue = Qt.rgba(0.0, 0.0, 0.0, 0.0);
                rootItem.maxValue = Qt.rgba(1.0, 1.0, 1.0, 1.0);
            }
            function updateFromPicker(newColor) {
                xDefaultValue.text = (newColor.r).toFixed(3);
                yDefaultValue.text = (newColor.g).toFixed(3);
                zDefaultValue.text = (newColor.b).toFixed(3);
                wDefaultValue.text = (newColor.a).toFixed(3);
            }

            spacing: 10
            Row {
                spacing: rootItem.componentSpacing
                ValueLabelComponent {
                    text: "default"
                }
                SingleFloatComponent {
                    id: xDefaultValue
                    name: "r:"
                    text: initialDefaultValue && initialDefaultValue.r ? roundedNumber(initialDefaultValue.r) : "0.0"
                    onTextChanged: {
                        updateDefaultValue();
                    }
                }
                SingleFloatComponent {
                    id: yDefaultValue
                    name: "g:"
                    text: initialDefaultValue && initialDefaultValue.g ? roundedNumber(initialDefaultValue.g) : "0.0"
                    onTextChanged: {
                        updateDefaultValue();
                    }
                }
                SingleFloatComponent {
                    id: zDefaultValue
                    name: "b:"
                    text: initialDefaultValue && initialDefaultValue.b ? roundedNumber(initialDefaultValue.b) : "0.0"
                    onTextChanged: {
                        updateDefaultValue();
                    }
                }
                SingleFloatComponent {
                    id: wDefaultValue
                    name: "a:"
                    text: initialDefaultValue && initialDefaultValue.a ? roundedNumber(initialDefaultValue.a) : "1.0"
                    onTextChanged: {
                        updateDefaultValue();
                    }
                }
            }
            Rectangle {
                width: 100
                height: 40
                border.width: 1
                border.color: foregroundColor2
                color: rootItem.defaultValue
                Text {
                    anchors.centerIn: parent
                    text: "PICK"
                    font.pixelSize: 12
                    color: mainView.foregroundColor2
                    style: Text.Outline
                    styleColor: mainView.backgroundColor1
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        colorPickerDialog.selectedColor = rootItem.defaultValue;
                        colorPickerDialog.open();
                    }
                }
            }
        }
    }

    Component {
        id: imageComponent
        Column {
            function loadSource(sourceImage, mipmapEnabled, exportImage) {
                previewImage.source = sourceImage ? sourceImage : "";
                mipmapCheckBox.checked = mipmapEnabled;
                exportImageCheckBox.checked = exportImage;
            }
            spacing: -10
            Rectangle {
                width: 180
                height: 90
                border.width: 1
                color: mainView.backgroundColor2
                border.color: mainView.foregroundColor1
                Image {
                    id: previewImage
                    anchors.fill: parent
                    anchors.margins: 1
                    sourceSize: Qt.size(width, height);
                    fillMode: Image.PreserveAspectFit
                }
                CustomIconButton {
                    width: 30
                    height: width
                    anchors.left: parent.left
                    anchors.top: parent.top
                    icon: "images/icon_remove_shadow.png"
                    visible: previewImage.source !== ""
                    onClicked: {
                        rootItem.defaultValue = "";
                        previewImage.source = "";
                    }
                }

                Button {
                    anchors.centerIn: parent
                    text: "Choose Image"
                    onClicked: {
                        if (initialDefaultValue) {
                            textureSourceDialog.currentFolder = effectManager.getDirectory(initialDefaultValue);
                            textureSourceDialog.selectedFile = effectManager.addFileToURL(initialDefaultValue);
                        } else {
                            textureSourceDialog.currentFolder = effectManager.getDefaultImagesDirectory();
                        }
                        textureSourceDialog.open()
                    }
                }
            }
            CheckBox {
                id: mipmapCheckBox
                text: "Enable mipmaps"
                checked: rootItem.initialEnableMipmap
                onToggled: {
                    rootItem.enableMipmap = checked;
                    if (initialName !== "") {
                        var mipmapPropertyName = effectManager.mipmapPropertyName(initialName);
                        g_propertyData[mipmapPropertyName] = checked;
                    }
                }
            }
            CheckBox {
                id: exportImageCheckBox
                text: "Create QML Image element"
                checked: rootItem.initialExportImage
                onToggled: {
                    rootItem.exportImage = checked;
                }
            }
            FileDialog {
                id: textureSourceDialog
                title: "Open an Image File"
                nameFilters: [ effectManager.getSupportedImageFormatsFilter() ]
                onAccepted: {
                    if (textureSourceDialog.selectedFile !== null) {
                        rootItem.defaultValue = textureSourceDialog.selectedFile
                        previewImage.source = textureSourceDialog.selectedFile
                    }
                }
            }
        }
    }

    Component {
        id: defineComponent
        Column {
            CustomTextField {
                width: 100
                text: initialDefaultValue ? initialDefaultValue.toString() : "0"
                onTextChanged: {
                    rootItem.defaultValue = Number(text);
                }
            }
        }
    }

    Item  {
        id: propertiesDialogView
        anchors.top: parent.top
        anchors.bottom: dialogButtons.top
        width: parent.width
        Text {
            id: dialogTitle
            anchors.horizontalCenter: parent.horizontalCenter
            text: isNewProperty ? "Add Property" : "Edit Property"
            font.pixelSize: 16
            color: mainView.foregroundColor2
        }
        GridLayout {
            anchors.top: dialogTitle.bottom
            anchors.topMargin: 10
            columns: 3
            Label {
                text: "Type:"
                Layout.row: 0
                Layout.column: 0
                Layout.preferredWidth: 100
                color: mainView.foregroundColor2
            }
            ComboBox {
                id: typeComboBox
                currentIndex: initialType
                Layout.preferredHeight: 40
                Layout.row: 0
                Layout.column: 1
                model: [
                    "bool",
                    "int",
                    "float",
                    "vec2",
                    "vec3",
                    "vec4",
                    "color",
                    "image",
                    "define"
                ]
                onCurrentIndexChanged: {
                    if (currentIndex === 8) {
                        customValueSwitch.checked = false;
                    }
                }
            }
            Label {
                text: "Name:"
                Layout.row: 1
                Layout.column: 0
                Layout.preferredWidth: 100
                color: mainView.foregroundColor2
            }
            CustomTextField {
                id: uniformNameTextInput
                RegularExpressionValidator {
                    id: propertyNameValidator
                    regularExpression: /[a-z_][a-zA-Z0-9_]+/
                }
                RegularExpressionValidator {
                    id: defineNameValidator
                    regularExpression: /[a-zA-Z_][a-zA-Z0-9_]+/
                }
                // Properties must start with lowercase letter, defines can be ALL_CAPS
                validator: (typeComboBox.currentIndex === 8) ? defineNameValidator : propertyNameValidator
                placeholderText: "propertyName"
                text: initialName ? initialName.toString() : ""
                onAccepted: rootItem.insertUniform()
                Layout.row: 1
                Layout.column: 1
                Layout.preferredWidth: 300
            }
            Label {
                text: "Custom:"
                Layout.row: 2
                Layout.column: 0
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: 10
                Layout.preferredWidth: 100
                color: mainView.foregroundColor2
            }
            CheckBox {
                id: customValueSwitch
                Layout.row: 2
                Layout.column: 1
                checked: initialUseCustomValue
                // Defines are already custom
                enabled: (typeComboBox.currentIndex !== 8)
            }
            Label {
                text: "Value:"
                Layout.row: 3
                Layout.column: 0
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: 10
                Layout.preferredWidth: 100
                color: mainView.foregroundColor2
            }
            CustomTextEdit {
                id: customValueTextArea
                Layout.row: 3
                Layout.column: 1
                Layout.columnSpan: 2
                Layout.preferredWidth: 380
                Layout.preferredHeight: 160
                visible: customValueSwitch.checked
                text: initialCustomValue ? initialCustomValue.toString() : ""
            }
            Loader {
                id: valueItemLoader
                Layout.row: 3
                Layout.column: 1
                Layout.preferredHeight: 160
                visible: !customValueTextArea.visible
                sourceComponent: {
                    if (typeComboBox.currentIndex === 0)
                        return boolComponent;
                    if (typeComboBox.currentIndex === 1 || typeComboBox.currentIndex === 2)
                        return floatComponent;
                    if (typeComboBox.currentIndex === 3)
                        return vec2Component;
                    if (typeComboBox.currentIndex === 4)
                        return vec3Component;
                    if (typeComboBox.currentIndex === 5)
                        return vec4Component;
                    if (typeComboBox.currentIndex === 6)
                        return colorComponent;
                    if (typeComboBox.currentIndex === 7)
                        return imageComponent;
                    if (typeComboBox.currentIndex === 8)
                        return defineComponent;
                    return null;
                }
            }
            Label {
                text: "Description:"
                Layout.row: 4
                Layout.column: 0
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 100
                color: mainView.foregroundColor2
            }
            CustomTextEdit {
                id: descriptionTextArea
                Layout.row: 4
                Layout.column: 1
                Layout.columnSpan: 2
                Layout.preferredWidth: 380
                Layout.preferredHeight: 200
                text: initialDescription ? initialDescription.toString() : ""
            }
        }
    }

    Item {
        id: dialogButtons
        anchors.bottom: parent.bottom
        width: parent.width
        height: 60
        Button {
            id: addButton
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            text: isNewProperty ? "Add" : "OK"
            enabled: uniformNameTextInput.text !== ""
            onClicked: {
                if (isNewProperty) {
                    rootItem.insertUniform();
                } else {
                    rootItem.updateUniform();
                }
            }
        }
        Button {
            id: removeButton
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            text: "Remove"
            visible: !isNewProperty
            onClicked: {
                removeUniformDialog.open();
            }
        }
        Button {
            id: cancelButton
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            text: "Cancel"
            onClicked: {
                if ((typeComboBox.currentIndex === 7) && initialName !== "") {
                    // For image properties return the initial mipmap value
                    var mipmapPropertyName = effectManager.mipmapPropertyName(initialName);
                    g_propertyData[mipmapPropertyName] = initialEnableMipmap;
                }
                rootItem.close();
            }
        }
    }
}
