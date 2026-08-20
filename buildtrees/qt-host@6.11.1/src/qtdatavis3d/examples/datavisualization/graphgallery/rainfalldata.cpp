// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "rainfalldata.h"
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/qbar3dseries.h>
#include <QtDataVisualization/q3dtheme.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qfile.h>

using namespace Qt::StringLiterals;

RainfallData::RainfallData()
{
    // In data file the months are in numeric format, so create custom list
    for (int i = 1; i <= 12; ++i)
        m_numericMonths << QString::number(i);

    m_columnCount = m_numericMonths.size();

    updateYearsList(2010, 2022);

    // Create proxy and series
    //! [0]
    m_proxy = new VariantBarDataProxy;
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
    QStringList months{"January", "February", "March", "April",
                       "May", "June", "July", "August", "September", "October",
                       "November","December"};
    m_rowAxis->setTitle("Year");
    m_colAxis->setTitle("Month");
    m_valueAxis->setTitle("rainfall (mm)");
    m_valueAxis->setSegmentCount(5);
    m_rowAxis->setLabels(m_years);
    m_colAxis->setLabels(months);
    m_rowAxis->setTitleVisible(true);
    m_colAxis->setTitleVisible(true);
    m_valueAxis->setTitleVisible(true);

    addDataSet();
}

RainfallData::~RainfallData()
{
    delete m_mapping;
    delete m_dataSet;
    delete m_proxy;
}

void RainfallData::updateYearsList(int start, int end)
{
    m_years.clear();

    for (int i = start; i <= end; ++i)
        m_years << QString::number(i);

    m_rowCount = m_years.size();
}

//! [1]
void RainfallData::addDataSet()
{
    // Create a new variant data set and data item list
    m_dataSet =  new VariantDataSet;
    auto *itemList = new VariantDataItemList;

    // Read data from a data file into the data item list
    QFile dataFile(":/data/raindata.txt");
    if (dataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&dataFile);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            if (line.startsWith('#')) // Ignore comments
                continue;
            const auto strList = QStringView{line}.split(',', Qt::SkipEmptyParts);
            // Each line has three data items: Year, month, and rainfall value
            if (strList.size() < 3) {
                qWarning() << "Invalid row read from data:" << line;
                continue;
            }
            // Store year and month as strings, and rainfall value as double
            // into a variant data item and add the item to the item list.
            auto *newItem = new VariantDataItem;
            for (int i = 0; i < 2; ++i)
                newItem->append(strList.at(i).trimmed().toString());
            newItem->append(strList.at(2).trimmed().toDouble());
            itemList->append(newItem);
        }
    } else {
        qWarning() << "Unable to open data file:" << dataFile.fileName();
    }
    //! [1]

    //! [2]
    // Add items to the data set and set it to the proxy
    m_dataSet->addItems(itemList);
    m_proxy->setDataSet(m_dataSet);

    // Create new mapping for the data and set it to the proxy
    m_mapping = new VariantBarDataMapping(0, 1, 2, m_years, m_numericMonths);
    m_proxy->setMapping(m_mapping);
    //! [2]
}
