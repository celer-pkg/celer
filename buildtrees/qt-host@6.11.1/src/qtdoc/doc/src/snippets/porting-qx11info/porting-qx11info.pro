lessThan(QT_MAJOR_VERSION, 6) {
   QT += x11extras
}

LIBS += -lxcb -lX11

HEADERS  = testwindow.h
SOURCES += testwindow.cpp \
    main.cpp
