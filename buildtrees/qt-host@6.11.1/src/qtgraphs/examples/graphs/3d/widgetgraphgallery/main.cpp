// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "bargraph.h"
#include "scattergraph.h"
#include "surfacegraph.h"

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qtabwidget.h>
#include <QtWidgets/qwidget.h>

using namespace Qt::StringLiterals;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

// QTBUG-127884: crash on iOS when using QQuickWidget in QTabWidget
#ifndef Q_OS_IOS
    QTabWidget tabWidget;
#endif

    // Create bar graph
    BarGraph bars;

    // Create scatter graph
    ScatterGraph scatter;

    // Create surface graph
    SurfaceGraph surface;

// QTBUG-127884: crash on iOS when using QQuickWidget in QTabWidget
#ifndef Q_OS_IOS
    // Create a tab widget for creating own tabs for Q3DBarsWidgetItem, Q3DScatterWidgetItem, and Q3DSurfaceWidgetItem
    tabWidget.setWindowTitle(u"Graph Gallery"_s);

    // Add bars widget
    tabWidget.addTab(bars.barsWidget(), u"Bar Graph"_s);
    // Add scatter widget
    tabWidget.addTab(scatter.scatterWidget(), u"Scatter Graph"_s);
    // Add surface widget
    tabWidget.addTab(surface.surfaceWidget(), u"Surface Graph"_s);

    tabWidget.setMinimumHeight(bars.barsWidget()->height() * bars.barsWidget()->devicePixelRatioF());
    tabWidget.show();
#else
    bars.barsWidget()->show();
    // scatter.scatterWidget()->show();
    // surface.surfaceWidget()->show();
#endif

    return QCoreApplication::exec();
}
