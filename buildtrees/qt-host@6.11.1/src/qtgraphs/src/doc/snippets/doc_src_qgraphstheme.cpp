// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#include <QtGraphsWidgets/q3dbarswidgetitem.h>
#include <QtGraphs/qgraphstheme.h>
#include <QtWidgets/qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    //! [1]
    //! [0]
    QGraphsTheme *theme = new QGraphsTheme();
    //! [0]
    theme->setBackgroundVisible(false);
    theme->setLabelBackgroundVisible(false);
    //! [1]

    //! [2]
    theme->setTheme(QGraphsTheme::Theme::UserDefined);
    theme->setBackgroundColor(QColor(QRgb(0x99ca53)));
    theme->setBackgroundVisible(true);
    QList<QColor> colors = { QColor(QRgb(0x209fdf)) };
    theme->setSeriesColors(colors);
    theme->setColorStyle(QGraphsTheme::ColorStyle::Uniform);
    theme->setLabelFont(QFont(QStringLiteral("Impact"), 35));
    theme->setGridVisible(true);
    auto gridline = theme->grid();
    gridline.setMainColor(QColor(QRgb(0x99ca53)));
    theme->setGrid(gridline);
    theme->setLabelBackgroundColor(QColor(0xf6, 0xa6, 0x25, 0xa0));
    theme->setLabelBackgroundVisible(true);
    theme->setLabelBorderVisible(true);
    theme->setLabelTextColor(QColor(QRgb(0x404044)));
    theme->setMultiHighlightColor(QColor(QRgb(0x6d5fd5)));
    theme->setSingleHighlightColor(QColor(QRgb(0xf6a625)));
    theme->setBackgroundColor(QColor(QRgb(0xffffff)));
    //! [2]

    //! [3]
    QQuickWidget quickWidget;
    Q3DBarsWidgetItem bars;
    bars.setWidget(&quickWidget);
    bars.widget()->setMinimumSize(QSize(512, 512));
    bars.activeTheme()->setTheme(QGraphsTheme::Theme::MixSeries);
    QList<QColor> color = { QColor(Qt::red) };
    bars.activeTheme()->setSeriesColors(color);
    bars.activeTheme()->setSingleHighlightColor(Qt::yellow);
    //! [3]

    QBar3DSeries series;
    QBarDataRow data;
    data << QBarDataItem(1.0f) << QBarDataItem(3.0f) << QBarDataItem(7.5f) << QBarDataItem(5.0f)
         << QBarDataItem(2.2f);
    series.dataProxy()->addRow(data);
    bars.addSeries(&series);

    bars.widget()->show();

    return app.exec();
}
