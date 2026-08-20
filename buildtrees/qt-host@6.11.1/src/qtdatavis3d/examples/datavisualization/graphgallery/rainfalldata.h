// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef RAINFALLDATA_H
#define RAINFALLDATA_H

#include "variantbardataproxy.h"
#include <QtDataVisualization/qcategory3daxis.h>
#include <QtDataVisualization/qvalue3daxis.h>

class RainfallData : public QObject
{
    Q_OBJECT
public:
    explicit RainfallData();
    ~RainfallData();

    void addDataSet();

    //! [0]
    QBar3DSeries *customSeries() { return m_series; }
    //! [0]

    QValue3DAxis *valueAxis() { return m_valueAxis; }
    QCategory3DAxis *rowAxis() { return m_rowAxis; }
    QCategory3DAxis *colAxis() { return m_colAxis; }

private:
    void updateYearsList(int start, int end);
    int m_columnCount;
    int m_rowCount;
    QStringList m_years;
    QStringList m_numericMonths;
    VariantBarDataProxy *m_proxy;
    VariantBarDataMapping *m_mapping;
    VariantDataSet *m_dataSet;
    QBar3DSeries *m_series;
    QValue3DAxis *m_valueAxis;
    QCategory3DAxis *m_rowAxis;
    QCategory3DAxis *m_colAxis;
};

#endif
