// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QBARSERIES_H
#define QTGRAPHS_QBARSERIES_H

#include <QtGraphs/qabstractseries.h>
#include <QtGraphs/qgraphsglobal.h>

QT_BEGIN_NAMESPACE

class QBarSet;
class QBarSeriesPrivate;

class Q_GRAPHS_EXPORT QBarSeries : public QAbstractSeries
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    Q_PROPERTY(QList<QColor> seriesColors READ seriesColors WRITE setSeriesColors NOTIFY seriesColorsChanged FINAL)
    Q_PROPERTY(QList<QColor> borderColors READ borderColors WRITE setBorderColors NOTIFY borderColorsChanged FINAL)
    Q_PROPERTY(BarsType barsType READ barsType WRITE setBarsType NOTIFY barsTypeChanged FINAL)
    Q_PROPERTY(qreal barWidth READ barWidth WRITE setBarWidth NOTIFY barWidthChanged FINAL)
    Q_PROPERTY(qsizetype count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(bool labelsVisible READ labelsVisible WRITE setLabelsVisible NOTIFY
                   labelsVisibleChanged FINAL)
    Q_PROPERTY(QString labelsFormat READ labelsFormat WRITE setLabelsFormat NOTIFY
                   labelsFormatChanged FINAL)
    Q_PROPERTY(LabelsPosition labelsPosition READ labelsPosition WRITE setLabelsPosition NOTIFY
                   labelsPositionChanged FINAL)
    Q_PROPERTY(
        qreal labelsMargin READ labelsMargin WRITE setLabelsMargin NOTIFY labelsMarginChanged FINAL)
    Q_PROPERTY(
        qreal labelsAngle READ labelsAngle WRITE setLabelsAngle NOTIFY labelsAngleChanged FINAL)
    Q_PROPERTY(int labelsPrecision READ labelsPrecision WRITE setLabelsPrecision NOTIFY
                   labelsPrecisionChanged FINAL)
    Q_PROPERTY(QQmlComponent *barDelegate READ barDelegate WRITE setBarDelegate NOTIFY barDelegateChanged FINAL)
    Q_PROPERTY(QList<QBarSet *> barSets READ barSets NOTIFY barSetsChanged FINAL)
    QML_NAMED_ELEMENT(BarSeries)

public:
    enum class LabelsPosition {
        Center,
        InsideEnd,
        InsideBase,
        OutsideEnd,
    };
    Q_ENUM(LabelsPosition)

    enum class BarsType {
        Groups,
        Stacked,
        StackedPercent,
    };
    Q_ENUM(BarsType)

    explicit QBarSeries(QObject *parent = nullptr);
    ~QBarSeries() override;
    QAbstractSeries::SeriesType type() const override;

    QList<QColor> seriesColors() const;
    void setSeriesColors(const QList<QColor> &newSeriesColors);

    QList<QColor> borderColors() const;
    void setBorderColors(const QList<QColor> &newBorderColors);

    void setBarsType(QBarSeries::BarsType type);
    QBarSeries::BarsType barsType() const;

    void setBarWidth(qreal width);
    qreal barWidth() const;

    Q_INVOKABLE bool append(QBarSet *set);
    Q_INVOKABLE bool take(QBarSet *set);
    Q_INVOKABLE qsizetype count() const;
    Q_INVOKABLE bool append(const QList<QBarSet *> &sets);
    Q_INVOKABLE bool remove(QBarSet *set);
    Q_INVOKABLE bool insert(qsizetype index, QBarSet *set);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void replace(qsizetype index, QBarSet *set);
    Q_INVOKABLE QBarSet *at(qsizetype index);
    Q_INVOKABLE qsizetype find(QBarSet *set) const;
    Q_INVOKABLE void removeMultiple(qsizetype index, qsizetype count);
    Q_INVOKABLE bool remove(qsizetype index);
    Q_INVOKABLE bool replace(QBarSet *oldValue, QBarSet *newValue);
    Q_INVOKABLE bool replace(const QList<QBarSet *> &sets);

    QList<QBarSet *> barSets() const;

    void setLabelsVisible(bool visible = true);
    bool labelsVisible() const;

    void setLabelsFormat(const QString &format);
    QString labelsFormat() const;

    void setLabelsMargin(qreal margin);
    qreal labelsMargin() const;

    void setLabelsAngle(qreal angle);
    qreal labelsAngle() const;

    void setLabelsPosition(QBarSeries::LabelsPosition position);
    QBarSeries::LabelsPosition labelsPosition() const;

    void setLabelsPrecision(int precision);
    int labelsPrecision() const;

    QQmlComponent *barDelegate() const;
    void setBarDelegate(QQmlComponent *newBarDelegate);

public Q_SLOTS:
    void selectAll();
    void deselectAll();

protected:
    QBarSeries(QBarSeriesPrivate &dd, QObject *parent = nullptr);
    void componentComplete() override;

Q_SIGNALS:
    void updatedBars();
    void seriesColorsChanged();
    void borderColorsChanged();
    void countChanged();
    void barWidthChanged();
    void labelsVisibleChanged(bool visible);
    void labelsFormatChanged(const QString &format);
    void labelsPositionChanged(QBarSeries::LabelsPosition position);
    void barsTypeChanged(QBarSeries::BarsType type);
    void labelsMarginChanged(qreal margin);
    void labelsAngleChanged(qreal angle);
    void labelsPrecisionChanged(int precision);
    void barDelegateChanged();

    void barsetsAdded(const QList<QBarSet *> &sets);
    void barsetsReplaced(const QList<QBarSet *> &sets);
    void barsetsRemoved(const QList<QBarSet *> &sets);
    void setValueChanged(qsizetype index, QBarSet *barset);
    void setValueAdded(qsizetype index, qsizetype count, QBarSet *barset);
    void setValueRemoved(qsizetype index, qsizetype count, QBarSet *barset);
    void barSetsChanged();

    Q_REVISION(6, 9) void clicked(qsizetype index, QBarSet *barset);
    Q_REVISION(6, 9) void doubleClicked(qsizetype index, QBarSet *barset);
    Q_REVISION(6, 9) void pressed(qsizetype index, QBarSet *barset);
    Q_REVISION(6, 9) void released(qsizetype index, QBarSet *barset);

private Q_SLOTS:
    void handleSetValueChange(qsizetype index);
    void handleSetValueAdd(qsizetype index, qsizetype count);
    void handleSetValueRemove(qsizetype index, qsizetype count);

private:
    Q_DECLARE_PRIVATE(QBarSeries)
    Q_DISABLE_COPY(QBarSeries)
    friend class BarSet;
    friend class BarsRenderer;
    bool barDelegateDirty() const;
    void setBarDelegateDirty(bool dirty);
};

QT_END_NAMESPACE

#endif // QTGRAPHS_QBARSERIES_H
