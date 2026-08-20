# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(StocQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 ${PROJECT_VERSION} REQUIRED COMPONENTS Core Gui Qml Quick Graphs)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appStocQt
    src/main.cpp
)

option(QT_ANDROID_FETCH_OPENSSL "Fetch Android OpenSSL support for Qt from gitlab" OFF)

if (ANDROID)
    if (QT_ANDROID_FETCH_OPENSSL)
        include(FetchContent)
        FetchContent_Declare(
            android_openssl
            DOWNLOAD_EXTRACT_TIMESTAMP true
            URL      https://github.com/KDAB/android_openssl/archive/refs/heads/master.zip
        )

        FetchContent_MakeAvailable(android_openssl)
        include(${android_openssl_SOURCE_DIR}/android_openssl.cmake)
        add_android_openssl_libraries(appStocQt)
    else()
        message(WARNING "android_openssl.cmake not found. Live data will not be available.
            Consider setting QT_ANDROID_FETCH_OPENSSL=TRUE")
    endif()
endif()

target_include_directories(appStocQt PRIVATE src)

qt_add_qml_module(appStocQt
    URI StocQt
    NO_RESOURCE_TARGET_PATH
    VERSION 1.0
    QML_FILES Main.qml
    SOURCES
        src/apihandler.cpp src/apihandler.h
        src/favoritesmodel.cpp src/favoritesmodel.h
        src/stockengine.cpp src/stockengine.h
        src/stocklistmodel.cpp src/stocklistmodel.h
        src/stockmodel.cpp src/stockmodel.h
        src/timeformatter.cpp src/timeformatter.h
)
qt_add_resources(appStocQt "configuration"
    PREFIX "/"
    FILES
        qtquickcontrols2.conf
    )

add_subdirectory(content)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appStocQt PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appStockQt
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appStocQt PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Graphs
)

set (data_resource_files
    "data/AAPL.json"
    "data/ABNB.json"
    "data/ADBE.json"
    "data/ADI.json"
    "data/ADP.json"
    "data/ADSK.json"
    "data/AEP.json"
    "data/ALGN.json"
    "data/AMAT.json"
    "data/AMD.json"
    "data/AMGN.json"
    "data/AMZN.json"
    "data/ANSS.json"
    "data/ASML.json"
    "data/ATVI.json"
    "data/AVGO.json"
    "data/AZN.json"
    "data/BIIB.json"
    "data/BKNG.json"
    "data/BKR.json"
    "data/CDNS.json"
    "data/CEG.json"
    "data/CHTR.json"
    "data/CMCSA.json"
    "data/COST.json"
    "data/CPRT.json"
    "data/CRWD.json"
    "data/CSCO.json"
    "data/CSGP.json"
    "data/CSX.json"
    "data/CTAS.json"
    "data/CTSH.json"
    "data/DDOG.json"
    "data/DLTR.json"
    "data/DXCM.json"
    "data/EA.json"
    "data/EBAY.json"
    "data/ENPH.json"
    "data/EXC.json"
    "data/FANG.json"
    "data/FAST.json"
    "data/FISV.json"
    "data/FTNT.json"
    "data/GFS.json"
    "data/GILD.json"
    "data/GOOG.json"
    "data/GOOGL.json"
    "data/HON.json"
    "data/IDXX.json"
    "data/ILMN.json"
    "data/INTC.json"
    "data/INTU.json"
    "data/ISRG.json"
    "data/JD.json"
    "data/KDP.json"
    "data/KHC.json"
    "data/KLAC.json"
    "data/LCID.json"
    "data/LRCX.json"
    "data/LULU.json"
    "data/MAR.json"
    "data/MCHP.json"
    "data/MDLZ.json"
    "data/MELI.json"
    "data/META.json"
    "data/MNST.json"
    "data/MRNA.json"
    "data/MRVL.json"
    "data/MSFT.json"
    "data/MU.json"
    "data/NFLX.json"
    "data/NVDA.json"
    "data/NXPI.json"
    "data/ODFL.json"
    "data/ORLY.json"
    "data/PANW.json"
    "data/PAYX.json"
    "data/PCAR.json"
    "data/PDD.json"
    "data/PEP.json"
    "data/PYPL.json"
    "data/QCOM.json"
    "data/REGN.json"
    "data/RIVN.json"
    "data/ROST.json"
    "data/SBUX.json"
    "data/SGEN.json"
    "data/SIRI.json"
    "data/SNPS.json"
    "data/TEAM.json"
    "data/TMUS.json"
    "data/TSLA.json"
    "data/TXN.json"
    "data/VRSK.json"
    "data/VRTX.json"
    "data/WBA.json"
    "data/WBD.json"
    "data/WDAY.json"
    "data/XEL.json"
    "data/ZM.json"
    "data/ZS.json"
    "data/QTCOM.HE.json"
    "data/quotes.json"
)

qt6_add_resources(appStocQt "data"
    PREFIX "/"
    FILES
        ${data_resource_files}
)

install(TARGETS appStocQt
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET appStocQt
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
