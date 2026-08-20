# Generated from places.pro.

cmake_minimum_required(VERSION 3.16)
project(qml_location_places LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/location/places")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS Positioning)
find_package(Qt6 COMPONENTS Location)

qt_add_executable(qml_location_places WIN32 MACOSX_BUNDLE
    main.cpp
)
target_link_libraries(qml_location_places PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Location
    Qt::Positioning
    Qt::Qml
    Qt::Quick
)


# Resources:
set(places_resource_files
    "forms/Message.qml"
    "forms/MessageForm.ui.qml"
    "forms/PlaceDetails.qml"
    "forms/PlaceDetailsForm.ui.qml"
    "forms/SearchBoundingBox.qml"
    "forms/SearchBoundingBoxForm.ui.qml"
    "forms/SearchBoundingCircle.qml"
    "forms/SearchBoundingCircleForm.ui.qml"
    "forms/SearchCenter.qml"
    "forms/SearchCenterForm.ui.qml"
    "forms/SearchOptions.qml"
    "forms/SearchOptionsForm.ui.qml"
    "helper.js"
    "items/MainMenu.qml"
    "items/MapComponent.qml"
    "items/SearchBar.qml"
    "places.qml"
    "resources/categories.png"
    "resources/left.png"
    "resources/marker.png"
    "resources/right.png"
    "resources/scale.png"
    "resources/scale_end.png"
    "resources/search.png"
    "resources/star.png"
    "views/CategoryDelegate.qml"
    "views/CategoryView.qml"
    "views/EditorialDelegate.qml"
    "views/EditorialPage.qml"
    "views/EditorialView.qml"
    "views/ImageView.qml"
    "views/RatingView.qml"
    "views/ReviewDelegate.qml"
    "views/ReviewPage.qml"
    "views/ReviewView.qml"
    "views/SearchResultDelegate.qml"
    "views/SearchResultView.qml"
    "views/SuggestionView.qml"
)

qt6_add_resources(qml_location_places "places"
    PREFIX
        "/"
    FILES
        ${places_resource_files}
)

install(TARGETS qml_location_places
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
