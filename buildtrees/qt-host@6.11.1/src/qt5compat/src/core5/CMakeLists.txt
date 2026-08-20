# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from core5.pro.

#####################################################################
## Core5Compat Module:
#####################################################################

qt_internal_add_module(Core5Compat
    SOURCES
        codecs/qcodecmacros_p.h
        codecs/qtextcodec_p.h
        codecs/qutfcodec.cpp codecs/qutfcodec_p.h
        qcore5global.h
        sax/qxml.cpp sax/qxml.h sax/qxml_p.h
        serialization/qbinaryjson.cpp serialization/qbinaryjson.h serialization/qbinaryjson_p.h
        serialization/qbinaryjsonarray.cpp serialization/qbinaryjsonarray_p.h
        serialization/qbinaryjsonobject.cpp serialization/qbinaryjsonobject_p.h
        serialization/qbinaryjsonvalue.cpp serialization/qbinaryjsonvalue_p.h
        text/qregexp.cpp text/qregexp.h
        text/qstringref.cpp text/qstringref.h
        tools/qlinkedlist.cpp tools/qlinkedlist.h
    DEFINES
        QT_USE_FAST_OPERATOR_PLUS
        QT_USE_QSTRINGBUILDER
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate Qt::CorePrivate
    NO_GENERATE_CPP_EXPORTS
    ATTRIBUTION_FILE_DIR_PATHS
        codecs
)

#### Keys ignored in scope 1:.:.:core5.pro:<TRUE>:
# MODULE = "core5compat"
# QT_FOR_CONFIG = "core-private"

## Scopes:
#####################################################################

qt_internal_extend_target(Core5Compat CONDITION QT_FEATURE_textcodec
    SOURCES
        codecs/qlatincodec.cpp codecs/qlatincodec_p.h
        codecs/qsimplecodec.cpp codecs/qsimplecodec_p.h
        codecs/qtextcodec.cpp codecs/qtextcodec.h
)

qt_internal_extend_target(Core5Compat CONDITION QT_FEATURE_codecs AND QT_FEATURE_textcodec
    SOURCES
        codecs/qisciicodec.cpp codecs/qisciicodec_p.h
        codecs/qtsciicodec.cpp codecs/qtsciicodec_p.h
)

qt_internal_extend_target(Core5Compat CONDITION QT_FEATURE_icu AND QT_FEATURE_textcodec
    SOURCES
        codecs/qicucodec.cpp codecs/qicucodec_p.h
    LIBRARIES
        ICU::i18n ICU::uc ICU::data
)

qt_internal_extend_target(Core5Compat CONDITION QT_FEATURE_big_codecs AND QT_FEATURE_textcodec AND NOT QT_FEATURE_icu
    SOURCES
        codecs/qbig5codec.cpp codecs/qbig5codec_p.h
        codecs/qeucjpcodec.cpp codecs/qeucjpcodec_p.h
        codecs/qeuckrcodec.cpp codecs/qeuckrcodec_p.h
        codecs/qgb18030codec.cpp codecs/qgb18030codec_p.h
        codecs/qjiscodec.cpp codecs/qjiscodec_p.h
        codecs/qjpunicode.cpp codecs/qjpunicode_p.h
        codecs/qsjiscodec.cpp codecs/qsjiscodec_p.h
)

qt_internal_extend_target(Core5Compat CONDITION QT_FEATURE_iconv AND QT_FEATURE_textcodec AND NOT QT_FEATURE_icu
    SOURCES
        codecs/qiconvcodec.cpp codecs/qiconvcodec_p.h
    LIBRARIES
        WrapIconv::WrapIconv
)

qt_internal_extend_target(Core5Compat CONDITION QT_FEATURE_textcodec AND WIN32 AND NOT QT_FEATURE_icu
    SOURCES
        codecs/qwindowscodec.cpp codecs/qwindowscodec_p.h
)
qt_internal_add_docs(Core5Compat
    doc/qtcore5compat.qdocconf
)

