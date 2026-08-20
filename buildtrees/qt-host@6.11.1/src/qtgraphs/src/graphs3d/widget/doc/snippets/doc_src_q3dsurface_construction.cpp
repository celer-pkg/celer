// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [5]
#include <QtGraphs>
#include <QtGraphsWidgets/q3dsurfacewidgetitem.h>
#include <QtWidgets/qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    //! [0]
    QQuickWidget quickWidget;
    Q3DSurfaceWidgetItem surface;
    surface.setWidget(&quickWidget);
    surface.widget()->setMinimumSize(QSize(256, 256));
    //! [0]
    //! [1]
    QSurfaceDataArray data;
    QSurfaceDataRow dataRow1;
    QSurfaceDataRow dataRow2;
    //! [1]

    //! [2]
    dataRow1 << QSurfaceDataItem(0.0f, 0.1f, 0.5f) << QSurfaceDataItem(1.0f, 0.5f, 0.5f);
    dataRow2 << QSurfaceDataItem(0.0f, 1.8f, 1.0f) << QSurfaceDataItem(1.0f, 1.2f, 1.0f);
    data << dataRow1 << dataRow2;
    //! [2]

    //! [3]
    QSurface3DSeries series;
    series.dataProxy()->resetArray(data);
    surface.addSeries(&series);
    //! [3]
    //! [4]
    surface.widget()->show();
    //! [4]

    return app.exec();
}
//! [5]
