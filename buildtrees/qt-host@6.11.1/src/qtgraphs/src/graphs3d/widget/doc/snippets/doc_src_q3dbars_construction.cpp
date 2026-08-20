// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [3]
#include <QtGraphs>
#include <QtGraphsWidgets/q3dbarswidgetitem.h>
#include <QtWidgets/qapplication.h>
int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    //! [4]
    QQuickWidget quickWidget;
    Q3DBarsWidgetItem bars;
    bars.setWidget(&quickWidget);
    bars.widget()->setMinimumSize(QSize(256, 256));
    //! [4]
    //! [0]
    bars.rowAxis()->setRange(0, 4);
    bars.columnAxis()->setRange(0, 4);
    //! [0]
    //! [1]
    QBar3DSeries series;
    QBarDataRow data;
    data << QBarDataItem(1.0f) << QBarDataItem(3.0f) << QBarDataItem(7.5f) << QBarDataItem(5.0f)
         << QBarDataItem(2.2f);
    series.dataProxy()->addRow(data);
    bars.addSeries(&series);
    //! [1]
    //! [2]
    bars.widget()->show();
    //! [2]

    return app.exec();
}
//! [3]
