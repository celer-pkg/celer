qt_internal_add_test(tst_qgeoroute
    SOURCES
        tst_qgeoroute.h tst_qgeoroute.cpp
    LIBRARIES
        Qt::Core
        Qt::LocationPrivate
)
