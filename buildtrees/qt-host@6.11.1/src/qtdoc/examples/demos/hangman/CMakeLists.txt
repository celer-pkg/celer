# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#####################################################################
## hangman Binary:
#####################################################################

cmake_minimum_required(VERSION 3.16)
project(InAppPurchasingDemo LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(OUTPUT_DIRECTORY "${INSTALL_EXAMPLESDIR}/demos/hangman")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_add_executable(hangman
    MANUAL_FINALIZATION
    hangmangame.cpp hangmangame.h
    main.cpp
    purchasing/inapp/inappproduct.cpp purchasing/inapp/inappproduct.h
    purchasing/inapp/inapppurchasebackend.cpp purchasing/inapp/inapppurchasebackend.h
    purchasing/inapp/inappstore.cpp purchasing/inapp/inappstore.h
    purchasing/inapp/inapptransaction.cpp purchasing/inapp/inapptransaction.h
    purchasing/qmltypes/inappproductqmltype.cpp purchasing/qmltypes/inappproductqmltype.h
    purchasing/qmltypes/inappstoreqmltype.cpp purchasing/qmltypes/inappstoreqmltype.h
)

target_include_directories(hangman PRIVATE
    purchasing/inapp
    purchasing/qmltypes
)

qt_add_qml_module(hangman
    URI examples.Hangman
    NO_RESOURCE_TARGET_PATH
    QML_FILES
      qml/GameView.qml
      qml/GuessWordView.qml
      qml/Hangman.qml
      qml/HowToView.qml
      qml/Key.qml
      qml/Letter.qml
      qml/LetterSelector.qml
      qml/MainView.qml
      qml/PageHeader.qml
      qml/ScoreItem.qml
      qml/SimpleButton.qml
      qml/SplashScreen.qml
      qml/StoreItem.qml
      qml/StoreView.qml
      qml/Word.qml
      main.qml
)

qt_add_resources(hangman "resources"
    PREFIX
        "/"
    FILES
        dict.txt
)

if(ANDROID)
    set_property(TARGET hangman APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/purchasing/android/
    )

    target_sources(hangman PRIVATE
        purchasing/android/androidinappproduct.cpp purchasing/android/androidinappproduct.h
        purchasing/android/androidinapppurchasebackend.cpp purchasing/android/androidinapppurchasebackend.h
        purchasing/android/androidinapptransaction.cpp purchasing/android/androidinapptransaction.h
        purchasing/android/androidjni.cpp
    )
endif()

if(IOS)
    target_sources(hangman PRIVATE
            purchasing/ios/iosinapppurchasebackend.h purchasing/ios/iosinapppurchasebackend.mm
            purchasing/ios/iosinapppurchaseproduct.h purchasing/ios/iosinapppurchaseproduct.mm
            purchasing/ios/iosinapppurchasetransaction.h purchasing/ios/iosinapppurchasetransaction.mm
    )

    target_link_libraries(hangman PRIVATE
        "-framework Foundation"
        "-framework StoreKit"
    )
endif()

target_link_libraries(hangman PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

qt_finalize_executable(hangman)

install(TARGETS hangman
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET hangman
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
