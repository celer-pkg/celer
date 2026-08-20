// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
#include <QtDataVisualization>
//! [0]

//! [1]
proxy->setItemLabelFormat(QStringLiteral("@valueTitle for (@rowLabel, @colLabel): %.1f"));
//! [1]

//! [2]
proxy->setItemLabelFormat(QStringLiteral("@xTitle: @xValue, @yTitle: @yValue, @zTitle: @zValue"));
//! [2]

//! [3]
// By defining row and column categories, you tell the mapping which row and column each item
// belongs to. The categories must match the data stored in the model in the roles you define
// for row and column mapping. In this example we expect "year" role to return four digit year
// and "month" to return three letter designation for the month.
//
// An example of an item in model would be:
// Requested role -> Returned data
// "year" -> "2006" // Matches the first row category, so this item is added to the first row.
// "month" -> "jan" // Matches the first column category, so this item is added as first item in the row.
// "income" -> "12.1"
// "expenses" -> "9.2"
QStringList years;
QStringList months;
years << "2006" << "2007" << "2008" << "2009" << "2010" << "2011" << "2012";
months << "jan" << "feb" << "mar" << "apr" << "may" << "jun" << "jul" << "aug" << "sep" << "oct" << "nov" << "dec";

QItemModelBarDataProxy *proxy = new QItemModelBarDataProxy(customModel,
                                                           QStringLiteral("year"), // Row role
                                                           QStringLiteral("month"), // Column role
                                                           QStringLiteral("income"), // Value role
                                                           years, // Row categories
                                                           months); // Column categories

//...

// To display different data later, you can simply change the mapping.
proxy->setValueRole(QStringLiteral("expenses"));
//! [3]

//! [4]
// Map "density" value to X-axis, "hardness" to Y-axis and "conductivity" to Z-axis.
QItemModelScatterDataProxy *proxy = new QItemModelScatterDataProxy(customModel,
                                                                   QStringLiteral("density"),
                                                                   QStringLiteral("hardness"),
                                                                   QStringLiteral("conductivity"));
//! [4]

//! [5]
QItemModelSurfaceDataProxy *proxy = new QItemModelSurfaceDataProxy(customModel,
                                                                   QStringLiteral("longitude"), // Row role
                                                                   QStringLiteral("latitude"), // Column role
                                                                   QStringLiteral("height")); // Y-position role
//! [5]

//! [6]
qmake
make
make install
//! [6]

//! [7]
qmake CONFIG+=static
make
make install
//! [7]

//! [8]
qmake
make
./qmlsurface
//! [8]

//! [9]
Q3DBars *graph = new Q3DBars();
QWidget *container = QWidget::createWindowContainer(graph);
//! [9]

//! [10]
Q3DBars graph;
QBarDataProxy *newProxy = new QBarDataProxy;

QBarDataArray *dataArray = new QBarDataArray;
dataArray->reserve(10);
for (int i = 0; i < 10; i++) {
    QBarDataRow *dataRow = new QBarDataRow(5);
    for (int j = 0; j < 5; j++)
        (*dataRow)[j].setValue(myData->getValue(i, j));
    dataArray->append(dataRow);
}

newProxy->resetArray(dataArray);
graph->addSeries(new QBar3DSeries(newProxy));
//! [10]

//! [11]
Q3DBars graph;
QBar3DSeries *series = new QBar3DSeries;
QLinearGradient barGradient(0, 0, 1, 100);
barGradient.setColorAt(1.0, Qt::white);
barGradient.setColorAt(0.0, Qt::black);

series->setBaseGradient(barGradient);
series->setColorStyle(Q3DTheme::ColorStyleObjectGradient);
series->setMesh(QAbstract3DSeries::MeshCylinder);

graph->addSeries(series);
//! [11]
