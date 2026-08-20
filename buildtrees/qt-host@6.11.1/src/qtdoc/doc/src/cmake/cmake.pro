TEMPLATE = aux

QMAKE_DOCS = $$PWD/qtcmake.qdocconf

QTDIR = $$[QT_HOST_PREFIX]
exists($$QTDIR/.qmake.cache): \
    QMAKE_DOCS_OUTPUTDIR = $$QTDIR/doc/cmake
else: \
    QMAKE_DOCS_OUTPUTDIR = $$OUT_PWD/cmake
