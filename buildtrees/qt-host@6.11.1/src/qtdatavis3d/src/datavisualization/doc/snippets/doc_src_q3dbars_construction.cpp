// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [3]
#include <QtDataVisualization>

int main(int argc, char **argv)
{
    qputenv("QSG_RHI_BACKEND", "opengl");
    QGuiApplication app(argc, argv);

    //! [4]
    Q3DBars bars;
    bars.setFlags(bars.flags() ^ Qt::FramelessWindowHint);
    //! [4]
    //! [0]
    bars.rowAxis()->setRange(0, 4);
    bars.columnAxis()->setRange(0, 4);
    //! [0]
    //! [1]
    QBar3DSeries *series = new QBar3DSeries;
    QBarDataRow *data = new QBarDataRow;
    *data << 1.0f << 3.0f << 7.5f << 5.0f << 2.2f;
    series->dataProxy()->addRow(data);
    bars.addSeries(series);
    //! [1]
    //! [2]
    bars.show();
    //! [2]

    return app.exec();
}
//! [3]
