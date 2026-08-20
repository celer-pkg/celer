qt_add_library(MediaControls STATIC)

target_link_libraries(MediaControls PRIVATE Qt6::Quick)

set_source_files_properties(ControlImages.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE true
)

qt_add_qml_module(MediaControls
    URI "MediaControls"
    OUTPUT_DIRECTORY MediaControls
    QML_FILES
        "AudioControl.qml"
        "ControlImages.qml"
        "PlaybackSeekControl.qml"
        "PlaybackRateControl.qml"
        "PlaybackControl.qml"
        "CustomSlider.qml"
        "CustomButton.qml"
        "CustomRadioButton.qml"
    RESOURCES
        "icons/Rate_Icon.svg"
        "icons/Rate_Icon_Dark.svg"
        "icons/Loop_Icon.svg"
        "icons/Loop_Icon_Dark.svg"
        "icons/Play_Icon.svg"
        "icons/Previous_Icon.svg"
        "icons/Previous_Icon_Dark.svg"
        "icons/Next_Icon.svg"
        "icons/Next_Icon_Dark.svg"
        "icons/Shuffle_Icon.svg"
        "icons/Volume_Icon.svg"
        "icons/Volume_Icon_Dark.svg"
        "icons/Playlist_Icon.svg"
        "icons/Playlist_Icon_Dark.svg"
        "icons/Settings_Icon.svg"
        "icons/Settings_Icon_Dark.svg"
        "icons/FullScreen_Icon.svg"
        "icons/FullScreen_Icon_Dark.svg"
        "icons/Stop_Icon.svg"
        "icons/Loop_Playlist.svg"
        "icons/Single_Loop.svg"
        "icons/Playlist_Active.svg"
        "icons/Add_file.svg"
        "icons/Add_file_Dark.svg"
        "icons/Shuffle_Icon_Dark.svg"
        "icons/Shuffle_Active.svg"
        "icons/Cancel_Button.svg"
        "icons/Load_Button.svg"
        "icons/Menu_Icon.svg"
        "icons/Menu_Icon_Dark.svg"
        "icons/Mute_Icon.svg"
        "icons/Mute_Icon_Dark.svg"
        "icons/Error.svg"
        "icons/Trash_Icon.svg"
        "icons/Trash_Icon_Dark.svg"
    DEPENDENCIES TARGET Config
)
