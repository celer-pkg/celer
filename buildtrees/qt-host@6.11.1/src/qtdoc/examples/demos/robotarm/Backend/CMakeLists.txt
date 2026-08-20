find_package(Qt6 REQUIRED COMPONENTS Gui)

qt_add_library(backendmodule STATIC)

qt_add_qml_module(backendmodule
    URI Backend
    SOURCES
        animatedparam.cpp
        animatedparam.h
        backend.cpp
        backend.h
    RESOURCE_PREFIX "/"
)

target_link_libraries(backendmodule PUBLIC Qt6::Gui)
