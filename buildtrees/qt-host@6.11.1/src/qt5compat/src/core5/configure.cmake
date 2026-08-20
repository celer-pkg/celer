# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause



#### Inputs



#### Libraries
# special case begin
if(NOT TARGET ICU::i18n)
    qt_find_package(ICU 50.1 COMPONENTS i18n uc data PROVIDED_TARGETS ICU::i18n ICU::uc ICU::data MODULE_NAME qt5compat QMAKE_LIB icu)
endif()
# special case end

qt_find_package(WrapIconv PROVIDED_TARGETS WrapIconv::WrapIconv MODULE_NAME core5compat
    QMAKE_LIB iconv
    VCPKG_PORT libconv
    VCPKG_ADD_TO_FEATURE iconv
)


#### Tests



#### Features

qt_feature("iconv" PUBLIC PRIVATE
    SECTION "Internationalization"
    LABEL "iconv"
    PURPOSE "Provides internationalization on Unix."
    CONDITION NOT QT_FEATURE_icu AND QT_FEATURE_textcodec AND NOT WIN32 AND NOT QNX AND NOT ANDROID AND NOT APPLE AND WrapIconv_FOUND
)
qt_feature_definition("iconv" "QT_NO_ICONV" NEGATE VALUE "1")
qt_feature("textcodec" PUBLIC
    SECTION "Internationalization"
    LABEL "QTextCodec"
    PURPOSE "Supports conversions between text encodings."
)
qt_feature_definition("textcodec" "QT_NO_TEXTCODEC" NEGATE VALUE "1")
qt_feature("codecs" PUBLIC
    SECTION "Internationalization"
    LABEL "Codecs"
    PURPOSE "Supports non-unicode text conversions."
    CONDITION QT_FEATURE_textcodec
)
qt_feature_definition("codecs" "QT_NO_CODECS" NEGATE VALUE "1")
qt_feature("big_codecs" PUBLIC
    SECTION "Internationalization"
    LABEL "Big Codecs"
    PURPOSE "Supports big codecs, e.g. CJK."
    CONDITION QT_FEATURE_textcodec
)
qt_feature_definition("big_codecs" "QT_NO_BIG_CODECS" NEGATE VALUE "1")
qt_configure_add_summary_section(NAME "Qt 5 Compatibility Libraries")
qt_configure_add_summary_entry(ARGS "iconv")
qt_configure_end_summary_section() # end of "Qt 5 Compatibility Libraries" section
