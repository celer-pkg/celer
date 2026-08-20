INCLUDEPATH += ../../../include

LIBS += -L$$OUT_PWD/../../lib

TEMPLATE = app

QT += graphs

contains(TARGET, qml.*) {
    QT += qml quick
}

qtHaveModule(widgets) {
    QT += graphswidgets
}

target.path = $$[QT_INSTALL_TESTS]/graphs/$$TARGET
INSTALLS += target
