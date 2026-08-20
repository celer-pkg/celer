// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PIEGRAPH_H
#define PIEGRAPH_H

#include <QObject>
#include <QPieSeries>
#include <QPieSlice>

class PieGraph : public QObject
{
    Q_OBJECT
    //! [0]
    Q_PROPERTY(QPieSeries *pieSeries READ pieSeries WRITE setPieSeries NOTIFY pieSeriesChanged FINAL)
    //! [0]
public:
    explicit PieGraph(QObject *parent = nullptr);
    ~PieGraph();

    QPieSeries *pieSeries() const;
    void setPieSeries(QPieSeries *series);
    void appendSlice();
    void removeSlice();
    void explodeSlices();
    void clearSeries();
    void fillSliceInfo();
public Q_SLOTS:
    void onAddSlice();
    void onRemoveSlice();
    void onExplode();
    void onClearSeries();
Q_SIGNALS:
    void pieSeriesChanged();
private:
    QPieSeries *m_pieSeries;

    struct SliceInfo
    {
        QList<int> value;
        QList<QColor> color;
        QList<QString> label;
    } m_sliceInfo;
};

#endif // PIEGRAPH_H
