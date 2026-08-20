// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [3]
#include <QtGraphs>
#include <QtGraphsWidgets/q3dscatterwidgetitem.h>
#include <QtWidgets/qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    //! [0]
    QQuickWidget quickWidget;
    Q3DScatterWidgetItem scatter;
    scatter.setWidget(&quickWidget);
    scatter.widget()->setMinimumSize(QSize(256, 256));
    //! [0]
    //! [1]
    QScatter3DSeries series;
    QScatterDataArray data;
    data << QScatterDataItem(0.5f, 0.5f, 0.5f) << QScatterDataItem(-0.3f, -0.5f, -0.4f)
         << QScatterDataItem(0.0f, -0.3f, 0.2f);
    series.dataProxy()->addItems(data);
    scatter.addSeries(&series);
    //! [1]
    //! [2]
    scatter.widget()->show();
    //! [2]

    return app.exec();
}
//! [3]
