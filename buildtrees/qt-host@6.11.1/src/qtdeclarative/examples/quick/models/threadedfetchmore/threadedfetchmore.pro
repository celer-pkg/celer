TARGET = threadedfetchmore
QT += qml quick

HEADERS = fetchworker.h \
          threadedfetchmoremodel.h
SOURCES = main.cpp \
          fetchworker.cpp \
          threadedfetchmoremodel.cpp
RESOURCES += threadedfetchmore.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/quick/models/threadedfetchmore
INSTALLS += target
