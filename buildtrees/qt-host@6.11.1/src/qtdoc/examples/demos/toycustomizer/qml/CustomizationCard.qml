// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Item {
    id: customizationCard

    property int oldPrice
    property url cardImageSource
    property int newPrice
    property string cardName
    property bool selected: false
    property string selectedColorName: ""
    property real modelRating
    property bool isExclusive: false

    property real discountAmount: Math.max(oldPrice - newPrice, 0)
    property bool hasDiscount: discountAmount > 0
    property int discountPercent: (oldPrice > 0 && hasDiscount)
                                   ? Math.round((discountAmount / oldPrice) * 100) : 0
    readonly property bool ready: toyImage.status === Image.Ready

    function __updateSelectedColor(colorName :string) {
        colorChosen(colorName)
    }

    signal colorChosen(string colorName)
    signal chooseRequested
    signal removeRequested

    implicitWidth: ApplicationConfig.responsiveSize(558)
    implicitHeight: ApplicationConfig.responsiveSize(820)

    Rectangle {
        anchors.fill: parent
        color: customizationCard.selected ? "#DAEBFF" : "#FFFFFF"
        radius: ApplicationConfig.responsiveSize(16)
        border.color: customizationCard.selected ? "#2269EE" : "#D7D6E1"
        border.width: ApplicationConfig.responsiveSize(4)
    }

    ColumnLayout {
        id: accessoryColumn

        anchors {
            fill: parent
            leftMargin: ApplicationConfig.responsiveSize(48)
            rightMargin: ApplicationConfig.responsiveSize(48)
            bottomMargin: ApplicationConfig.responsiveSize(48)
        }
        spacing: 6

        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

            ToyImage {
                id: toyImage
                asynchronous: true
                implicitHeight: customizationCard.selected ? ApplicationConfig.responsiveSize(230)
                                                           : ApplicationConfig.responsiveSize(340)
                implicitWidth: customizationCard.selected ? ApplicationConfig.responsiveSize(302)
                                                          : ApplicationConfig.responsiveSize(448)
                source: customizationCard.cardImageSource
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            }

            RowLayout {
                id: colorSelectionLayout
                visible: customizationCard.selected
                spacing: 24

                ColorButton {
                    id: greenButton
                    property string colorName: "#36B45A"
                    label: "Green"
                    buttonColor: "#36B45A"
                    checked: customizationCard.selectedColorName === colorName
                    onClicked: customizationCard.__updateSelectedColor(checked ? colorName : "")
                }

                ColorButton {
                    id: pinkButton
                    property string colorName: "#EF4CB0"
                    label: "Pink"
                    buttonColor: "#EF4CB0"
                    checked: customizationCard.selectedColorName === colorName
                    onClicked: customizationCard.__updateSelectedColor(checked ? colorName : "")
                }

                ColorButton {
                    id: blueButton
                    property string colorName: "#419DE3"
                    label: "Blue"
                    buttonColor: "#419DE3"
                    checked: customizationCard.selectedColorName === colorName
                    onClicked: customizationCard.__updateSelectedColor(checked ? colorName : "")
                }
            }
        }

        ToyLabel {
            Layout.fillWidth: true
            text: customizationCard.cardName
            // Setting lineHeight to 1 means no spacing between the lines
            // if the text is wrapped to new line.
            lineHeight: 1
            wrapMode: Text.Wrap
            textStyle: ApplicationConfig.TextStyle.H3_Bold
        }

        ColumnLayout {
            spacing: 4

            RowLayout {
                id: priceAndRatingLayout
                Layout.preferredHeight: newPriceLabel.height
                Layout.fillWidth: true

                ToyLabel {
                    id: newPriceLabel
                    textStyle: ApplicationConfig.TextStyle.Price_M
                    text: customizationCard.newPrice
                }
                ColorIcon {
                    implicitWidth: 51
                    implicitHeight: 12
                    source: "icons/currency.svg"
                    color: newPriceLabel.color
                }
                Item {
                    Layout.fillWidth: true
                }

                ToyLabel {
                    id: modelRating
                    Layout.alignment: Qt.AlignRight
                    textStyle: ApplicationConfig.TextStyle.Body
                    text: qsTr("★ %1").arg(customizationCard.modelRating)
                }
            }

            RowLayout {
                id: discountLayout
                visible: customizationCard.hasDiscount
                Layout.preferredWidth: ApplicationConfig.responsiveSize(180)
                Layout.preferredHeight: ApplicationConfig.responsiveSize(22)

                ToyLabel {
                    visible: customizationCard.hasDiscount
                    color: "#6A6A8D"
                    text: customizationCard.oldPrice
                    wrapMode: Text.Wrap
                    textStyle: ApplicationConfig.TextStyle.Price_S
                    font.strikeout: true
                }

                ToyLabel {
                    visible: customizationCard.hasDiscount
                    color: "#6A6A8D"
                    text: qsTr("(%1%)").arg(-customizationCard.discountPercent)
                    wrapMode: Text.Wrap
                    textStyle: ApplicationConfig.TextStyle.Price_S
                }
            }
        }

        ToyButton {
            id: chooseButton
            enabled: !customizationCard.isExclusive || !customizationCard.selected
            type: ToyButton.Type.Secondary
            textStyle: ApplicationConfig.TextStyle.Button_L
            font.bold: false

            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            Layout.preferredWidth: ApplicationConfig.responsiveSize(448)
            Layout.preferredHeight: ApplicationConfig.responsiveSize(104)

            text: {
                if (customizationCard.isExclusive)
                    return customizationCard.selected ? qsTr("Selected") : qsTr("Select")
                else
                    return customizationCard.selected ? qsTr("Remove") : qsTr("Choose")
            }

            onClicked: {
                if (!customizationCard.selected)
                    customizationCard.chooseRequested()
                else if (!customizationCard.isExclusive)
                    customizationCard.removeRequested()
            }
        }
    }
}
