// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QLINESERIES_H
#define QTGRAPHS_QLINESERIES_H

#include <QtGraphs/qxyseries.h>
#include <QtGraphs/qgraphsglobal.h>

QT_BEGIN_NAMESPACE

class QLineSeriesPrivate;

class Q_GRAPHS_EXPORT QLineSeries : public QXYSeries
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged FINAL)
    Q_PROPERTY(Qt::PenCapStyle capStyle READ capStyle WRITE setCapStyle NOTIFY capStyleChanged FINAL)
    Q_PROPERTY(Qt::PenJoinStyle joinStyle READ joinStyle WRITE setJoinStyle NOTIFY joinStyleChanged REVISION(6, 11))
    Q_PROPERTY(QLineSeries::LineStyle lineStyle READ lineStyle WRITE setLineStyle NOTIFY lineStyleChanged REVISION(6, 11))
    Q_PROPERTY(QLineSeries::StrokeStyle strokeStyle READ strokeStyle WRITE setStrokeStyle NOTIFY strokeStyleChanged REVISION(6, 11))
    Q_PROPERTY(qreal dashOffset READ dashOffset WRITE setDashOffset NOTIFY dashOffsetChanged REVISION(6, 11))
    Q_PROPERTY(QList<qreal> dashPattern READ dashPattern WRITE setDashPattern NOTIFY dashPatternChanged REVISION(6, 11))

    QML_NAMED_ELEMENT(LineSeries)
public:
    enum class LineStyle {
        Straight,
        StepLeft,
        StepRight,
        StepCenter,
    };
    Q_ENUM(LineStyle);

    enum class StrokeStyle {
        SolidLine = Qt::SolidLine,
        DashLine = Qt::DashLine,
    };
    Q_ENUM(StrokeStyle);

    explicit QLineSeries(QObject *parent = nullptr);
    ~QLineSeries() override;
    QAbstractSeries::SeriesType type() const override;


    qreal width() const;
    void setWidth(qreal newWidth);

    Qt::PenCapStyle capStyle() const;
    void setCapStyle(Qt::PenCapStyle newCapStyle);

    Qt::PenJoinStyle joinStyle() const;
    void setJoinStyle(Qt::PenJoinStyle newJoinStyle);

    QLineSeries::LineStyle lineStyle() const;
    void setLineStyle(QLineSeries::LineStyle newLineStyle);

    QLineSeries::StrokeStyle strokeStyle() const;
    void setStrokeStyle(QLineSeries::StrokeStyle newStrokeStyle);

    qreal dashOffset() const;
    void setDashOffset(qreal newDashOffset);

    QList<qreal> dashPattern() const;
    void setDashPattern(const QList<qreal> &pattern);

    Q_REVISION(6, 10) Q_INVOKABLE QPointF dataPointCoordinatesAt(qreal x, qreal y);

Q_SIGNALS:
    void widthChanged();
    void capStyleChanged();
    Q_REVISION(6, 11) void joinStyleChanged(Qt::PenJoinStyle newJoinStyle);
    Q_REVISION(6, 11) void lineStyleChanged(QLineSeries::LineStyle newLineStyle);
    Q_REVISION(6, 11) void strokeStyleChanged(QLineSeries::StrokeStyle newStrokeStyle);
    Q_REVISION(6, 11) void dashOffsetChanged(qreal newDashOffset);
    Q_REVISION(6, 11) void dashPatternChanged(const QList<qreal> &newDashPattern);

protected:
    QLineSeries(QLineSeriesPrivate &dd, QObject *parent = nullptr);

    void componentComplete() override;

private:
    Q_DECLARE_PRIVATE(QLineSeries)
    Q_DISABLE_COPY(QLineSeries)
};

QT_END_NAMESPACE

#endif // QTGRAPHS_QLINESERIES_H
