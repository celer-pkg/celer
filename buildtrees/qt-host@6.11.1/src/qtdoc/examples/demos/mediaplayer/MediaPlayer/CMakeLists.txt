qt_add_library(MediaPlayer STATIC)

set_source_files_properties(Images.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE true
)

qt_add_qml_module(MediaPlayer
    URI "MediaPlayer"
    QML_FILES
        "Main.qml"
        "Images.qml"
        "MetadataInfo.qml"
        "PlayerMenuBar.qml"
        "TracksInfo.qml"
        "TracksOptions.qml"
        "PlaylistInfo.qml"
        "UrlPopup.qml"
        "SettingsInfo.qml"
        "ThemeInfo.qml"
        "ErrorPopup.qml"
        "TouchMenu.qml"
    RESOURCES
        "icons/Shadow.png"
        "icons/Shadow@2x.png"
        "icons/Default_CoverArt.svg"
        "icons/Movie_Active.svg"
        "icons/Movie_Icon_Dark.svg"
        "icons/Movie_Icon.svg"
        "icons/Music_Active.svg"
        "icons/Music_Icon_Dark.svg"
        "icons/Music_Icon.svg"
        "icons/Warning_Icon.svg"
    DEPENDENCIES
        TARGET Config
        TARGET MediaControls
)
