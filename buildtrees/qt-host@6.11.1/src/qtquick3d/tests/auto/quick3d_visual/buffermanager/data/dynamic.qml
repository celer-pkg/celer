// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick3D
import QtQml
import QtQuick3D.Helpers
import io.qt.tests.auto.BufferManager

Item {
    id: root
    width: 1280
    height: 720
    visible: true

    property alias controller: objectSpawner

    Node {
        id: sceneRoot

        PerspectiveCamera {
            id: mainCamera
            z: 600
        }

        Model {
            id: mainSphere
            source: "#Sphere"
            materials: PrincipledMaterial {
                id: sphereMaterial
                baseColor: "blue"
            }

            function updateTexture() {
                if (objectSpawner.textures.length > 0)
                    sphereMaterial.baseColorMap = objectSpawner.textures[objectSpawner.textures.length - 1];
                else
                    sphereMaterial.baseColorMap = null

                if (objectSpawner.dynamicTextures.length > 0) {
                    sphereMaterial.emissiveFactor = Qt.vector3d(1, 1, 1)
                    sphereMaterial.emissiveMap = objectSpawner.dynamicTextures[objectSpawner.dynamicTextures.length - 1]
                } else {
                    sphereMaterial.emissiveMap = null
                    sphereMaterial.emissiveFactor = Qt.vector3d(0, 0, 0)
                }
            }
        }

        GridGeometry {
            id: unusedStaticGeometry
        }

        Node {
            id: objectSpawner
            property real range: 300
            property var directionLights: []
            property var pointLights: []
            property var spotLights: []
            property var cameras: []
            property var models: []
            property var dynamicModels: []
            property var dynamicGeometries: []
            property var textures: []
            property var dynamicTextures: []
            property var qmlTextures: []
            property var qmlSharedTextures: []
            property var item2Ds: []

            property int directionLightsCount: 0
            property int pointLightsCount: 0
            property int spotLightsCount: 0
            property int camerasCount: 0
            property int modelsCount: 0
            property int dynamicModelsCount: 0
            property int texturesCount: 0
            property int dynamicTexturesCount: 0
            property int qmlTexturesCount: 0
            property int qmlSharedTexturesCount: 0
            property int item2DsCount: 0

            Component {
                id: directionalLight
                DirectionalLight {
                    ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
                }
            }

            Component {
                id: pointLight
                PointLight {
                    ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
                }
            }

            Component {
                id: spotLight
                SpotLight {
                    ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
                }
            }

            Component {
                id: camera
                PerspectiveCamera {

                }
            }

            Component {
                id: model
                Model {
                    property alias color: material.diffuseColor
                    //source: "#Cube"
                    materials: DefaultMaterial {
                        id: material
                        diffuseColor: "red"
                    }
                }
            }

            Component {
                id: dynamicModel
                Model {
                    property alias color: material.baseColor
                    property alias lines: gridGeometry.horizontalLines
                    property alias step: gridGeometry.horizontalStep
                    materials: PrincipledMaterial {
                        lighting: PrincipledMaterial.NoLighting
                        id: material
                        baseColor: "red"
                    }
                    geometry: GridGeometry {
                        id: gridGeometry
                        verticalLines: horizontalLines
                        verticalStep: horizontalStep
                    }
                }
            }

            Component {
                id: texture
                Texture {
                }
            }

            Component {
                id: dynamicTexture
                Texture {
                    property alias height: gradientTexture.height
                    property alias width: gradientTexture.width
                    property alias startColor: gradientTexture.startColor
                    property alias endColor: gradientTexture.endColor
                    textureData: GradientTexture {
                        id: gradientTexture

                    }
                }
            }

            Component {
                id: qmlTexture
                Model {
                    property alias color: sourceItemRect.color
                    property alias text: textLabel.text
                    source: "#Rectangle"
                    materials: PrincipledMaterial {
                        lighting: PrincipledMaterial.NoLighting
                        baseColorMap: Texture {
                            sourceItem: Rectangle {
                                id: sourceItemRect
                                width: 256
                                height: 256
                                color: "black"
                                Text {
                                    id: textLabel
                                    anchors.centerIn: parent
                                    color: "white"
                                    font.pointSize: 64
                                }
                            }
                        }
                    }
                }
            }
            Component {
                id: qmlTextureShared
                Model {
                    property alias sharedItem: texture.sourceItem
                    source: "#Rectangle"
                    materials: PrincipledMaterial {
                        lighting: PrincipledMaterial.NoLighting
                        baseColorMap: Texture {
                            id: texture
                        }
                    }
                }
            }

            Component {
                id: item2D
                Node {
                    property alias text: textLabel.text
                    Text {
                        id: textLabel
                        anchors.centerIn: parent
                        font.pointSize: 64
                        color: "black"
                    }
                }
            }

            Component {
                id: resourceLoader
                ResourceLoader {
                    meshSources: [
                        "#Cube",
                        "#Sphere",
                        "#Cone",
                        "random1.mesh",
                        "random2.mesh"
                    ]
                    textures: [
                        Texture {
                            source: "noise1.jpg"
                        },
                        Texture {
                            source: "noise2.jpg"
                        },
                        Texture {
                            source: "noise1.jpg"
                            generateMipmaps: true
                        },
                        Texture {
                            sourceItem: Rectangle {
                                width: 200
                                height: 200
                                color: "pink"
                            }
                        },
                        Texture {
                            textureData: GradientTexture {

                            }
                        }
                    ]
                    geometries: [
                        unusedStaticGeometry
                    ]
                }
            }

            function getRandomVector3d(range) {
                return Qt.vector3d((2 * Math.random() * range) - range,
                                   (2 * Math.random() * range) - range,
                                   (2 * Math.random() * range) - range)
            }

            function getRandomColor() {
                return Qt.rgba(Math.random(),
                               Math.random(),
                               Math.random(),
                               1.0)
            }

            function getRandomInt(min, max) {
                return Math.floor(Math.random() * max) + min;
            }

            function getMeshSource() {
                let sources = [
                        "random1.mesh",
                        "random2.mesh",
                        "random3.mesh",
                        "random4.mesh",
                        "#Cube",
                        "#Sphere",
                        "#Cone",
                        "#Cylinder"
                    ]
                let index = Math.floor(Math.random() * sources.length);
                return sources[index]
            }

            function getImageSource() {
                let sources = [
                        "noise1.jpg",
                        "noise2.jpg",
                        "noise3.jpg",
                        "noise4.jpg",
                        "noise5.jpg"
                    ]
                let index = Math.floor(Math.random() * sources.length);
                return sources[index]
            }

            function addDirectionalLight() {
                let position = getRandomVector3d(objectSpawner.range)
                let rotation = getRandomVector3d(360)
                let instance = directionalLight.createObject(objectSpawner, { "position": position, "eulerRotation": rotation})
                directionLights.push(instance);
                directionLightsCount++
            }
            function removeDirectionalLight() {
                if (directionLights.length > 0) {
                    let instance = directionLights.pop();
                    instance.destroy();
                    directionLightsCount--
                }
            }
            function addPointLight() {
                let position = getRandomVector3d(objectSpawner.range)
                let instance = pointLight.createObject(objectSpawner, { "position": position})
                pointLights.push(instance);
                pointLightsCount++
            }
            function removePointLight() {
                if (pointLights.length > 0) {
                    let instance = pointLights.pop();
                    instance.destroy();
                    pointLightsCount--
                }
            }
            function addSpotLight() {
                let position = getRandomVector3d(objectSpawner.range)
                let rotation = getRandomVector3d(360)
                let instance = spotLight.createObject(objectSpawner, { "position": position, "eulerRotation": rotation})
                spotLights.push(instance);
                spotLightsCount++

            }
            function removeSpotLight() {
                if (spotLights.length > 0) {
                    let instance = spotLights.pop();
                    instance.destroy();
                    spotLightsCount--
                }
            }
            function addCamera() {
                let position = getRandomVector3d(objectSpawner.range * 2)
                let rotation = getRandomVector3d(360)
                let instance = camera.createObject(objectSpawner, { "position": position, "eulerRotation": rotation})
                cameras.push(instance);
                instance.lookAt(Qt.vector3d(0, 0, 0))
                view2.updateCamera();
                camerasCount++
            }
            function removeCamera() {
                if (cameras.length > 0) {
                    let instance = cameras.pop();
                    instance.destroy();
                    camerasCount--
                }
                view2.updateCamera();
            }
            function addModel(source : string) : Model {
                let position = getRandomVector3d(objectSpawner.range)
                let rotation = getRandomVector3d(360)
                let color = getRandomColor();
                let instance = model.createObject(objectSpawner, { "position": position, "eulerRotation": rotation, "color": color, "source": source})
                models.push(instance);
                modelsCount++
                return instance;
            }
            function removeModel() {
                if (models.length > 0) {
                    let instance = models.pop();
                    //instance.destroy();
                    modelsCount--
                }
            }

            function addDynamicModel() : Model {
                let position = getRandomVector3d(objectSpawner.range)
                let rotation = getRandomVector3d(360)
                let color = getRandomColor();
                let lines = getRandomInt(100, 1000);
                let steps = getRandomInt(1, 25);
                let instance = dynamicModel.createObject(objectSpawner, {"position": position, "eulerRotation": rotation, "color": color, "lines": lines, "step": steps})
                dynamicModels.push(instance)
                dynamicModelsCount++
                return instance
            }

            function removeDynamicModel() {
                if (dynamicModels.length > 0) {
                    let instance = dynamicModels.pop();
                    //instance.destroy();
                    dynamicModelsCount--
                }
            }

            function addTexture(source : string) : Texture {
                let instance = texture.createObject(objectSpawner, {"source": source})
                textures.push(instance);
                texturesCount++
                mainSphere.updateTexture();
                return instance;
            }
            function removeTexture() {
                if (textures.length > 0) {
                    let instance = textures.pop();
                    //instance.destroy();
                    texturesCount--
                    mainSphere.updateTexture()
                }
            }
            function changeTextures() {
                // reset the texture sources
                textures.forEach(texture => texture.source = getImageSource())
            }

            function addDynamicTexture() : Texture {
                let startColor = getRandomColor()
                let endColor = getRandomColor()
                let width = 2048
                let height = 2048
                let instance = dynamicTexture.createObject(objectSpawner, {"startColor": startColor, "endColor": endColor, "height": height, "width": width})
                dynamicTextures.push(instance);
                dynamicTexturesCount++
                mainSphere.updateTexture()
                return instance;
            }

            function removeDynamicTexture() {
                if (dynamicTextures.length > 0) {
                    let instance = dynamicTextures.pop();
                    //instance.destroy();
                    dynamicTexturesCount--
                    mainSphere.updateTexture()
                }
            }

            function addQmlTexture() : Model {
                let rectColor = getRandomColor()
                let position = getRandomVector3d(objectSpawner.range * 2)
                let labelText = qmlTexturesCount + 1
                let instance = qmlTexture.createObject(objectSpawner, {"position": position, "color": rectColor, "text": labelText})
                qmlTextures.push(instance)
                qmlTexturesCount++
                return instance
            }

            function removeQmlTexture() {
                if (qmlTextures.length > 0) {
                    let instance = qmlTextures.pop()
                    //instance.destroy()
                    qmlTexturesCount--
                }
            }

            function addQmlSharedTexture() : Model {
                let position = getRandomVector3d(objectSpawner.range * 2)
                let instance = qmlTextureShared.createObject(objectSpawner, {"sharedItem": sharedItem, "position": position})
                qmlSharedTextures.push(instance)
                qmlSharedTexturesCount++
                return instance
            }

            function removeQmlSharedTexture() {
                if (qmlSharedTextures.length > 0) {
                    let instance = qmlSharedTextures.pop()
                    //instance.destroy()
                    qmlSharedTexturesCount--
                }
            }

            function addItem2D() {
                let position = getRandomVector3d(objectSpawner.range * 2)
                let labelText = item2DsCount + 1
                let instance = item2D.createObject(objectSpawner, {"position": position, "text":labelText})
                item2Ds.push(instance);
                item2DsCount++
            }

            function removeItem2D() {
                if (item2Ds.length > 0) {
                    let instance = item2Ds.pop()
                    instance.destroy()
                    item2DsCount--
                }
            }

            function addResourceLoader(): ResourceLoader {
                return resourceLoader.createObject(objectSpawner);
            }

            function changeModels() {
                // reset the model sources
                models.forEach(model => model.source = getMeshSource())
            }
            function changeDynamicModels() {
                // reset the dynamic model sources
                if (dynamicModels.length == 0)
                    return

                var shuffleIndexs = Array.from({length: dynamicModels.length}, (x, i) => i);
                function shuffle(a) {
                    for (let i = a.length - 1; i > 0; i--) {
                        const j = Math.floor(Math.random() * (i + 1));
                        [a[i], a[j]] = [a[j], a[i]];
                    }
                    return a;
                }
                shuffleIndexs = shuffle(shuffleIndexs);
                for (var i = 0; i < dynamicModels.length; i++)
                    dynamicModels[i].geometry = dynamicGeometries[shuffleIndexs[i]]
            }
        }

    }

    Item {
        id: viewsContainer
        anchors.fill: parent

        Image {
            id: sharedItem
            source: "noise1.jpg"
            width: 256
            height: 256
        }

        View3D {
            id: view1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            width: parent.width * 0.5
            importScene: sceneRoot
            camera: mainCamera
            environment: SceneEnvironment {
                clearColor: "green"
                backgroundMode: SceneEnvironment.Color
            }
        }

        View3D {
            id: view2
            anchors.top: parent.top
            anchors.left: view1.right
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            importScene: sceneRoot
            environment: SceneEnvironment {
                clearColor: "pink"
                backgroundMode: SceneEnvironment.Color
            }

            function updateCamera() {
                if (objectSpawner.cameras.length > 0) {
                    var dynamicCamera = objectSpawner.cameras[objectSpawner.cameras.length - 1]
                    view2.camera = dynamicCamera
                } else {
                    view2.camera = mainCamera
                }
            }
        }
    }
}
