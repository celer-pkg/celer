// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QCUSTOMSERIES_H
#define QTGRAPHS_QCUSTOMSERIES_H

#include <QtGraphs/qabstractseries.h>
#include <QtGraphs/qgraphsglobal.h>

QT_BEGIN_NAMESPACE

class QCustomSeriesPrivate;
class QCustomSeriesData;
class QQuickItem;

class Q_GRAPHS_EXPORT QCustomSeries : public QAbstractSeries
{
    Q_OBJECT
    Q_PROPERTY(QQmlComponent *delegate READ delegate WRITE setDelegate NOTIFY delegateChanged FINAL)

    QML_NAMED_ELEMENT(CustomSeries)
    QML_ADDED_IN_VERSION(6, 11)

public:
    explicit QCustomSeries(QObject *parent = nullptr);
    ~QCustomSeries() override;
    QAbstractSeries::SeriesType type() const override;

    QList<QCustomSeriesData *> dataItems() const;

    QQmlComponent *delegate() const;
    void setDelegate(QQmlComponent *newDelegate);

    Q_INVOKABLE qreal mapX(qreal x);
    Q_INVOKABLE qreal mapY(qreal y);

    Q_INVOKABLE void append(const QVariantMap &data);
    Q_INVOKABLE void append();
    Q_INVOKABLE void insert(qsizetype index, const QVariantMap &data);
    Q_INVOKABLE void insert(qsizetype index);
    Q_INVOKABLE void remove(qsizetype index);
    Q_SLOT void clear();

Q_SIGNALS:
    void delegateChanged();

protected:
    explicit QCustomSeries(QCustomSeriesPrivate &dd, QObject *parent = nullptr);
    void componentComplete() override;
    virtual void updateDelegate(QQuickItem *item, qsizetype index);
    bool event(QEvent *event) override;

private:
    friend class CustomRenderer;
    Q_DECLARE_PRIVATE(QCustomSeries)
    Q_DISABLE_COPY_MOVE(QCustomSeries)
};

QT_END_NAMESPACE

#endif // QTGRAPHS_QCUSTOMSERIES_H
