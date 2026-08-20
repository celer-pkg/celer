# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_module(Quick3DGlslParserPrivate
    INTERNAL_MODULE
    DEFINES
        QT_BUILD_QUICK3DGLSLPARSER_PRIVATE_LIB
    SOURCES
        glsl_p.h
        glslast.cpp glslast_p.h
        glslastdump.cpp glslastdump_p.h
        glslastvisitor.cpp glslastvisitor_p.h
        glslengine.cpp glslengine_p.h
        glslkeywords.cpp
        glsllexer.cpp glsllexer_p.h
        glslmemorypool.cpp glslmemorypool_p.h
        glslparser.cpp glslparser_p.h
        glslparsertable.cpp glslparsertable_p.h
        glslsemantic.cpp glslsemantic_p.h
        glslsymbol.cpp glslsymbol_p.h
        glslsymbols.cpp glslsymbols_p.h
        glsltype.cpp glsltype_p.h
        glsltypes.cpp glsltypes_p.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
    PUBLIC_LIBRARIES
        Qt::Core
)
