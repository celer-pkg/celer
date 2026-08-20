#include "my_class.h"
#include <QTranslator>
#include <QFlags>

enum class MyEnum1: int {
    FIRST,
    SECOND,
    THIRD
};

MyClass::MyClass()
{
    const QString title = tr("My window title");
    setWindowTitle(title);
}

enum MyEnum2 : int { v1 = Qt::AlignLeft | Qt::AlignRight, v2 };

void MyClass::myMethod() {
    tr("translation in method");
}
