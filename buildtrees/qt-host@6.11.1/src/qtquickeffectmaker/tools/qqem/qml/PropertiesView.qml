// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QQEMLib 1.0

Item {
    id: rootItem
    property alias uniformModel: uniformTable.model
    property bool showSettings: false
    property real showSettingsAnimated: showSettings

    function truncateString(str, n) {
        return (str.length > n) ? str.substr(0, n - 3) + '...' : str;
    }

    Behavior on showSettingsAnimated {
        NumberAnimation {
            duration: 500
            easing.type: Easing.InOutQuad
        }
    }

    FileDialog {
        id: textureSourceDialog

        property int modelIndex: -1

        title: "Select an Image File"
        nameFilters: [ effectManager.getSupportedImageFormatsFilter() ]
        onAccepted: {
            if (textureSourceDialog.selectedFile !== null) {
                effectManager.uniformModel.setImage(modelIndex, textureSourceDialog.selectedFile);
            }
        }
    }

    ColorDialog {
        id: colorPickerDialog
        property var propertyItem: null
        property color initialColor: "#000000"
        options: ColorDialog.ShowAlphaChannel

        onSelectedColorChanged: {
            if (propertyItem)
                propertyItem.updateValue(selectedColor);
        }
        onAccepted: {
            if (propertyItem)
                propertyItem.updateValue(selectedColor);
        }
        onRejected: {
            if (propertyItem)
                propertyItem.updateValue(initialColor);
        }
    }

    Rectangle {
        anchors.fill: parent
        color: mainView.backgroundColor1
    }

    ColumnLayout {
        anchors.fill: parent
        clip: true
        spacing: 0
        Item {
            Layout.fillWidth: true
            height: childrenRect.height + 10
            Row {
                id: columnsHeader
                Layout.fillWidth: true
                spacing: 10
                Item {
                    id: addPropertyButton
                    width: 10 + (1.0 - designModeAnimated) * uniformTable.columnWidth(0)
                    height: parent.height
                    opacity: (1.0 - designModeAnimated)
                    CustomIconButton {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        height: 24
                        width: height
                        icon: "images/icon_add.png"
                        enabled: !designMode && effectManager.nodeView.effectNodeSelected
                        description: "Add property"
                        onClicked: {
                            mainWindow.addPropertyAction();
                        }
                    }
                }
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    width: uniformTable.columnWidth(1)
                    text: "TYPE"
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 12
                    font.bold: true
                }
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    width: uniformTable.columnWidth(2)
                    text: "NAME"
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 12
                    font.bold: true
                }
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    width: uniformTable.columnWidth(3)
                    text: "VALUE"
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 12
                    font.bold: true
                }
                Item {
                    width: uniformTable.columnWidth(4)
                    height: 1
                }
                CustomIconButton {
                    anchors.verticalCenter: parent.verticalCenter
                    height: 24
                    width: height
                    icon: "images/icon_settings.png"
                    toggledIcon: "images/icon_settings_on.png"
                    toggleButton: true
                    toggled: showSettings
                    description: "Show / hide property settings"
                    onClicked: {
                        showSettings = !showSettings;
                    }
                }
            }
        }
        Rectangle {
            Layout.fillWidth: true
            height: 1
            z: -1
            color: mainView.backgroundColor2
        }
        ListView {
            id: uniformTable
            Layout.fillHeight: true
            Layout.fillWidth: true
            flickableDirection: Flickable.VerticalFlick
            model: effectManager.uniformModel
            clip: true
            cacheBuffer: 10000
            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AlwaysOn
            }

            property var typeStrings: [
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

            function convertValueToString(value, type) {
                if (type === 0) {
                    // bool
                    return String(value);
                } if (type === 1) {
                    // int
                    return String(value.toFixed(0));
                } if (type === 2) {
                    // float
                    return String(value.toFixed(3));
                } if (type === 3) {
                    // vec2
                    return "(" + value.x.toFixed(3) + ", " + value.y.toFixed(3) + ")"
                } if (type === 4) {
                    // vec3
                    return "(" + value.x.toFixed(3) + ", " + value.y.toFixed(3) + ", " + value.z.toFixed(3) + ")"
                } if (type === 5) {
                    // vec4
                    return "(" + value.x.toFixed(3) + ", " + value.y.toFixed(3) + ", " + value.z.toFixed(3) + ", " + value.w.toFixed(3) + ")"
                } if (type === 6) {
                    // color
                    return "(" + value.r.toFixed(3) + ", " + value.g.toFixed(3) + ", " + value.b.toFixed(3) + ", " + value.a.toFixed(3) + ")"
                } if (type === 7) {
                    // sampler, show filename.
                    var text = String(value).slice(value.lastIndexOf('/') + 1);
                    return text;
                } if (type === 8) {
                    // define
                    return String(value);
                }
            }

            function columnWidth(column) {
                var w = uniformTable.width - (120 - 20 * designModeAnimated);
                if (column === 0)
                    return 20;
                if (column === 1)
                    return w * 0.10;
                if (column === 2)
                    return w * 0.40;
                if (column === 3)
                    return w * 0.20;
                if (column === 4)
                    return w * 0.30;
                if (column === 5)
                    return 20;
                return 0;
            }

            delegate: Item {
                width: uniformTable.width
                height: visible ? controlsItem.height : 0
                visible: model.visible
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    width: parent.width - 40
                    height: 1
                    z: -1
                    color: mainView.backgroundColor2
                }
                Row {
                    id: propertyListItemRow
                    x: 10
                    spacing: 10
                    height: parent.height
                    Item {
                        id: editPropertyButton
                        anchors.verticalCenter: parent.verticalCenter
                        width: 1 + opacity * uniformTable.columnWidth(0)
                        height: 20
                        opacity: (1.0 - designModeAnimated)
                        Image {
                            anchors.fill: parent
                            anchors.margins: 2
                            fillMode: Image.PreserveAspectFit
                            source: "images/icon_edit.png"
                            mipmap: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                uniformTable.currentIndex = index;
                                // Pressing the edit button
                                propertyEditDialog.propertyIndex = model.index;
                                propertyEditDialog.initialType = model.type;
                                propertyEditDialog.initialName = model.name;
                                propertyEditDialog.initialDefaultValue = model.defaultValue;
                                propertyEditDialog.initialMinValue = model.minValue;
                                propertyEditDialog.initialMaxValue = model.maxValue;
                                propertyEditDialog.initialDescription = model.description;
                                propertyEditDialog.initialCustomValue = model.customValue;
                                propertyEditDialog.initialUseCustomValue = model.useCustomValue;
                                propertyEditDialog.initialEnableMipmap = model.enableMipmap;
                                propertyEditDialog.initialExportImage = model.exportImage;
                                propertyEditDialog.open();
                            }
                        }
                    }
                    Label {
                        id: typeLabel
                        anchors.verticalCenter: parent.verticalCenter
                        width: uniformTable.columnWidth(1)
                        text: uniformTable.typeStrings[model.type]
                        color: mainView.foregroundColor2
                        font.pixelSize: 14
                    }
                    Label {
                        id: nameLabel
                        anchors.verticalCenter: parent.verticalCenter
                        width: uniformTable.columnWidth(2)
                        text: name
                        color: mainView.foregroundColor2
                        elide: Text.ElideMiddle
                        font.pixelSize: 14
                        MouseArea {
                            id: nameLabelMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                        }
                        ToolTip {
                            parent: nameLabel
                            visible: nameLabelMouseArea.containsMouse && model.description !== ""
                            text: truncateString(model.description, 100);
                        }
                        Rectangle {
                            id: colorPickerButton
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right
                            width: 40
                            height: 40
                            border.width: 1
                            border.color: foregroundColor2
                            color: visible ? model.value : "#000000"
                            visible: (model.type === 6)
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    colorPickerDialog.propertyItem = controlsLoader.item;
                                    colorPickerDialog.selectedColor = model.value;
                                    colorPickerDialog.initialColor = model.value;
                                    colorPickerDialog.open();
                                }
                            }
                        }
                    }

                    Item {
                        id: controlsItem
                        width: uniformTable.columnWidth(3) + uniformTable.columnWidth(4)
                        height: controlsLoader.item ? controlsLoader.item.height : 30

                        // This will update sliders, checkbox etc. to match current value
                        function updateUIValues() {
                            var controlsItem = controlsLoader.item;
                            if (controlsItem)
                                controlsItem.updateValue(model.value);
                        }

                        Loader {
                            id: controlsLoader
                            sourceComponent: {
                                if (model.useCustomValue)
                                    return customControlsComponent;
                                if (model.type === 0)
                                    return boolControlsComponent;
                                if (model.type === 1 || model.type === 2)
                                    return floatControlsComponent;
                                if (model.type === 3)
                                    return vec2ControlsComponent
                                if (model.type === 4)
                                    return vec3ControlsComponent
                                if (model.type === 5)
                                    return vec4ControlsComponent
                                if (model.type === 6)
                                    return colorControlsComponent
                                if (model.type === 7)
                                    return imageControlsComponent
                                if (model.type === 8)
                                    return defineControlsComponent
                            }
                        }

                        // Component with a text field and a slider
                        component FloatSliderComponent : Item {

                            property alias sliderValue: valueSlider.value
                            property alias from: valueSlider.from
                            property alias to: valueSlider.to

                            function updatePropertyValue(newValue) {
                                sliderValue = newValue;
                            }

                            width: controlsItem.width
                            height: 30

                            CustomTextField {
                                id: valueTextInput

                                property string originalText: ""
                                property bool editCancelled: false
                                // Show int as int and others as float
                                readonly property int valueFormat: (model.type === 1) ? 1 : 2
                                anchors.verticalCenter: parent.verticalCenter
                                width: uniformTable.columnWidth(3)
                                height: parent.height - 2
                                text: uniformTable.convertValueToString(valueSlider.value, valueFormat)
                                validator: RegularExpressionValidator {
                                    regularExpression: model.type === 1 ? /[-]?[0-9]+/ : /[-]?(\d{1,3})([.,]\d{1,3})?$/
                                }
                                Keys.onEscapePressed: {
                                    editCancelled = true;
                                    focus = false;
                                }
                                onActiveFocusChanged: {
                                    if (activeFocus) {
                                        originalText = text;
                                        editCancelled = false;
                                        selectAll();
                                    }
                                }
                                onEditingFinished: {
                                    if (editCancelled) {
                                        text = originalText;
                                        editCancelled = false;
                                    } else {
                                        // Update slider, which updates model.value
                                        sliderValue = valueTextInput.text;
                                        if (model.exportProperty === false) {
                                            // Non-exported properties (const variables) need shader baking
                                            effectManager.bakeShaders(true);
                                        }
                                    }
                                    valueTextInput.focus = false;
                                }
                            }

                            Slider {
                                id: valueSlider
                                anchors.left: valueTextInput.right
                                anchors.leftMargin: 20
                                anchors.verticalCenter: parent.verticalCenter
                                width: uniformTable.columnWidth(4) - anchors.leftMargin
                                height: 30
                                stepSize: model.type === 1 ? 1.0 : 0.0
                            }
                        }

                        Component {
                            id: boolControlsComponent
                            Row {
                                spacing: 20
                                function updateValue(newValue) {
                                    valueCheckBox.checked = newValue;
                                    model.value = newValue;
                                }
                                Label {
                                    id: valueTextLabel
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: uniformTable.columnWidth(3)
                                    text: uniformTable.convertValueToString(model.value, model.type)
                                    font.pixelSize: 14
                                    color: mainView.foregroundColor2
                                    wrapMode: Text.Wrap
                                }
                                CheckBox {
                                    id: valueCheckBox
                                    checked: model.value
                                    onToggled: model.value = checked;
                                }
                            }
                        }

                        Component {
                            id: floatControlsComponent
                            FloatSliderComponent {
                                id: valueSlider1
                                function updateValue(newValue) {
                                    valueSlider1.sliderValue = newValue;
                                    model.value = newValue;
                                }
                                from: model.minValue
                                to: model.maxValue
                                onSliderValueChanged: {
                                    if (status === Component.Ready)
                                        model.value = sliderValue;
                                }
                                Component.onCompleted: {
                                    updatePropertyValue(model.value);
                                }
                            }
                        }
                        Component {
                            id: vec2ControlsComponent
                            Column {
                                function updateValue(newValue) {
                                    valueSlider1.sliderValue = newValue.x;
                                    valueSlider2.sliderValue = newValue.y;
                                    model.value = newValue;
                                }
                                FloatSliderComponent {
                                    id: valueSlider1
                                    from: model.minValue.x
                                    to: model.maxValue.x
                                    onSliderValueChanged: {
                                        if (status === Component.Ready)
                                            model.value.x = sliderValue;
                                    }
                                    Component.onCompleted: {
                                        updatePropertyValue(model.value.x);
                                    }
                                }
                                FloatSliderComponent {
                                    id: valueSlider2
                                    from: model.minValue.y
                                    to: model.maxValue.y
                                    onSliderValueChanged: {
                                        if (status === Component.Ready)
                                            model.value.y = sliderValue;
                                    }
                                    Component.onCompleted: {
                                        updatePropertyValue(model.value.y);
                                    }
                                }
                            }
                        }
                        Component {
                            id: vec3ControlsComponent
                            Column {
                                function updateValue(newValue) {
                                    valueSlider1.sliderValue = newValue.x;
                                    valueSlider2.sliderValue = newValue.y;
                                    valueSlider3.sliderValue = newValue.z;
                                    model.value = newValue;
                                }
                                FloatSliderComponent {
                                    id: valueSlider1
                                    from: model.minValue.x
                                    to: model.maxValue.x
                                    onSliderValueChanged: {
                                        if (status === Component.Ready)
                                            model.value.x = sliderValue;
                                    }
                                    Component.onCompleted: {
                                        updatePropertyValue(model.value.x);
                                    }
                                }
                                FloatSliderComponent {
                                    id: valueSlider2
                                    from: model.minValue.y
                                    to: model.maxValue.y
                                    onSliderValueChanged: {
                                        if (status === Component.Ready)
                                            model.value.y = sliderValue;
                                    }
                                    Component.onCompleted: {
                                        updatePropertyValue(model.value.y);
                                    }
                                }
                                FloatSliderComponent {
                                    id: valueSlider3
                                    from: model.minValue.z
                                    to: model.maxValue.z
                                    onSliderValueChanged: {
                                        if (status === Component.Ready)
                                            model.value.z = sliderValue;
                                    }
                                    Component.onCompleted: {
                                        updatePropertyValue(model.value.z);
                                    }
                                }
                            }
                        }
                        Component {
                            id: vec4ControlsComponent
                            Column {
                                function updateValue(newValue) {
                                    valueSlider1.sliderValue = newValue.x;
                                    valueSlider2.sliderValue = newValue.y;
                                    valueSlider3.sliderValue = newValue.z;
                                    valueSlider4.sliderValue = newValue.w;
                                    model.value = newValue;
                                }
                                FloatSliderComponent {
                                    id: valueSlider1
                                    from: model.minValue.x
                                    to: model.maxValue.x
                                    onSliderValueChanged: {
                                        if (status === Component.Ready)
                                            model.value.x = sliderValue;
                                    }
                                    Component.onCompleted: {
                                        updatePropertyValue(model.value.x);
                                    }
                                }
                                FloatSliderComponent {
                                    id: valueSlider2
                                    from: model.minValue.y
                                    to: model.maxValue.y
                                    onSliderValueChanged: {
                                        if (status === Component.Ready)
                                            model.value.y = sliderValue;
                                    }
                                    Component.onCompleted: {
                                        updatePropertyValue(model.value.y);
                                    }
                                }
                                FloatSliderComponent {
                                    id: valueSlider3
                                    from: model.minValue.z
                                    to: model.maxValue.z
                                    onSliderValueChanged: {
                                        if (status === Component.Ready)
                                            model.value.z = sliderValue;
                                    }
                                    Component.onCompleted: {
                                        updatePropertyValue(model.value.z);
                                    }
                                }
                                FloatSliderComponent {
                                    id: valueSlider4
                                    from: model.minValue.w
                                    to: model.maxValue.w
                                    onSliderValueChanged: {
                                        if (status === Component.Ready)
                                            model.value.w = sliderValue;
                                    }
                                    Component.onCompleted: {
                                        updatePropertyValue(model.value.w);
                                    }
                                }
                            }
                        }
                        Component {
                            id: colorControlsComponent
                            Column {
                                function updateValue(newColor) {
                                    valueSliderR.sliderValue = newColor.r;
                                    valueSliderG.sliderValue = newColor.g;
                                    valueSliderB.sliderValue = newColor.b;
                                    valueSliderA.sliderValue = newColor.a;
                                    model.value = newColor;
                                }
                                FloatSliderComponent {
                                    id: valueSliderR
                                    from: model.minValue.r
                                    to: model.maxValue.r
                                    onSliderValueChanged: {
                                        if (status === Component.Ready)
                                            model.value.r = sliderValue;
                                    }
                                    Component.onCompleted: {
                                        updatePropertyValue(model.value.r);
                                    }
                                }
                                FloatSliderComponent {
                                    id: valueSliderG
                                    from: model.minValue.g
                                    to: model.maxValue.g
                                    onSliderValueChanged: {
                                        if (status === Component.Ready)
                                            model.value.g = sliderValue;
                                    }
                                    Component.onCompleted: {
                                        updatePropertyValue(model.value.g);
                                    }
                                }
                                FloatSliderComponent {
                                    id: valueSliderB
                                    from: model.minValue.b
                                    to: model.maxValue.b
                                    onSliderValueChanged: {
                                        if (status === Component.Ready)
                                            model.value.b = sliderValue;
                                    }
                                    Component.onCompleted: {
                                        updatePropertyValue(model.value.b);
                                    }
                                }
                                FloatSliderComponent {
                                    id: valueSliderA
                                    from: model.minValue.a
                                    to: model.maxValue.a
                                    onSliderValueChanged: {
                                        if (status === Component.Ready)
                                            model.value.a = sliderValue;
                                    }
                                    Component.onCompleted: {
                                        updatePropertyValue(model.value.a);
                                    }
                                }
                            }
                        }
                        Component {
                            id: imageControlsComponent
                            Row {
                                function updateValue(newValue) {
                                    model.value = newValue;
                                }
                                spacing: 20
                                Label {
                                    id: valueTextLabel
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: uniformTable.columnWidth(3)
                                    text: uniformTable.convertValueToString(model.value, model.type)
                                    font.pixelSize: 14
                                    color: mainView.foregroundColor2
                                    wrapMode: Text.Wrap
                                }
                                Item {
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: uniformTable.columnWidth(4) - 20
                                    height: 40
                                    Rectangle {
                                        anchors.fill: parent
                                        anchors.margins: 4
                                        color: mainView.foregroundColor2
                                        Image {
                                            anchors.fill: parent
                                            anchors.margins: 1
                                            fillMode: Image.PreserveAspectCrop
                                            source: model.value
                                        }
                                        Text {
                                            anchors.centerIn: parent
                                            text: "CHANGE"
                                            font.bold: true
                                            font.pixelSize: 12
                                            color: mainView.foregroundColor2
                                            style: Text.Outline
                                            styleColor: "#000000"
                                        }
                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                textureSourceDialog.modelIndex = index;
                                                if (model.value) {
                                                    textureSourceDialog.currentFolder = effectManager.getDirectory(model.value);
                                                    textureSourceDialog.selectedFile = effectManager.addFileToURL(model.value);
                                                } else {
                                                    textureSourceDialog.currentFolder = effectManager.getDefaultImagesDirectory();
                                                }
                                                textureSourceDialog.open();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        Component {
                            id: defineControlsComponent
                            CustomTextField {
                                id: valueTextInput

                                property string originalText: ""
                                property bool editCancelled: false

                                function updateValue(newValue) {
                                    model.value = newValue;
                                }

                                anchors.verticalCenter: parent.verticalCenter
                                width: controlsItem.width
                                height: 28
                                text: model.value
                                Keys.onEscapePressed: {
                                    editCancelled = true;
                                    focus = false;
                                }
                                onActiveFocusChanged: {
                                    if (activeFocus) {
                                        originalText = text;
                                        editCancelled = false;
                                        selectAll();
                                    }
                                }
                                onEditingFinished: {
                                    if (editCancelled) {
                                        text = originalText;
                                        editCancelled = false;
                                    } else {
                                        model.value = valueTextInput.text;
                                        // Defines need shader baking
                                        effectManager.bakeShaders(true);
                                    }
                                    valueTextInput.focus = false;
                                }
                            }
                        }
                        Component {
                            id: customControlsComponent
                            Label {
                                id: valueTextLabel
                                anchors.verticalCenter: parent.verticalCenter
                                verticalAlignment: Text.AlignVCenter
                                width: controlsItem.width
                                height: 28
                                text: "[custom]"
                                font.pixelSize: 14
                                color: mainView.foregroundColor2
                                wrapMode: Text.Wrap
                            }
                        }
                    }
                    Item {
                        id: resetPropertyButton
                        anchors.verticalCenter: parent.verticalCenter
                        width: uniformTable.columnWidth(5)
                        height: 20
                        enabled: model.value !== model.defaultValue
                        Image {
                            anchors.fill: parent
                            anchors.margins: 2
                            fillMode: Image.PreserveAspectFit
                            source: "images/icon_reset.png"
                            opacity: enabled ? 1.0 : 0.5
                            mipmap: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                uniformTable.currentIndex = index;
                                effectManager.uniformModel.resetValue(index);
                                controlsItem.updateUIValues();
                            }
                        }
                    }
                }
                Item {
                    id: settingsOverlay
                    anchors.fill: parent
                    opacity: showSettingsAnimated
                    visible: opacity > 0
                    Rectangle {
                        anchors.fill: parent
                        opacity: 0.6
                        color: mainView.backgroundColor1
                        MouseArea {
                            anchors.fill: parent
                            onPressed: { }
                        }
                    }

                    Rectangle {
                        property real sideMargin: 20
                        anchors.right: parent.right
                        anchors.rightMargin: sideMargin - (1.0 - showSettingsAnimated) * (width + sideMargin)
                        height: parent.height
                        width: settingsArea.width
                        color: mainView.backgroundColor2
                        Row {
                            id: settingsArea
                            height: parent.height
                            Item {
                                width: 10
                                height: 1
                            }
                            CustomIconButton {
                                anchors.verticalCenter: parent.verticalCenter
                                height: 24
                                width: height
                                icon: "images/icon_api.png"
                                toggledIcon: "images/icon_api_on.png"
                                toggleButton: true
                                toggled: model.exportProperty
                                // Samplers & Defines can't be made to non-exportable
                                enabled: (model.type <= 6)
                                description: "Export as effect API"
                                onClicked: {
                                    model.exportProperty = !model.exportProperty;
                                }
                            }
                            CustomIconButton {
                                anchors.verticalCenter: parent.verticalCenter
                                height: 24
                                width: height
                                icon: "images/icon_arrow_up.png"
                                enabled: model.canMoveUp
                                description: "Move up"
                                onClicked: {
                                    effectManager.uniformModel.moveIndex(model.index, -1);
                                }
                            }
                            CustomIconButton {
                                anchors.verticalCenter: parent.verticalCenter
                                height: 24
                                width: height
                                icon: "images/icon_arrow_down.png"
                                enabled: model.canMoveDown
                                description: "Move down"
                                onClicked: {
                                    effectManager.uniformModel.moveIndex(model.index, 1);
                                }
                            }
                            Item {
                                width: 10
                                height: 1
                            }
                        }
                    }
                }
            }
        }
    }
}
