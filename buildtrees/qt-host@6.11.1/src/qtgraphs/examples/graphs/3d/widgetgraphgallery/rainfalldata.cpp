// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:critical reason:data-parser

#include "rainfalldata.h"
#include <QtCore/qfile.h>
#include <QtCore/qrangemodel.h>
#include <QtCore/qlist.h>
#include <QtCore/qtextstream.h>
#include <QtGraphs/q3dscene.h>
#include <QtGraphs/qbar3dseries.h>
#include <QtGraphs/qcategory3daxis.h>
#include <QtGraphs/qgraphstheme.h>
#include <QtGraphs/qitemmodelbardataproxy.h>
#include <QtGraphs/qvalue3daxis.h>

#include <array>

using namespace Qt::StringLiterals;

//! [1]
using YearlyData = std::array<double, 12>;
using ModelData = QList<YearlyData>;

static ModelData readData(const QString &fileName, int *firstYear)
{
    ModelData result;

    *firstYear = -1;

    // Read data from a data file into the data item list
    QFile dataFile(fileName);
    if (!dataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Unable to open data file:" << dataFile.fileName() << dataFile.errorString();
        return result;
    }

    QTextStream stream(&dataFile);
    int lastYear = -1;
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.startsWith(u'#')) // Ignore comments
            continue;
        const auto strList = QStringView{line}.split(',', Qt::SkipEmptyParts);
        // Each line has three data items: Year, month, and rainfall value
        if (strList.size() < 3) {
            qWarning() << "Invalid row read from data:" << line;
            continue;
        }
        // Store year and month as int, and rainfall value as double
        bool yearOk{};
        bool monthOk{};
        bool valueOk{};
        const int year = strList.at(0).trimmed().toInt(&yearOk);
        const int month = strList.at(1).trimmed().toInt(&monthOk);
        const double value = strList.at(2).trimmed().toDouble(&valueOk);
        if (!yearOk || !monthOk || month < 1 || month > 12 || !valueOk) {
            qWarning() << "Invalid row values:" << line;
            continue;
        }
        if (year != lastYear) {
            if (lastYear == -1) {
                *firstYear = year;
            } else if (year != lastYear + 1) {
                qWarning() << "Non-consecutive years" << year << lastYear;
                return {};
            }
            lastYear = year;
            result.emplace_back(YearlyData{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
        }
        result.back()[month - 1] = value;
    }
    return result;
}
//! [1]

RainfallData::RainfallData()
{
    // Create proxy and series
    //! [0]
    int firstYear{};
    auto data = readData(":/data/raindata.txt"_L1, &firstYear);
    Q_ASSERT(!data.isEmpty());
    updateYearsList(firstYear, firstYear + int(data.size()) - 1);
    auto *model = new QRangeModel(data, this);

    m_proxy = new QItemModelBarDataProxy(model);
    m_proxy->setUseModelCategories(true);
    m_series = new QBar3DSeries(m_proxy);
    //! [0]

    m_series->setItemLabelFormat(u"%.1f mm"_s);

    // Create the axes
    m_rowAxis = new QCategory3DAxis(this);
    m_colAxis = new QCategory3DAxis(this);
    m_valueAxis = new QValue3DAxis(this);
    m_rowAxis->setAutoAdjustRange(true);
    m_colAxis->setAutoAdjustRange(true);
    m_valueAxis->setAutoAdjustRange(true);

    // Set axis labels and titles
    QStringList months{"January",
                       "February",
                       "March",
                       "April",
                       "May",
                       "June",
                       "July",
                       "August",
                       "September",
                       "October",
                       "November",
                       "December"};
    m_rowAxis->setTitle("Year");
    m_colAxis->setTitle("Month");
    m_valueAxis->setTitle("rainfall (mm)");
    m_valueAxis->setSegmentCount(5);
    m_rowAxis->setLabels(m_years);
    m_colAxis->setLabels(months);
    m_rowAxis->setTitleVisible(true);
    m_colAxis->setTitleVisible(true);
    m_valueAxis->setTitleVisible(true);
}

RainfallData::~RainfallData() = default;

void RainfallData::updateYearsList(int start, int end)
{
    m_years.clear();

    for (int i = start; i <= end; ++i)
        m_years << QString::number(i);
}
