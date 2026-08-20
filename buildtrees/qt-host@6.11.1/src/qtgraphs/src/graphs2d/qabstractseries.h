// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QABSTRACTSERIES_H
#define QTGRAPHS_QABSTRACTSERIES_H

#include <QtCore/qobject.h>
#include <QtGraphs/qabstractaxis.h>
#include <QtGraphs/qgraphsglobal.h>
#include <QtGui/qpen.h>
#include <QtQml/qqmllist.h>
#include <QtQml/qqmlparserstatus.h>

QT_BEGIN_NAMESPACE

class QAbstractSeriesPrivate;
class QGraphsView;

struct Q_GRAPHS_EXPORT QLegendData
{
    Q_GADGET
    QML_VALUE_TYPE(legendData)
    QML_ADDED_IN_VERSION(6, 10) // formerly anonymous
    Q_PROPERTY(QColor color MEMBER color FINAL)
    Q_PROPERTY(QColor borderColor MEMBER borderColor FINAL)
    Q_PROPERTY(QString label MEMBER label FINAL)

public:
    QColor color;
    QColor borderColor;
    QString label;
};

class Q_GRAPHS_EXPORT QAbstractSeries : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractSeries)
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged FINAL)
    Q_PROPERTY(bool selectable READ isSelectable WRITE setSelectable NOTIFY selectableChanged FINAL)
    Q_PROPERTY(bool hoverable READ isHoverable WRITE setHoverable NOTIFY hoverableChanged FINAL)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged FINAL)
    Q_PROPERTY(qreal valuesMultiplier READ valuesMultiplier WRITE setValuesMultiplier NOTIFY
                   valuesMultiplierChanged FINAL)
    Q_PROPERTY(SeriesType type READ type CONSTANT FINAL)
    Q_PROPERTY(QQmlListProperty<QObject> seriesChildren READ seriesChildren CONSTANT FINAL)
    Q_PROPERTY(QList<QLegendData> legendData READ legendData NOTIFY legendDataChanged FINAL)
    Q_PROPERTY(bool hovered READ isHovered NOTIFY hoveredChanged REVISION(6, 10))
    Q_PROPERTY(QAbstractAxis *axisX READ axisX WRITE setAxisX NOTIFY axisXChanged REVISION(6, 10))
    Q_PROPERTY(QAbstractAxis *axisY READ axisY WRITE setAxisY NOTIFY axisYChanged REVISION(6, 10))
    Q_PROPERTY(int zValue READ zValue WRITE setZValue NOTIFY zValueChanged REVISION(6, 10))
    Q_CLASSINFO("DefaultProperty", "seriesChildren")
    QML_ANONYMOUS

public:
    enum class SeriesType {
        Line,
        Area,
        Bar,
        Pie,
        Scatter,
        Spline,
        Custom,
    };
    Q_ENUM(SeriesType)

protected:
    explicit QAbstractSeries(QAbstractSeriesPrivate &dd, QObject *parent = nullptr);

    // from QDeclarativeParserStatus
    void classBegin() override;
    void componentComplete() override;

public:
    ~QAbstractSeries() override;
    virtual SeriesType type() const = 0;

    QString name() const;
    void setName(const QString &name);

    bool isVisible() const;
    void setVisible(bool visible = true);

    bool isSelectable() const;
    void setSelectable(bool selectable);

    qreal opacity() const;
    void setOpacity(qreal opacity);

    qreal valuesMultiplier() const;
    void setValuesMultiplier(qreal valuesMultiplier);

    QGraphsView *graph() const;
    void setGraph(QGraphsView *graph);

    const QList<QLegendData> legendData() const;

    void show();
    void hide();

    QQmlListProperty<QObject> seriesChildren();

    bool isHoverable() const;
    void setHoverable(bool newHoverable);

    bool hasLoaded() const;

    bool isHovered() const;
    void setHovered(bool enabled);

    QAbstractAxis *axisX() const;
    void setAxisX(QAbstractAxis *newAxisX);

    QAbstractAxis *axisY() const;
    void setAxisY(QAbstractAxis *newAxisY);

    int zValue() const;
    void setZValue(int newDrawOrder);

Q_SIGNALS:
    void update();
    void nameChanged();
    void visibleChanged();
    void selectableChanged();
    void hoverableChanged();
    void opacityChanged();
    void valuesMultiplierChanged();
    void legendDataChanged();
    void hoverEnter(const QString &seriesName, QPointF position, QPointF value);
    void hoverExit(const QString &seriesName, QPointF position);
    void hover(const QString &seriesName, QPointF position, QPointF value);
    Q_REVISION(6, 10) void hoveredChanged(bool hovered);

    Q_REVISION(6, 10) void axisXChanged(QAbstractAxis *newAxis);
    Q_REVISION(6, 10) void axisYChanged(QAbstractAxis *newAxis);
    Q_REVISION(6, 10) void zValueChanged(int z);

protected:
    friend class BarsRenderer;
    friend class PointRenderer;
    friend class PieRenderer;
    friend class AreaRenderer;
    friend class QGraphsView;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QLegendData)

#endif // QTGRAPHS_QABSTRACTSERIES_H
