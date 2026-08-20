HEADERS       =
RESOURCES     = svgwidget.qrc
SOURCES       = main.cpp
QT           += widgets svg svgwidgets

CONFIG += console

# install
target.path = $$[QT_INSTALL_EXAMPLES]/svg/svgwidget
INSTALLS += target
