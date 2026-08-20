// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTGRAPHS_QPIEMODELMAPPER_H
#define QTGRAPHS_QPIEMODELMAPPER_H

#include <QtCore/qobject.h>
#include <QtGraphs/qgraphsglobal.h>
#include <QtQmlIntegration/qqmlintegration.h>

QT_BEGIN_NAMESPACE

class QAbstractItemModel;
class QPieModelMapperPrivate;
class QPieSeries;

class Q_GRAPHS_EXPORT QPieModelMapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QPieSeries *series READ series WRITE setSeries NOTIFY seriesChanged FINAL)
    Q_PROPERTY(QAbstractItemModel *model READ model WRITE setModel NOTIFY modelChanged FINAL)
    Q_PROPERTY(qsizetype valuesSection READ valuesSection WRITE setValuesSection NOTIFY
                   valuesSectionChanged FINAL)
    Q_PROPERTY(qsizetype labelsSection READ labelsSection WRITE setLabelsSection NOTIFY
                   labelsSectionChanged FINAL)
    Q_PROPERTY(qsizetype first READ first WRITE setFirst NOTIFY firstChanged FINAL)
    Q_PROPERTY(qsizetype count READ count WRITE setCount NOTIFY countChanged FINAL)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY
                   orientationChanged FINAL)
    QML_NAMED_ELEMENT(PieModelMapper)

    Q_DECLARE_PRIVATE(QPieModelMapper)
public:
    explicit QPieModelMapper(QObject *parent = nullptr);
    ~QPieModelMapper() override;

    QPieSeries *series() const;
    void setSeries(QPieSeries *series);

    qsizetype first() const;
    void setFirst(qsizetype first);

    qsizetype count() const;
    void setCount(qsizetype count);

    qsizetype valuesSection() const;
    void setValuesSection(qsizetype valuesSection);

    qsizetype labelsSection() const;
    void setLabelsSection(qsizetype labelsSection);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation orientation);

    QAbstractItemModel *model() const;
    void setModel(QAbstractItemModel *model);
Q_SIGNALS:
    void seriesChanged();
    void modelChanged();
    void valuesSectionChanged();
    void labelsSectionChanged();
    void firstChanged();
    void countChanged();
    void orientationChanged();

public Q_SLOTS:
    void onSliceLabelChanged();
    void onSliceValueChanged();

protected:
    QPieModelMapper(QPieModelMapperPrivate &dd, QObject *parent = nullptr);

    Q_DISABLE_COPY_MOVE(QPieModelMapper)
};

QT_END_NAMESPACE

#endif // QTGRAPHS_QPIEMODELMAPPER_H
