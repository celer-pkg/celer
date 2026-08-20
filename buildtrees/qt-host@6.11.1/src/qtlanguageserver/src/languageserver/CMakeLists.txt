# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## LanguageServer Module:
#####################################################################

qt_internal_add_module(LanguageServerPrivate
    INTERNAL_MODULE
    STATIC
    SOURCES
      qlanguageserverprespectypes_p.h
      qlanguageserverspec_p.h
      qlanguageserverspectypes_p.h
      qlanguageserverjsonrpctransport_p.h qlanguageserverjsonrpctransport.cpp
      qtlanguageserverglobal.h
      qlanguageserverbase_p.h qlanguageserverbase_p_p.h qlanguageserverbase.cpp
      qlanguageservergen_p.h qlanguageservergen_p_p.h qlanguageservergen.cpp
      qlanguageserverprotocol_p.h qlanguageserverprotocol.cpp
      qlspnotifysignals_p.h qlspnotifysignals.cpp
    DEFINES
        QT_BUILD_LANGUAGESERVER_LIB
        QT_NO_CONTEXTLESS_CONNECT
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::JsonRpcPrivate
)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
elseif (MINGW)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
endif()
