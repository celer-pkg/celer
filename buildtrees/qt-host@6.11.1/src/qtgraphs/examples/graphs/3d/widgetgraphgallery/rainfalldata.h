// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef RAINFALLDATA_H
#define RAINFALLDATA_H

#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>

QT_FORWARD_DECLARE_CLASS(QBar3DSeries)
QT_FORWARD_DECLARE_CLASS(QItemModelBarDataProxy)
QT_FORWARD_DECLARE_CLASS(QBar3DSeries)
QT_FORWARD_DECLARE_CLASS(QValue3DAxis)
QT_FORWARD_DECLARE_CLASS(QCategory3DAxis)

class RainfallData : public QObject
{
    Q_OBJECT
public:
    Q_DISABLE_COPY_MOVE(RainfallData)

    explicit RainfallData();
    ~RainfallData() override;

    //! [0]
    QBar3DSeries *customSeries() { return m_series; }
    //! [0]

    QValue3DAxis *valueAxis() { return m_valueAxis; }
    QCategory3DAxis *rowAxis() { return m_rowAxis; }
    QCategory3DAxis *colAxis() { return m_colAxis; }

private:
    void updateYearsList(int start, int end);

    QStringList m_years;
    QItemModelBarDataProxy *m_proxy;
    QBar3DSeries *m_series;
    QValue3DAxis *m_valueAxis;
    QCategory3DAxis *m_rowAxis;
    QCategory3DAxis *m_colAxis;
};

#endif
