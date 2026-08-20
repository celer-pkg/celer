// Copyright (C) 2024 The Qt Company Ltd.

// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTGRAPHS_QXYMODELMAPPER_H
#define QTGRAPHS_QXYMODELMAPPER_H
#include <QtCore/qobject.h>
#include <QtGraphs/qgraphsglobal.h>
#include <QtQmlIntegration/qqmlintegration.h>

QT_BEGIN_NAMESPACE

class QXYModelMapperPrivate;
class QXYSeries;
class QAbstractItemModel;

class Q_GRAPHS_EXPORT QXYModelMapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QXYSeries *series READ series WRITE setSeries NOTIFY seriesChanged FINAL)
    Q_PROPERTY(QAbstractItemModel *model READ model WRITE setModel NOTIFY modelChanged FINAL)
    Q_PROPERTY(qsizetype xSection READ xSection WRITE setXSection NOTIFY xSectionChanged FINAL)
    Q_PROPERTY(qsizetype ySection READ ySection WRITE setYSection NOTIFY ySectionChanged FINAL)
    Q_PROPERTY(qsizetype first READ first WRITE setFirst NOTIFY firstChanged FINAL)
    Q_PROPERTY(qsizetype count READ count WRITE setCount NOTIFY countChanged FINAL)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY
                   orientationChanged FINAL)
    QML_NAMED_ELEMENT(XYModelMapper)
    Q_DECLARE_PRIVATE(QXYModelMapper)
public:
    explicit QXYModelMapper(QObject *parent = nullptr);
    ~QXYModelMapper() override;

    QAbstractItemModel *model() const;
    void setModel(QAbstractItemModel *model);

    QXYSeries *series() const;
    void setSeries(QXYSeries *series);

    qsizetype first() const;
    void setFirst(qsizetype first);

    qsizetype count() const;
    void setCount(qsizetype count);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation orientation);

    qsizetype xSection() const;
    void setXSection(qsizetype xSection);

    qsizetype ySection() const;
    void setYSection(qsizetype ySection);
Q_SIGNALS:
    void seriesChanged();
    void modelChanged();
    void xSectionChanged();
    void ySectionChanged();
    void firstChanged();
    void countChanged();
    void orientationChanged();

protected:
    QXYModelMapper(QXYModelMapperPrivate &dd, QObject *parent = nullptr);

    Q_DISABLE_COPY_MOVE(QXYModelMapper)
};

QT_END_NAMESPACE

#endif // QTGRAPHS_QXYMODELMAPPER_H
