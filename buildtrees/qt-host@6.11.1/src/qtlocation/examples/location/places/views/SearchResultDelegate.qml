// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtLocation
import QtQuick.Controls
import QtQuick.Layouts
import "../helper.js" as Helper

Item {
    id: root

    signal showPlaceDetails(var place,var distance)
    signal searchFor(string query)

    height: childrenRect.height

    //! [PlaceSearchModel place delegate]
    Component {
        id: placeComponent
        Item {
            id: placeRoot
            width: root.width
            height: Math.max(icon.height, 3 * placeName.height)

            Rectangle {
                anchors.fill: parent
                color: "#44ffffff"
                visible: mouse.pressed
            }

            Rectangle {
                anchors.fill: parent
                color: "#dbffde"
                visible: model.sponsored !== undefined ? model.sponsored : false

                Label {
                    text: qsTr("Sponsored result")
                    horizontalAlignment: Text.AlignRight
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    font.pixelSize: 8
                    visible: model.sponsored !== undefined ? model.sponsored : false
                }
            }

            GridLayout {
                rows: 2
                columns: 2
                anchors.fill: parent
                anchors.leftMargin: 30
                flow: GridLayout.TopToBottom

                Image {
                    // anchors.verticalCenter: parent.verticalCenter
                    id:icon
                    source: place.favorite ? Qt.resolvedUrl("../resources/star.png") : place.icon.url()
                    Layout.rowSpan: 2
                }

                Label {
                    id: placeName
                    text: place.favorite ? place.favorite.name : place.name
                    Layout.fillWidth: true
                }

                Label {
                    id: distanceText
                    font.italic: true
                    text: Helper.formatDistance(distance)
                    Layout.fillWidth: true
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 15
                height: 1
                color: "#46a2da"
            }

            MouseArea {
                id: mouse
                anchors.fill: parent
                onClicked: {
                    if (model.type === undefined || type === PlaceSearchModel.PlaceResult) {
                        if (!place.detailsFetched)
                            place.getDetails();
                        root.showPlaceDetails(model.place, model.distance);
                    }
                }
            }
        }
    }
    //! [PlaceSearchModel place delegate]

    Component {
        id: proposedSearchComponent

        Item {
            id: proposedSearchRoot

            width: root.width
            height: Math.max(icon.height, 2 * proposedSearchTitle.height)

            Rectangle {
                anchors.fill: parent
                color: "#11ffffff"
                visible: mouse.pressed
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 30

                Image {
                    source: icon.url()
                }

                Label {
                    id: proposedSearchTitle
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Search for " + title
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 15
                height: 1
                color: "#46a2da"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.ListView.view.model.updateWith(index);
            }
        }
    }

    Loader {
        anchors.left: parent.left
        anchors.right: parent.right

        sourceComponent: {
            switch (model.type) {
            case PlaceSearchModel.PlaceResult:
                return placeComponent;
            case PlaceSearchModel.ProposedSearchResult:
                return proposedSearchComponent;
            default:
                //do nothing, don't assign component if result type not recognized
            }
        }
    }
}
