// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtGui>

//! [0]
QAccessibleInterface *sliderFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = 0;

    if (classname == QLatin1String("QSlider") && object && object->isWidgetType())
        interface = new QAccessibleSlider(static_cast<QWidget *>(object));

    return interface;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QAccessible::installFactory(sliderFactory);
//! [0]

    QMainWindow mainWindow;
    mainWindow.show();

    return app.exec();
//! [1]
}
//! [1]
