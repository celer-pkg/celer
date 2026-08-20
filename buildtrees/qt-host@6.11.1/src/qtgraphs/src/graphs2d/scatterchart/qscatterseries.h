// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QSCATTERSERIES_H
#define QTGRAPHS_QSCATTERSERIES_H

#include <QtGraphs/qxyseries.h>
#include <QtGraphs/qgraphsglobal.h>

QT_BEGIN_NAMESPACE

class QScatterSeriesPrivate;

class Q_GRAPHS_EXPORT QScatterSeries : public QXYSeries
{
    Q_OBJECT

    QML_NAMED_ELEMENT(ScatterSeries)
public:
    explicit QScatterSeries(QObject *parent = nullptr);
    ~QScatterSeries() override;
    QAbstractSeries::SeriesType type() const override;

protected:
    QScatterSeries(QScatterSeriesPrivate &dd, QObject *parent = nullptr);

    void componentComplete() override;

private:
    Q_DECLARE_PRIVATE(QScatterSeries)
    Q_DISABLE_COPY(QScatterSeries)
};

QT_END_NAMESPACE

#endif // QTGRAPHS_QSCATTERSERIES_H
