// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtLocation
import QtPositioning

SearchOptionsForm {
    id: root
    property Plugin plugin
    property PlaceSearchModel model

    signal changeSearchSettings(bool orderByDistance,
                                bool orderByName,
                                string locales)
    signal closeForm()

    setButton.onClicked: changeSearchSettings(distanceOrderButton.checked,
                                              nameOrderButton.checked,
                                              locales.text)

    clearButton.onClicked: {
        locales.text = ""
        distanceOrderButton.checked = false
        nameOrderButton.checked = false
    }

    cancelButton.onClicked: {
        closeForm()
    }

    Component.onCompleted: {
          locales.visible = root.plugin !== null && root.plugin.supportsPlaces(Plugin.LocalizedPlacesFeature);
          favoritesButton.visible = false;
//          favoritesButton.enabled = placeSearchModel.favoritesPlugin !== null)
//                        isFavoritesEnabled = true;
          locales.text = root.plugin.locales.join(Qt.locale().groupSeparator);
          distanceOrderButton.checked = model.relevanceHint === PlaceSearchModel.DistanceHint
          nameOrderButton.checked = model.relevanceHint === PlaceSearchModel.LexicalPlaceNameHint
          unspecifiedButton.checked = model.relevanceHint === PlaceSearchModel.UnspecifiedHint
    }
}
