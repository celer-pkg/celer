message("Building designer components.")

set(QT_QDS_COMPONENTS_NOWARN on)
set(QT_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml")

include(FetchContent)
FetchContent_Declare(
    ds
    GIT_TAG qds-4.5
    GIT_REPOSITORY https://code.qt.io/qt-labs/qtquickdesigner-components.git
)

FetchContent_GetProperties(ds)
FetchContent_MakeAvailable(ds)

target_link_libraries(carconfigurator PRIVATE
    QuickStudioComponentsplugin
    QuickStudioDesignEffectsplugin
    QuickStudioEffectsplugin
    QuickStudioApplicationplugin
    FlowViewplugin
    QuickStudioLogicHelperplugin
    QuickStudioMultiTextplugin
    QuickStudioEventSimulatorplugin
    QuickStudioEventSystemplugin
    QuickStudioUtilsplugin
)
