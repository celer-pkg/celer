# Generated from clip2tri.pro.

#####################################################################
## Bundled_Clip2Tri Generic Library:
#####################################################################

qt_internal_add_3rdparty_library(Bundled_Clip2Tri
    QMAKE_LIB_NAME _clip2tri
    STATIC
    SKIP_AUTOMOC # special case
    EXCEPTIONS
    SOURCES
        clip2tri.cpp clip2tri.h
    INCLUDE_DIRECTORIES
        ../clipper
        ../poly2tri
    LIBRARIES
        Qt::Bundled_Clipper # special case
        Qt::Bundled_Poly2Tri # special case
)
qt_disable_warnings(Bundled_Clip2Tri)
qt_set_symbol_visibility_hidden(Bundled_Clip2Tri)

## Scopes:
#####################################################################

#### Keys ignored in scope 3:.:.:clip2tri.pro:GCC:
# QMAKE_CFLAGS_OPTIMIZE_FULL = "-ffast-math"

#### Keys ignored in scope 4:.:.:clip2tri.pro:NOT CLANG AND NOT ICC AND NOT rim_qcc:
# QMAKE_CXXFLAGS_WARN_ON = "-Wno-error=return-type"
