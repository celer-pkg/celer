// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "bargraph.h"
#include "scattergraph.h"
#include "surfacegraph.h"

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qtabwidget.h>
#include <QtGui/qscreen.h>

using namespace Qt::StringLiterals;

int main(int argc, char **argv)
{
    qputenv("QSG_RHI_BACKEND", "opengl");

    QApplication app(argc, argv);

    // Create a tab widget for creating own tabs for Q3DBars, Q3DScatter, and Q3DSurface
    QTabWidget tabWidget;
    tabWidget.setWindowTitle(u"Graph Gallery"_s);

    const QSize screenSize = tabWidget.screen()->size();
    const QSize minimumGraphSize{screenSize.width() / 2, qRound(screenSize.height() / 1.75)};

    // Create bar graph
    BarGraph bars;
    // Create scatter graph
    ScatterGraph scatter;
    // Create surface graph
    SurfaceGraph surface;

    if (!bars.initialize(minimumGraphSize, screenSize)
        || !scatter.initialize(minimumGraphSize, screenSize)
        || !surface.initialize(minimumGraphSize, screenSize)) {
        QMessageBox::warning(nullptr, u"Graph Gallery"_s, u"Couldn't initialize the OpenGL context."_s);
        return -1;
    }

    // Add bars widget
    tabWidget.addTab(bars.barsWidget(), u"Bar Graph"_s);
    // Add scatter widget
    tabWidget.addTab(scatter.scatterWidget(), u"Scatter Graph"_s);
    // Add surface widget
    tabWidget.addTab(surface.surfaceWidget(), u"Surface Graph"_s);

    tabWidget.show();
    return app.exec();
}
