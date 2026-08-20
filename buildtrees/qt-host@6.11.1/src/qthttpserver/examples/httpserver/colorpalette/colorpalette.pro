requires(qtHaveModule(httpserver))
requires(qtHaveModule(gui))
requires(qtHaveModule(concurrent))

TEMPLATE = app

QT += httpserver gui concurrent

SOURCES += \
    main.cpp

HEADERS += \
    apibehavior.h \
    types.h \
    utils.h

target.path = $$[QT_INSTALL_EXAMPLES]/httpserver/colorpalette
INSTALLS += target

RESOURCES += \
    assets/colors.json \
    assets/users.json \
    assets/sessions.json \
    assets/img/1-image.jpg \
    assets/img/2-image.jpg \
    assets/img/3-image.jpg \
    assets/img/4-image.jpg \
    assets/img/5-image.jpg \
    assets/img/6-image.jpg \
    assets/img/7-image.jpg \
    assets/img/8-image.jpg \
    assets/img/9-image.jpg \
    assets/img/10-image.jpg \
    assets/img/11-image.jpg \
    assets/img/12-image.jpg

CONFIG += cmdline
