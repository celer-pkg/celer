# Generated from poly2tri.pro.

#####################################################################
## Bundled_Poly2Tri Generic Library:
#####################################################################

qt_internal_add_3rdparty_library(Bundled_Poly2Tri
    QMAKE_LIB_NAME _poly2tri
    STATIC
    SKIP_AUTOMOC # special case
    SOURCES
        common/shapes.cpp common/shapes.h
        common/utils.h
        poly2tri.h
        sweep/advancing_front.cpp sweep/advancing_front.h
        sweep/cdt.cpp sweep/cdt.h
        sweep/sweep.cpp sweep/sweep.h
        sweep/sweep_context.cpp sweep/sweep_context.h
)
qt_disable_warnings(Bundled_Poly2Tri)
qt_set_symbol_visibility_hidden(Bundled_Poly2Tri)

## Scopes:
#####################################################################

#### Keys ignored in scope 3:.:.:poly2tri.pro:GCC:
# QMAKE_CFLAGS_OPTIMIZE_FULL = "-ffast-math"

#### Keys ignored in scope 4:.:.:poly2tri.pro:NOT CLANG AND NOT ICC AND NOT rim_qcc:
# QMAKE_CXXFLAGS_WARN_ON = "-Wno-error=return-type"
