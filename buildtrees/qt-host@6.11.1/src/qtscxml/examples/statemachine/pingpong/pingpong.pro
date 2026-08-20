QT = statemachine core
CONFIG += cmdline

SOURCES = main.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/statemachine/statemachine/pingpong
INSTALLS += target
