import QtQuick

Item {
    property string qsTranslate: qsTranslate("context", "Hello")
    property string qsTranslateNoop: QT_TRANSLATE_NOOP("context", "Hello")
}
