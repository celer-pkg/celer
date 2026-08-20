import QtQuick
import QtQuick.Window
import QtQuick3D
import QtQuick.Controls

Window {
    id: window
    width: 1280
    height: 720
    visible: true

    Rectangle {
        // the offscreen rendered content
        id: srcItem
        visible: false
        width: 256
        height: 256
        color: "lightgreen"
        Rectangle {
            anchors.centerIn: parent
            width: 140
            height: 140
            color: "yellow"
            NumberAnimation on rotation {
                running: animationCheckbox.checked
                from: 0
                to: 90
                loops: Animation.Infinite
            }
        }
    }

    View3D {
        id: sceneView
        anchors.fill: parent
        focus: true

        environment: SceneEnvironment {
            id: environment
            probeExposure: 0.05
            clearColor: "#cceeff"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 0)
        }

        Model {
            position: Qt.vector3d(0, 0, -300)
            source: "#Rectangle"
            materials: [
                DefaultMaterial {
                    lighting: DefaultMaterial.NoLighting
                    diffuseMap: Texture {
                        id: texture
                        sourceItem: srcItem
                    }
                }
            ]
        }

        // References Texture
        Model {
            position: Qt.vector3d(-150, 0, -300)
            source: "#Sphere"
            NumberAnimation on eulerRotation.y {
                running: animation3DCheckbox.checked
                duration: 4000
                from: 0
                to: 360
                loops: Animation.Infinite
            }
            NumberAnimation on eulerRotation.z {
                running: animation3DCheckbox.checked
                duration: 8000
                from: 0
                to: 360
                loops: Animation.Infinite
            }
            materials: [
                DefaultMaterial {
                    lighting: DefaultMaterial.NoLighting
                    diffuseMap: texture
                }
            ]
        }

        // References another Texture's sourceItem
        Model {
            position: Qt.vector3d(150, 0, -300)
            source: "#Cube"
            NumberAnimation on eulerRotation.y {
                running: animation3DCheckbox.checked
                duration: 1000
                from: 0
                to: 90
                loops: Animation.Infinite
            }
            materials: [
                DefaultMaterial {
                    lighting: DefaultMaterial.NoLighting
                    diffuseMap: Texture {
                        sourceItem: texture.sourceItem
                        onSourceItemChanged: console.log("source item for cube is now " + sourceItem)
                    }
                }
            ]
        }

        // Uses an inline item tree in sourceItem, not affected by the buttons
        Model {
            visible: coneCheckbox.checked
            position: Qt.vector3d(0, 70, -300)
            source: "#Cone"
            NumberAnimation on eulerRotation.y {
                running: animation3DCheckbox.checked
                duration: 1000
                from: 0
                to: 90
                loops: Animation.Infinite
            }
            materials: [
                DefaultMaterial {
                    lighting: DefaultMaterial.NoLighting
                    diffuseMap: Texture {
                        sourceItem: Rectangle {
                            width: Math.round(coneTexSizeSlider.value)
                            height: Math.round(coneTexSizeSlider.value)
                            color: "transparent"
                            Rectangle {
                                color: "lightgreen"
                                width: 192
                                height: 192
                                anchors.centerIn: parent
                                Rectangle {
                                    anchors.centerIn: parent
                                    width: 128
                                    height: 128
                                    color: "yellow"
                                    NumberAnimation on rotation {
                                        running: animationCheckbox.checked
                                        from: 0
                                        to: 90
                                        loops: Animation.Infinite
                                    }
                                }
                            }
                        }
                    }
                }
            ]
        }
    }
    Component {
        id: sourceItemComponent
        Rectangle {
            width: 500
            height: 500
            gradient: "NightFade"
            CheckBox {
                text: "Some quick control in a texture :D"
                checked: true
            }
            Rectangle {
                anchors.centerIn: parent
                color: "blue"
                width: 140
                height: 140
                NumberAnimation on rotation {
                    running: animationCheckbox.checked
                    from: 0
                    to: 90
                    loops: Animation.Infinite
                }
            }
        }
    }
    Component {
        id: textureProviderSourceItemComponent
        Image {
            width: 1024
            height: 1024
            sourceSize.width: 1024
            sourceSize.height: 1024
            property string src: "qt_logo_rect.png"
            source: src // url, so use a separate src string property
            function switchSource() {
                if (src === "qt_logo_rect.png")
                    src = "qt_logo.png";
                else
                    src = "qt_logo_rect.png";
                console.log("Image.source is now " + source);
            }
        }
    }
    Component {
        id: layerSourceItemComponent
        Rectangle {
            layer.enabled: true // the difference to sourceItemComponent lies in here

            width: 500
            height: 500
            color: "lightGray"
            CheckBox {
                text: "Some quick control in a texture :D"
                checked: true
            }
            Rectangle {
                anchors.centerIn: parent
                color: "blue"
                width: 140
                height: 140
                NumberAnimation on rotation {
                    running: animationCheckbox.checked
                    from: 0
                    to: 90
                    loops: Animation.Infinite
                }
            }
        }
    }
    Column {
        Button {
            text: "Assign new sourceItem (implicit layer on Quick3D side)"
            onClicked: {
                console.log("assigning new implicit layer-based sourceItem");
                const item = sourceItemComponent.createObject(sourceItemContainer);
                texture.sourceItem = item;
            }
        }
        Row {
            Button {
                text: "Assign new sourceItem (texture provider, non-layer)"
                onClicked: {
                    console.log("assigning new non-layer sourceItem");
                    const item = textureProviderSourceItemComponent.createObject(sourceItemContainer);
                    texture.sourceItem = item;
                }
            }
            Button {
                text: "Change Image.source"
                onClicked: texture.sourceItem.switchSource()
            }
        }
        Button {
            text: "Assign new sourceItem (explicit layer on Quick side)"
            onClicked: {
                console.log("assigning new explicit layer-based sourceItem");
                const item = layerSourceItemComponent.createObject(sourceItemContainer);
                texture.sourceItem = item;
            }
        }
        Button {
            text: "Set sourceItem null"
            onClicked: {
                console.log("sourceItem destroy and set to null");
                if (texture.sourceItem)
                    texture.sourceItem.destroy();
                texture.sourceItem = null;
            }
        }
        Button {
            text: "Destroy sourceItem"
            onClicked: {
                console.log("sourceItem destroy");
                if (texture.sourceItem)
                    texture.sourceItem.destroy()
            }
        }
        CheckBox {
            id: animationCheckbox
            text: "Animate quick content (layer only)"
        }
        CheckBox {
            id: animation3DCheckbox
            text: "Animate 3D objects"
        }
        CheckBox {
            id: coneCheckbox
            text: "Show Cone with inline,\nsemi-transparent sourceItem"
            checked: true
        }
        Column {
            Label {
                text: "Cone sourceItem size: " + Math.round(coneTexSizeSlider.value) + "x" + Math.round(coneTexSizeSlider.value)
            }
            Slider {
                id: coneTexSizeSlider
                value: 256
                from: 32
                to: 1024
            }
        }
    }
    Item {
        id: sourceItemContainer
        visible: false
    }
}
