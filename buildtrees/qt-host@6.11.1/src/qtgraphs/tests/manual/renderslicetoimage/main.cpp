// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "bargraph.h"
#include "surfacegraph.h"

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qstackedwidget.h>
#include <QtWidgets/qcombobox.h>

using namespace Qt::StringLiterals;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWidget *widget = new QWidget;

    QStackedWidget *stackedWidget = new QStackedWidget;
    widget->setWindowTitle(u"Render slice to image example"_s);

    SurfaceGraph surface;
    stackedWidget->addWidget(surface.surfaceWidget());

    BarGraph bar;
    stackedWidget->addWidget(bar.barWidget());

    QComboBox *comboBox = new QComboBox;
    comboBox->addItem(u"Surface"_s);
    comboBox->addItem(u"Bar"_s);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(comboBox, 1);
    layout->addWidget(stackedWidget);

    QObject::connect(comboBox, &QComboBox::activated, stackedWidget, &QStackedWidget::setCurrentIndex);

    widget->setLayout(layout);
    widget->show();
    int retVal = app.exec();
    return retVal;
}
