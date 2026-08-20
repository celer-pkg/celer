// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: selection

    enum ModelGroup {
        Face,
        Eyeswear,
        Headwear,
        Item,
        Name,
        Toy
    }

    property int totalPrice: 0
    property int toyIndex: -1
    required property AccessoryModel accessoryModel
    property bool wrap: false

    property var __model: ToyModel.get(selection.toyIndex) ?? null
    property var __modelFace: null
    property var __modelHeadwear: null
    property var __modelAccessory: null
    property var __modelEyewear: null
    property string __selectedName: ""

    function pickSelected() {
        let total = 0
        let selectedNoun = ""
        let selectedAdjective = ""
        for (let i = 0; i < accessoryModel.count; i++) {
            const item = accessoryModel.get(i)
            if (!item.selected)
                continue

            switch (item.group) {
            case "noun":
                selectedNoun = item.name
                break;

            case "adjectives":
                selectedAdjective = item.name
                break;

            case "headwear":
                __modelHeadwear = item
                total += item.newPrice
                break;

            case "eyes":
                __modelFace = item
                total +=item.newPrice
                break;

            case "eyewear":
                __modelEyewear = item
                total += item.newPrice
                break;

            case "items":
                __modelAccessory = item
                total += item.newPrice
                break;

            default:
                break;
            }
        }
        __selectedName = `${selectedAdjective} ${selectedNoun}`

        const discount_ = __model.discountPercent
        const toyPrice_ = __model.originalPrice * (1 - discount_ / 100)
        totalPrice = total + toyPrice_
    }

    component GridOrderItem : OrderItem {
        id: orderItem

        required property int group

        property var __modelData: {
            switch (group) {
                case OrderGrid.ModelGroup.Face:
                    return selection.__modelFace
                case OrderGrid.ModelGroup.Eyeswear:
                    return selection.__modelEyewear
                case OrderGrid.ModelGroup.Headwear:
                    return selection.__modelHeadwear
                case OrderGrid.ModelGroup.Item:
                    return selection.__modelAccessory
                case OrderGrid.ModelGroup.Toy:
                    return selection.__model
                default: break
            }
            return null
        }

        image: __modelData ? Qt.url(__modelData.image) : Qt.url("")
        name: {
            if (group === OrderGrid.ModelGroup.Name)
                return selection.__selectedName
            return __modelData ? __modelData.name : ""
        }
        oldPrice: {
            if (group === OrderGrid.ModelGroup.Name)
                return 0
            if (!__modelData)
                return 0
            if (group !== OrderGrid.ModelGroup.Toy)
                return __modelData.oldPrice
            else
                return __model.originalPrice
        }
        newPrice: {
            if (group === OrderGrid.ModelGroup.Name)
                return 0
            if (!__modelData)
                return 0
            if (group !== OrderGrid.ModelGroup.Toy)
                return __modelData.newPrice
            const discount_ = __modelData.discountPercent
            return oldPrice * (1 - discount_ / 100)
        }
        isSelected: {
            if (group !== OrderGrid.ModelGroup.Toy)
                return __modelData ? __modelData.selected : false
            if (selection.toyIndex < 0)
                return false
            return selection.accessoryModel.get(selection.toyIndex) ?? false
        }
        priceVisible: oldPrice > 0
    }

    implicitWidth: orders.implicitWidth

    Component.onCompleted: pickSelected()

    ToyLabel {
        id: order
        anchors {
            top: parent.top
            left: parent.left
        }
        textStyle: ApplicationConfig.TextStyle.H2_Bold
        text: qsTr("Your order")
    }

    GridLayout {
        id: orders
        anchors {
            top: order.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            topMargin: ApplicationConfig.responsiveSize(90)
        }
        columns: 2
        columnSpacing: Math.floor(ApplicationConfig.responsiveSize(80))
        rowSpacing: Math.floor(ApplicationConfig.responsiveSize(80))
        uniformCellWidths: true

        GridOrderItem {
            id: toy
            group: OrderGrid.ModelGroup.Toy
            isSelected: true // this should be always true as there is always a toy in an order
            label: "Toy"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: !selection.wrap ? implicitWidth : -1
        }
        GridOrderItem {
            id: face
            group: OrderGrid.ModelGroup.Face
            label: "Face"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: !selection.wrap ? implicitWidth : -1
        }
        GridOrderItem {
            id: headwear
            group: OrderGrid.ModelGroup.Headwear
            label: "HeadWear"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: !selection.wrap ? implicitWidth : -1
        }
        GridOrderItem {
            id: accessory
            group: OrderGrid.ModelGroup.Item
            label: "Accessory"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: !selection.wrap ? implicitWidth : -1
        }
        GridOrderItem {
            id: eyewear
            group: OrderGrid.ModelGroup.Eyeswear
            label: "Eyewear"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: !selection.wrap ? implicitWidth : -1
        }
        GridOrderItem {
            id: name
            group: OrderGrid.ModelGroup.Name
            label: "Name"
            isSelected: true
            priceVisible: false
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: !selection.wrap ? implicitWidth : -1
        }
    }
}
