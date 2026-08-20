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
#ifndef QXYMODELMAPPER_P_H
#define QXYMODELMAPPER_P_H

#include <QtGraphs/QXYModelMapper>
#include <private/qobject_p.h>
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcModelMapper2D)

class QXYModelMapperPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QXYModelMapper)
public:
    QXYModelMapperPrivate();
    ~QXYModelMapperPrivate() override;

public Q_SLOTS:
    // for the model
    void onModelUpdated(QModelIndex topLeft, QModelIndex bottomRight);
    void onModelRowsAdded(QModelIndex parent, qsizetype start, qsizetype end);
    void onModelRowsRemoved(QModelIndex parent, qsizetype start, qsizetype end);
    void onModelColumnsAdded(QModelIndex parent, qsizetype start, qsizetype end);
    void onModelColumnsRemoved(QModelIndex parent, qsizetype start, qsizetype end);
    void handleModelDestroyed();

    // for the series
    void onPointAdded(qsizetype pointIndex);
    void onPointRemoved(qsizetype pointIndex);
    void onPointsRemoved(qsizetype pointIndex, qsizetype count);
    void onPointReplaced(qsizetype pointIndex);
    void handleSeriesDestroyed();

    void initializeXYFromModel();

private:
    QModelIndex xModelIndex(qsizetype xIndex);
    QModelIndex yModelIndex(qsizetype yIndex);
    void insertData(int start, int end);
    void removeData(int start, int end);
    void blockModelSignals(bool block = true);
    void blockSeriesSignals(bool block = true);
    qreal valueFromModel(QModelIndex index);
    void setValueToModel(QModelIndex index, qreal value);

private:
    QXYSeries *m_series = nullptr;
    QAbstractItemModel *m_model = nullptr;
    qsizetype m_first = 0;
    qsizetype m_count = -1;
    Qt::Orientation m_orientation = Qt::Vertical;
    qsizetype m_xSection = -1;
    qsizetype m_ySection = -1;
    bool m_seriesSignalsBlock = false;
    bool m_modelSignalsBlock = false;

    Q_DISABLE_COPY_MOVE(QXYModelMapperPrivate)
};

QT_END_NAMESPACE

#endif // QXYMODELMAPPER_P_H
