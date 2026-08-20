// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/qtabwidget.h>
#include "magneticfield.h"
#include "customitemrotation.h"

int main(int argc, char **argv)
{

    QApplication app(argc, argv);
    MagneticField magneticField;
    CustomItemRotation customItemRotation;

    QTabWidget tabWidget;

    tabWidget.addTab(magneticField.widget(), "Magnetic field");
    tabWidget.addTab(customItemRotation.widget(), "Custom item rotation");
    tabWidget.show();

    int retVal = app.exec();
    return retVal;
}
