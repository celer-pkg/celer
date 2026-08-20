// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtPositioning
import QtLocation
import "items"

ApplicationWindow {
    id: appWindow
    property MapView view
    property var parameters
    property var searchLocation: view ? view.map.center : QtPositioning.coordinate()
    property var searchRegion: QtPositioning.circle(searchLocation, 5000)
    property var searchRegionItem

    function getPlugins() {
        var plugin = Qt.createQmlObject('import QtLocation; Plugin {}', appWindow);
        var myArray = new Array;
        for (var i = 0; i < plugin.availableServiceProviders.length; i++) {
            var tempPlugin = Qt.createQmlObject ('import QtLocation; Plugin {name: "' + plugin.availableServiceProviders[i]+ '"}', appWindow)
            if (tempPlugin.supportsPlaces() && tempPlugin.supportsMapping() )
                myArray.push(tempPlugin.name)
        }
        myArray.sort()
        return myArray;
    }

    function initializeProviders(pluginParameters)
    {
        var parameters = []
        for (var prop in pluginParameters) {
            var parameter = Qt.createQmlObject('import QtLocation; PluginParameter{ name: "'+ prop + '"; value: "' + pluginParameters[prop]+'"}',appWindow)
            parameters.push(parameter)
        }
        appWindow.parameters = parameters
        var plugins = getPlugins()
        mainMenu.providerMenu.create(plugins)
        for (var i = 0; i<plugins.length; i++) {
            if (plugins[i] === "osm")
                mainMenu.selectProvider(plugins[i])
        }
    }

    function createMap(provider) {
        var plugin;
        if (parameters && parameters.length>0)
            plugin = Qt.createQmlObject ('import QtLocation; Plugin{ name:"' + provider + '"; parameters: appWindow.parameters}', appWindow)
        else
            plugin = Qt.createQmlObject ('import QtLocation; Plugin{ name:"' + provider + '"}', appWindow)

        if (view)
            view.destroy();
        view = mapComponent.createObject(page);
        view.map.plugin = plugin;
        view.map.zoomLevel = (view.map.maximumZoomLevel - view.map.minimumZoomLevel)/2
        categoryModel.plugin = plugin;
        categoryModel.update();
        placeSearchModel.plugin = plugin;
        suggestionModel.plugin = plugin;

        searchRegionItem = Qt.createQmlObject('import QtLocation; MapCircle { parent: view.map; color: "#46a2da"; border.color: "#190a33"; border.width: 2; opacity: 0.25 }', view.map);
        searchRegionItem.center = searchRegion.center;
        searchRegionItem.radius = searchRegion.radius;
        view.map.addMapItem(searchRegionItem);

    }

    title: qsTr("Places")
    width: 360
    height: 640
    visible: true
    menuBar: mainMenu
    header: searchBar

    MainMenu {
        id: mainMenu

        property var settings: ["Search Center", "Search Bounding Box", "Search Bounding Circle", "Search Options"]

        onSelectProvider: function (providerName) {
            stackView.pop(page)
            for (var i = 0; i < providerMenu.contentData.length; i++) {
                providerMenu.contentData[i].checked = providerMenu.contentData[i].text === providerName
            }
            createMap(providerName)
            if (view.map.error === Map.NoError) {
                settingsMenu.create(settings)
            } else {
                mainMenu.clearMenu(settingsMenu)
            }
        }

        onSelectSetting: function (setting) {
            stackView.pop({item:page,immediate: true})
            switch (setting) {
            case "Search Center":
                stackView.push(Qt.resolvedUrl("forms/SearchCenter.qml"), { "coordinate": view.map.center })
                stackView.currentItem.changeSearchCenter.connect(stackView.changeSearchCenter)
                stackView.currentItem.closeForm.connect(stackView.closeForm)
                break
            case "Search Bounding Box":
                stackView.push(Qt.resolvedUrl("forms/SearchBoundingBox.qml"), { "searchRegion": searchRegion })
                stackView.currentItem.changeSearchBoundingBox.connect(stackView.changeSearchBoundingBox)
                stackView.currentItem.closeForm.connect(stackView.closeForm)
                break
            case "Search Bounding Circle":
                stackView.push(Qt.resolvedUrl("forms/SearchBoundingCircle.qml"), { "searchRegion": searchRegion })
                stackView.currentItem.changeSearchBoundingCircle.connect(stackView.changeSearchBoundingCircle)
                stackView.currentItem.closeForm.connect(stackView.closeForm)
                break
            case "Search Options":
                stackView.push(Qt.resolvedUrl("forms/SearchOptions.qml"), { "plugin": view.map.plugin, "model": placeSearchModel })
                stackView.currentItem.changeSearchSettings.connect(stackView.changeSearchSettings)
                stackView.currentItem.closeForm.connect(stackView.closeForm)
                break
            default:
                console.log("Unsupported setting !")
            }
        }
    }

    //! [PlaceSearchSuggestionModel search text changed 1]
    SearchBar {
        id: searchBar
    //! [PlaceSearchSuggestionModel search text changed 1]
        width: appWindow.width
        searchBarVisbile: stackView.depth > 1 &&
                          stackView.currentItem &&
                          stackView.currentItem.objectName != "suggestionView" ? false : true
        onShowCategories: {
            if (view && view.map.plugin) {
                stackView.pop({item: page,immediate: true})
                stackView.enterCategory()
            }
        }
        onGoBack: stackView.pop()
    //! [PlaceSearchSuggestionModel search text changed 2]
        onSearchTextChanged: function (searchText) {
            if (searchText.length >= 3 && suggestionModel != null) {
                suggestionModel.searchTerm = searchText;
                suggestionModel.update();
            }
        }
    //! [PlaceSearchSuggestionModel search text changed 2]
        onDoSearch: function (searchText) {
            if (searchText.length > 0)
                placeSearchModel.searchForText(searchText);
        }

        onShowMap: stackView.pop(page)
    //! [PlaceSearchSuggestionModel search text changed 3]
    }
    //! [PlaceSearchSuggestionModel search text changed 3]

    StackView {
        id: stackView

        function showMessage(title,message,backPage)
        {
            stackView.push(Qt.resolvedUrl("forms/Message.qml") ,
                 {
                     "title" : title,
                     "message" : message,
                     "backPage" : backPage
                 })
            currentItem.closeForm.connect(closeMessage)
        }

        function closeMessage(backPage)
        {
            stackView.pop(backPage)
        }

        function closeForm()
        {
            stackView.pop(page)
        }

        function enterCategory(index)
        {
            stackView.push(Qt.resolvedUrl("views/CategoryView.qml"),
                 {
                     "categoryModel": categoryModel,
                     "rootIndex" : index
                 })
            currentItem.showSubcategories.connect(stackView.enterCategory)
            currentItem.searchCategory.connect(placeSearchModel.searchForCategory)
        }

        function showSuggestions()
        {
            if (currentItem.objectName !== "suggestionView") {
                stackView.pop(page)
                stackView.push(Qt.resolvedUrl("views/SuggestionView.qml"),
                     {
                          "suggestionModel": suggestionModel,
                          "width": stackView.width,
                          "height": stackView.height
                     })
                currentItem.objectName = "suggestionView"
                currentItem.suggestionSelected.connect(searchBar.showSearch)
                currentItem.suggestionSelected.connect(placeSearchModel.searchForText)
            }
        }

        function showPlaces()
        {
            if (currentItem.objectName !== "searchResultView") {
                stackView.pop({tem:page,immediate: true})
                stackView.push(Qt.resolvedUrl("views/SearchResultView.qml"),
                     {
                         "placeSearchModel": placeSearchModel,
                         "width": stackView.width,
                         "height": stackView.height
                     })
                currentItem.showPlaceDetails.connect(showPlaceDatails)
                currentItem.showMap.connect(searchBar.showMap)
                currentItem.objectName = "searchResultView"
            }
        }

        function showPlaceDatails(place, distance)
        {
            stackView.push(Qt.resolvedUrl("forms/PlaceDetails.qml") ,
                 {
                     "place": place,
                     "distanceToPlace": distance,
                     "width": stackView.width,
                     "height": stackView.height
                 })
            currentItem.searchForSimilar.connect(searchForSimilar)
            currentItem.showReviews.connect(showReviews)
            currentItem.showEditorials.connect(showEditorials)
            currentItem.showImages.connect(showImages)
        }

        function showEditorials(place)
        {
            stackView.push(Qt.resolvedUrl("views/EditorialView.qml"),
                 {
                    "place": place,
                    "width": stackView.width,
                    "height": stackView.height
                 })
            currentItem.showEditorial.connect(showEditorial)
        }

        function showReviews(place)
        {
            stackView.push(Qt.resolvedUrl("views/ReviewView.qml") ,
                 {
                    "place": place,
                    "width": stackView.width,
                    "height": stackView.height
                 })
            currentItem.showReview.connect(showReview)
        }

        function showImages(place)
        {
            stackView.push(Qt.resolvedUrl("views/ImageView.qml") ,
                 {
                     "place": place,
                     "width": stackView.width,
                     "height": stackView.height
                 })
        }

        function showEditorial(editorial)
        {
            stackView.push(Qt.resolvedUrl("views/EditorialPage.qml") ,
                 {
                     "editorial": editorial,
                     "width": stackView.width,
                     "height": stackView.height
                 })
        }

        function showReview(review)
        {
            stackView.push(Qt.resolvedUrl("views/ReviewPage.qml") ,
                 {
                     "review": review,
                     "width": stackView.width,
                     "height": stackView.height
                 })
        }

        function changeSearchCenter(coordinate)
        {
            stackView.pop(page)
            view.map.center = coordinate;
            if (searchRegionItem) {
                view.map.removeMapItem(searchRegionItem);
                searchRegionItem.destroy();
            }
        }

        function changeSearchBoundingBox(coordinate,widthDeg,heightDeg)
        {
            stackView.pop(page)
            view.map.center = coordinate
            searchRegion = QtPositioning.rectangle(view.map.center, widthDeg, heightDeg)
            if (searchRegionItem) {
                view.map.removeMapItem(searchRegionItem);
                searchRegionItem.destroy();
            }
            searchRegionItem = Qt.createQmlObject('import QtLocation; MapRectangle { parent: view.map; color: "#46a2da"; border.color: "#190a33"; border.width: 2; opacity: 0.25 }', page);
            searchRegionItem.topLeft = searchRegion.topLeft;
            searchRegionItem.bottomRight = searchRegion.bottomRight;
            view.map.addMapItem(searchRegionItem);
        }

        function changeSearchBoundingCircle(coordinate,radius)
        {
            stackView.pop(page)
            view.map.center = coordinate;
            searchRegion = QtPositioning.circle(coordinate, radius)

            if (searchRegionItem) {
                view.map.removeMapItem(searchRegionItem);
                searchRegionItem.destroy();
            }
            searchRegionItem = Qt.createQmlObject('import QtLocation; MapCircle { parent: view.map; color: "#46a2da"; border.color: "#190a33"; border.width: 2; opacity: 0.25 }', page);
            searchRegionItem.center = searchRegion.center;
            searchRegionItem.radius = searchRegion.radius;
            view.map.addMapItem(searchRegionItem);
        }

        function changeSearchSettings(orderByDistance, orderByName, locales)
        {
            stackView.pop(page)
            placeSearchModel.favoritesPlugin = null;

            placeSearchModel.relevanceHint = orderByDistance ? PlaceSearchModel.DistanceHint :
                                                               orderByName ? PlaceSearchModel.LexicalPlaceNameHint :
                                                                             PlaceSearchModel.UnspecifiedHint;
            view.map.plugin.locales = locales.split(Qt.locale().groupSeparator);
        }

        //! [PlaceRecommendationModel search]
        function searchForSimilar(place) {
            stackView.pop(page)
            searchBar.showSearch(place.name)
            placeSearchModel.searchForRecommendations(place.placeId);
        }
        //! [PlaceRecommendationModel search]

        anchors.fill: parent
        focus: true
        initialItem:  Item {
            id: page

            //! [PlaceSearchModel model]
            PlaceSearchModel {
                id: placeSearchModel
                searchArea: searchRegion

                function searchForCategory(category) {
                    searchTerm = "";
                    categories = category;
                    recommendationId = "";
                    searchArea = searchRegion
                    limit = -1;
                    update();
                }

                function searchForText(text) {
                    searchTerm = text;
                    categories = null;
                    recommendationId = "";
                    searchArea = searchRegion
                    limit = -1;
                    update();
                }

                function searchForRecommendations(placeId) {
                    searchTerm = "";
                    categories = null;
                    recommendationId = placeId;
                    searchArea = null;
                    limit = -1;
                    update();
                }

                onStatusChanged: {
                    switch (status) {
                    case PlaceSearchModel.Ready:
                        if (count > 0)
                            stackView.showPlaces()
                        else
                            stackView.showMessage(qsTr("Search Place Error"),qsTr("Place not found !"))
                        break;
                    case PlaceSearchModel.Error:
                        stackView.showMessage(qsTr("Search Place Error"),errorString())
                        break;
                    }
                }
            }
            //! [PlaceSearchModel model]

            //! [PlaceSearchSuggestionModel model]
            PlaceSearchSuggestionModel {
                id: suggestionModel
                searchArea: searchRegion

                onStatusChanged: {
                    if (status == PlaceSearchSuggestionModel.Ready)
                        stackView.showSuggestions()
                }
            }
            //! [PlaceSearchSuggestionModel model]

            //! [CategoryModel model]
            CategoryModel {
                id: categoryModel
                hierarchical: true
            }
            //! [CategoryModel model]

            Component {
                id: mapComponent

                MapComponent {
                    width: page.width
                    height: page.height
                    id: view

                    map.onErrorChanged: {
                        if (map.error !== Map.NoError) {
                            var title = qsTr("ProviderError");
                            var message = map.errorString + "<br/><br/><b>" + qsTr("Try to select other provider") + "</b>";
                            if (map.error === Map.MissingRequiredParameterError)
                                message += "<br/>" + qsTr("or see") + " \'mapviewer --help\' "
                                        + qsTr("how to pass plugin parameters.");
                            stackView.showMessage(title,message);
                        }
                    }

                    MapItemView {
                        model: placeSearchModel
                        parent: view.map

                        delegate: MapQuickItem {
                            coordinate: model.type === PlaceSearchModel.PlaceResult ? place.location.coordinate : QtPositioning.coordinate()
                            visible: model.type === PlaceSearchModel.PlaceResult

                            anchorPoint.x: image.width * 0.5
                            anchorPoint.y: image.height * 0.5

                            sourceItem: Column {
                                TapHandler {
                                    onTapped: stackView.showPlaceDatails(model.place,model.distance)
                                }
                                HoverHandler {
                                    cursorShape: Qt.PointingHandCursor
                                }

                                Image {
                                    id: image
                                    source: place.icon.url(Qt.size(64,64))
                                    anchors.horizontalCenter: parent.horizontalCenter;
                                }
                                Text {
                                    id: text
                                    text: title;
                                    font.bold: true
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        color: "white"
        opacity: busyIndicator.running ? 0.8 : 0
        anchors.fill: parent
        Behavior on opacity { NumberAnimation{} }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: placeSearchModel.status == PlaceSearchModel.Loading ||
                 categoryModel.status === CategoryModel.Loading
    }
}
