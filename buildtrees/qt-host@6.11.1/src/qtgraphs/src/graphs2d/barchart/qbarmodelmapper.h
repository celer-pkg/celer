// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTGRAPHS_QBARMODELMAPPER_H
#define QTGRAPHS_QBARMODELMAPPER_H

#include <QtCore/qobject.h>
#include <QtGraphs/qgraphsglobal.h>
#include <QtQmlIntegration/qqmlintegration.h>

QT_BEGIN_NAMESPACE

class QAbstractItemModel;
class QBarSeries;
class QBarModelMapperPrivate;

class Q_GRAPHS_EXPORT QBarModelMapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QBarSeries *series READ series WRITE setSeries NOTIFY seriesChanged FINAL)
    Q_PROPERTY(QAbstractItemModel *model READ model WRITE setModel NOTIFY modelChanged FINAL)
    Q_PROPERTY(qsizetype firstBarSetSection READ firstBarSetSection WRITE setFirstBarSetSection
                   NOTIFY firstBarSetSectionChanged FINAL)
    Q_PROPERTY(qsizetype lastBarSetSection READ lastBarSetSection WRITE setLastBarSetSection NOTIFY
                   lastBarSetSectionChanged FINAL)
    Q_PROPERTY(qsizetype first READ first WRITE setFirst NOTIFY firstChanged FINAL)
    Q_PROPERTY(qsizetype count READ count WRITE setCount NOTIFY countChanged FINAL)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY
                   orientationChanged FINAL)

    QML_NAMED_ELEMENT(BarModelMapper)
    Q_DECLARE_PRIVATE(QBarModelMapper)
public:
    explicit QBarModelMapper(QObject *parent = nullptr);
    ~QBarModelMapper() override;

    QAbstractItemModel *model() const;
    void setModel(QAbstractItemModel *model);

    QBarSeries *series() const;
    void setSeries(QBarSeries *series);

    qsizetype firstBarSetSection() const;
    void setFirstBarSetSection(qsizetype newFirstBarSetSection);

    qsizetype lastBarSetSection() const;
    void setLastBarSetSection(qsizetype newLastBarSetSection);

    qsizetype count() const;
    void setCount(qsizetype newCount);

    qsizetype first() const;
    void setFirst(qsizetype newFirst);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation orientation);

Q_SIGNALS:
    void seriesChanged();
    void modelChanged();
    void firstBarSetSectionChanged();
    void lastBarSetSectionChanged();
    void firstChanged();
    void countChanged();
    void orientationChanged();

protected:
    QBarModelMapper(QBarModelMapperPrivate &dd, QObject *parent = nullptr);

private Q_SLOTS:
    void onValuesAdded(qsizetype index, qsizetype count);
    void onBarLabelChanged();
    void onBarValueChanged(qsizetype index);

private:
    Q_DISABLE_COPY_MOVE(QBarModelMapper)
};

QT_END_NAMESPACE

#endif // QTGRAPHS_QBARMODELMAPPER_H
