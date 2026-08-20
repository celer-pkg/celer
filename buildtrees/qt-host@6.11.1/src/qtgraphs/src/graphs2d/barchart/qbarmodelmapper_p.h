// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
#ifndef QBARMODELMAPPER_P_H
#define QBARMODELMAPPER_P_H

#include <QtGraphs/QBarModelMapper>
#include <QtGraphs/QBarSet>
#include <private/qabstractitemmodel_p.h>
#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QAbstractItemModel;

class QBarModelMapperPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QBarModelMapper)
public:
    QBarModelMapperPrivate();
    ~QBarModelMapperPrivate() override;

protected:
    QModelIndex barModelIndex(qsizetype barSection, qsizetype posInBar);
    void blockSeriesSignals(const bool block = true);
    void blockModelSignals(const bool block = true);
    QBarSet *barSet(QModelIndex index);
    void insertData(qsizetype start, qsizetype end);
    void removeData(qsizetype start, qsizetype end);

private:
    QAbstractItemModel *m_model = nullptr;
    QBarSeries *m_series = nullptr;
    QList<QBarSet *> m_barSets;

    qsizetype m_firstBarSetSection = -1;
    qsizetype m_lastBarSetSection = -1;

    qsizetype m_count = -1;
    qsizetype m_first = 0;
    Qt::Orientation m_orientation = Qt::Vertical;
    bool m_seriesSignalsBlock = false;
    bool m_modelSignalsBlock = false;

private Q_SLOTS:
    void initializeBarsFromModel();

    // for the model
    void modelUpdated(QModelIndex topLeft, QModelIndex bottomRight);
    void modelHeaderDataUpdated(Qt::Orientation orientation, qsizetype first, qsizetype last);
    void modelRowsAdded(QModelIndex parent, qsizetype start, qsizetype end);
    void modelRowsRemoved(QModelIndex parent, qsizetype start, qsizetype end);
    void modelColumnsAdded(QModelIndex parent, qsizetype start, qsizetype end);
    void modelColumnsRemoved(QModelIndex parent, qsizetype start, qsizetype end);
    void handleModelDestroyed();

    // for the series
    void barSetsAdded(const QList<QBarSet *> &sets);
    void barSetsRemoved(const QList<QBarSet *> &sets);
    void valuesRemoved(qsizetype index, qsizetype count);
    void handleSeriesDestroyed();

private:
    void handleValuesAdded(QBarSet *set, qsizetype index, qsizetype count);
    void handleBarLabelChanged(QBarSet *set);
    void handleBarValueChanged(QBarSet *set, qsizetype index);

    Q_DISABLE_COPY_MOVE(QBarModelMapperPrivate)
};

QT_END_NAMESPACE

#endif // QBARMODELMAPPER_P_H
