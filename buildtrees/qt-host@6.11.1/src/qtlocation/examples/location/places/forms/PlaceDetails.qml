// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import "../helper.js" as Helper

PlaceDetailsForm {

    property var place
    property real distanceToPlace

    signal searchForSimilar(var place)
    signal showReviews(var place)
    signal showEditorials(var place)
    signal showImages(var place)

    function placeAddress(place) {
        if (!place)
            return "";

        if (place.location.address.text.length > 0)
            return place.location.address.text;

        return place.location.address.street;
    }

    function categoryNames(categories) {
        var result = "";

        for (var i = 0; i < categories.length; ++i) {
            if (result === "") {
                result = categories[i].name;
            } else {
                result = result + ", " + categories[i].name;
            }
        }

        return result;
    }

    function additonalInformation(place) {
        var keys = place.extendedAttributes.keys();
        var result;

        for (var i = 0; i < keys.length; ++i) {
            var label = place.extendedAttributes[keys[i]].label;
            var text = place.extendedAttributes[keys[i]].text;
            if (label) {
                result += label + ": "
                if (text)
                    result += text
                result += "<br/>"
            }
        }

        if (!result)
            result = qsTr("No information")

        return result;
    }

    editorialsButton.onClicked: showEditorials(place)
    imagesButton.onClicked: showImages(place)
    reviewsButton.onClicked: showReviews(place)
    findSimilarButton.onClicked: searchForSimilar(place)

    Component.onCompleted: {
        placeName.text = place ? (place.favorite ? place.favorite.name : place.name) : ""
        placeIcon.source = place ? (place.favorite ? place.favorite.icon.url(Qt.size(40,40))
                                        : place.icon.url() == "" ?
                                          Qt.resolvedUrl("../resources/marker.png")
                                        : place.icon.url(Qt.size(40,40))) : ""
        ratingView.rating = (place && place.ratings) ? place.ratings.average : 0
        distance.text = Helper.formatDistance(distanceToPlace)
        address.text = placeAddress(place)
        categories.text = place ? categoryNames(place.categories) : ""
        phone.text = place ? place.primaryPhone : ""
        fax.text = place ? place.primaryFax : ""
        email.text = place ? place.primaryEmail : ""
        website.text = place ? '<a href=\"' + place.primaryWebsite + '\">' + place.primaryWebsite + '</a>' : ""
        addInformation.text = place ? additonalInformation(place) : ""
        if (place) {
            editorialsButton.enabled = Qt.binding(function(){ return place && place.editorialModel.totalCount > 0 })
            reviewsButton.enabled = Qt.binding(function(){ return place && place.reviewModel.totalCount > 0 })
            imagesButton.enabled = Qt.binding(function(){ return place && place.imageModel.totalCount > 0 })
            findSimilarButton.enabled = true
        }
    }
}

