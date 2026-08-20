# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from positioning.pro.

#####################################################################
## Positioning Module:
#####################################################################

qt_internal_add_module(Positioning
    PLUGIN_TYPES position
    SOURCES
        qclipperutils.cpp qclipperutils_p.h
        qdoublematrix4x4.cpp qdoublematrix4x4_p.h
        qdoublevector2d.cpp qdoublevector2d_p.h
        qdoublevector3d.cpp qdoublevector3d_p.h
        qgeoaddress.cpp qgeoaddress.h qgeoaddress_p.h
        qgeoareamonitorinfo.cpp qgeoareamonitorinfo.h
        qgeoareamonitorsource.cpp qgeoareamonitorsource.h
        qgeocircle.cpp qgeocircle.h qgeocircle_p.h
        qgeocoordinate.cpp qgeocoordinate.h qgeocoordinate_p.h
        qgeocoordinateobject.cpp qgeocoordinateobject_p.h
        qgeolocation.cpp qgeolocation.h qgeolocation_p.h
        qgeopath.cpp qgeopath.h qgeopath_p.h
        qgeopolygon.cpp qgeopolygon.h qgeopolygon_p.h
        qgeopositioninfo.cpp qgeopositioninfo.h qgeopositioninfo_p.h
        qgeopositioninfosource.cpp qgeopositioninfosource.h qgeopositioninfosource_p.h
        qgeopositioninfosourcefactory.cpp qgeopositioninfosourcefactory.h
        qgeorectangle.cpp qgeorectangle.h qgeorectangle_p.h
        qgeosatelliteinfo.cpp qgeosatelliteinfo.h qgeosatelliteinfo_p.h
        qgeosatelliteinfosource.cpp qgeosatelliteinfosource.h qgeosatelliteinfosource_p.h
        qgeoshape.cpp qgeoshape.h qgeoshape_p.h
        qlocationutils.cpp qlocationutils_p.h
        qnmeapositioninfosource.cpp qnmeapositioninfosource.h qnmeapositioninfosource_p.h
        qnmeasatelliteinfosource.cpp qnmeasatelliteinfosource.h qnmeasatelliteinfosource_p.h
        qpositioningglobal.h qpositioningglobal_p.h
        qwebmercator.cpp qwebmercator_p.h
    INCLUDE_DIRECTORIES
        ../3rdparty/clip2tri
        ../3rdparty/clipper
    LIBRARIES
        Qt::CorePrivate
        Qt::Bundled_Clip2Tri
    PUBLIC_LIBRARIES
        Qt::Core
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
    DEFINES
        QT_NO_CONTEXTLESS_CONNECT
)

if(ANDROID)
    set_property(TARGET Positioning APPEND PROPERTY QT_ANDROID_BUNDLED_JAR_DEPENDENCIES
        jar/Qt${QtPositioning_VERSION_MAJOR}AndroidPositioning.jar
    )
    set_property(TARGET Positioning APPEND PROPERTY QT_ANDROID_LIB_DEPENDENCIES
        ${INSTALL_PLUGINSDIR}/position/libplugins_position_qtposition_android.so
    )
    qt_internal_add_android_permission(Positioning
        NAME android.permission.ACCESS_FINE_LOCATION
    )
endif()

qt_internal_extend_target(Positioning CONDITION WIN32
    NO_UNITY_BUILD_SOURCES
        qclipperutils.cpp # `using namespace std` in clip2tri.h results in 'byte': ambiguous symbol
)

#### Keys ignored in scope 1:.:.:positioning.pro:<TRUE>:
# ANDROID_FEATURES = "android.hardware.location.gps"
# MODULE_WINRT_CAPABILITIES_DEVICE = "location"
# OTHER_FILES = "configure.json" "doc/src/*.qdoc" "doc/src/plugins/*.qdoc"
qt_internal_add_docs(Positioning
    doc/qtpositioning.qdocconf
)
