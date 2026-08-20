// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick3D

import Example

RowLayout {
    id: rootView
    property bool specularGlossyMode: false
    property int principledMaterialMask: PrincipledMaterial.NoMask
    property int specularGlossyMaterialMask: SpecularGlossyMaterial.NoMask

    ListModel {
        id: mdlPrincipledMaterialLeft
        ListElement {
            name: "Roughness"
            number: PrincipledMaterial.RoughnessMask
        }
        ListElement {
            name: "Normal Strength"
            number: PrincipledMaterial.NormalStrengthMask
        }
        ListElement {
            name: "Specular Amount"
            number: PrincipledMaterial.SpecularAmountMask
        }
        ListElement {
            name: "Clearcoat Amount"
            number: PrincipledMaterial.ClearcoatAmountMask
        }
        ListElement {
            name: "Clearcoat Roughness Amount"
            number: PrincipledMaterial.ClearcoatRoughnessAmountMask
        }
        ListElement {
            name: "Clearcoat Normal Strength"
            number: PrincipledMaterial.ClearcoatNormalStrengthMask
        }
        ListElement {
            name: "Height Amount"
            number: PrincipledMaterial.HeightAmountMask
        }
        ListElement {
            name: "Metalness"
            number: PrincipledMaterial.MetalnessMask
        }
        ListElement {
            name: "Occlusion Amount"
            number: PrincipledMaterial.OcclusionAmountMask
        }
        ListElement {
            name: "Thickness Factor"
            number: PrincipledMaterial.ThicknessFactorMask
        }
        ListElement {
            name: "Transmission Factor"
            number: PrincipledMaterial.TransmissionFactorMask
        }
    }
    ListModel {
        id: mdlSpecularGlossyMaterialLeft
        ListElement {
            name: "Glossiness"
            number: SpecularGlossyMaterial.GlossinessMask
        }
        ListElement {
            name: "Normal Strength"
            number: SpecularGlossyMaterial.NormalStrengthMask
        }
        ListElement {
            name: "Clearcoat Amount"
            number: SpecularGlossyMaterial.ClearcoatAmountMask
        }
        ListElement {
            name: "Clearcoat Roughness Amount"
            number: SpecularGlossyMaterial.ClearcoatRoughnessAmountMask
        }
        ListElement {
            name: "Clearcoat Normal Strength"
            number: SpecularGlossyMaterial.ClearcoatNormalStrengthMask
        }
        ListElement {
            name: "Height Amount"
            number: SpecularGlossyMaterial.HeightAmountMask
        }
        ListElement {
            name: "Occlusion Amount"
            number: SpecularGlossyMaterial.OcclusionAmountMask
        }
        ListElement {
            name: "Thickness Factor"
            number: SpecularGlossyMaterial.ThicknessFactorMask
        }
        ListElement {
            name: "Transmission Factor"
            number: SpecularGlossyMaterial.TransmissionFactorMask
        }
    }

    ListModel {
        id: mdlPrincipledMaterialRight
    }
    ListModel {
        id: mdlSpecularGlossyMaterialRight
    }

    ListView {
        id: listLeft
        Rectangle {
            z: -1
            anchors.fill: parent
            color:"gray"
        }
        boundsBehavior: Flickable.StopAtBounds
        implicitWidth: 170
        implicitHeight: 11 * 20
        model: rootView.specularGlossyMode ? mdlSpecularGlossyMaterialLeft : mdlPrincipledMaterialLeft
        delegate: Rectangle {
            id: delegateRect
            required property int index
            required property string name
            width: ListView.view.width
            height: 20
            color: ListView.isCurrentItem ? "lightsteelblue" :
                                            ( (index % 2) ? "darkgray" : "lightgray" )
            MouseArea {
                anchors.fill: parent
                onClicked:{
                    parent.ListView.view.currentIndex = delegateRect.index
                }
            }
            Text {
                id: contactInfo
                anchors.fill: parent
                anchors.leftMargin: 5
                color: parent.ListView.isCurrentItem ? "white" : "black"
                text: delegateRect.name
            }
        }
    }

    ColumnLayout {
        Button {
            id: addButton
            text: ">>"
            onPressed: {
                if ( listLeft.count > 0
                        && listLeft.currentIndex >= 0
                        && listLeft.currentIndex < listLeft.count ) {
                    let item = listLeft.model.get(listLeft.currentIndex)
                    listRight.model.append( item )
                    if ( rootView.specularGlossyMode )
                        rootView.specularGlossyMaterialMask |= item.number
                    else
                        rootView.principledMaterialMask |= item.number
                    listLeft.model.remove( listLeft.currentIndex, 1 )
                }
            }
            background: Rectangle {
                implicitWidth: 20
                implicitHeight: 50
                opacity: enabled ? 1 : 0.3
                color: addButton.down ? "#d0d0d0" : "#e0e0e0"
            }
        }
        Button {
            id: removeButton
            text: "<<"
            onPressed: {
                if ( listRight.count > 0
                        && listRight.currentIndex >= 0
                        && listRight.currentIndex < listRight.count ) {
                    let item = listRight.model.get(listRight.currentIndex)
                    listLeft.model.append( item )
                    if ( rootView.specularGlossyMode )
                        rootView.specularGlossyMaterialMask &= ~(item.number)
                    else
                        rootView.principledMaterialMask &= ~(item.number)
                    listRight.model.remove( listRight.currentIndex, 1 )
                }
            }
            background: Rectangle {
                implicitWidth: 20
                implicitHeight: 50
                opacity: enabled ? 1 : 0.3
                color: removeButton.down ? "#d0d0d0" : "#e0e0e0"
            }
        }
    }

    ListView {
        id: listRight
        Rectangle {
            z: -1
            anchors.fill: parent
            color:"gray"
        }
        boundsBehavior: Flickable.StopAtBounds
        implicitWidth: 170
        implicitHeight: 11 * 20
        model: rootView.specularGlossyMode ? mdlSpecularGlossyMaterialRight : mdlPrincipledMaterialRight
        delegate: Rectangle {
            id: delegateRect2
            required property int index
            required property string name
            width: ListView.view.width
            height: 20
            color: ListView.isCurrentItem ? "lightsteelblue" :
                                            ( (index % 2) ? "darkgray" : "lightgray" )
            MouseArea {
                anchors.fill: parent
                onClicked:{
                    parent.ListView.view.currentIndex = delegateRect2.index
                }
            }
            Text {
                anchors.fill: parent
                anchors.leftMargin: 5
                color: parent.ListView.isCurrentItem ? "white" : "black"
                text: delegateRect2.name
            }
        }
    }
}
