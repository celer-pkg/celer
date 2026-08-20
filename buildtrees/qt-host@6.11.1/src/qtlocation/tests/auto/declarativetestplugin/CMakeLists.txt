qt_internal_add_qml_module(declarative_location_test
    URI QtLocation.Test
    VERSION ${PROJECT_VERSION}
    PLUGIN_TARGET declarative_location_test
    CLASS_NAME QLocationDeclarativeTestModule
    NO_GENERATE_PLUGIN_SOURCE
    NO_PLUGIN_OPTIONAL
    SOURCES
        qdeclarativepinchgenerator_p.h qdeclarativepinchgenerator.cpp
        qdeclarativelocationtestmodel_p.h qdeclarativelocationtestmodel.cpp
        testhelper.h
        locationtest.cpp
    LIBRARIES
        Qt::Core
        Qt::Qml
        Qt::Quick
        Qt::GuiPrivate
        Qt::Test
        Qt::Location
        Qt::Positioning
    NO_GENERATE_CPP_EXPORTS
)
