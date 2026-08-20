# Generated from clipper.pro.

#####################################################################
## Bundled_Clipper Generic Library:
#####################################################################

qt_internal_add_3rdparty_library(Bundled_Clipper
    QMAKE_LIB_NAME _clipper
    STATIC
    SKIP_AUTOMOC # special case
    EXCEPTIONS
    SOURCES
        clipper.cpp clipper.h
)
qt_disable_warnings(Bundled_Clipper)
qt_set_symbol_visibility_hidden(Bundled_Clipper)

## Scopes:
#####################################################################

#### Keys ignored in scope 3:.:.:clipper.pro:GCC:
# QMAKE_CFLAGS_OPTIMIZE_FULL = "-ffast-math"

#### Keys ignored in scope 4:.:.:clipper.pro:NOT CLANG AND NOT ICC AND NOT rim_qcc:
# QMAKE_CXXFLAGS_WARN_ON = "-Wno-error=return-type"
