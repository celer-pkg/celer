// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtGraphs/qcustomseries.h>
#include <private/charthelpers_p.h>
#include <private/qcustomseries_p.h>
#include <private/qcustomseriesdata_p.h>
#include <private/qgraphsview_p.h>
#include <QtGraphs/private/customrenderer_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QCustomSeries
    \inmodule QtGraphs
    \brief The QCustomSeries class allows presenting customized graph types.
    \since 6.11

    Use QCustomSeries to create customized graphs. Think of it as a scatter graph
    that lets you access custom data for each element.

    QCustomSeries defines a delegate that is used to render every item added to
    the graph. Each item includes user-defined data stored in a \l QVariantMap.
    Index of the item in the graph is also automatically added to the data map.
    The series passes this map to each element created based on the delegate.
    The delegate determines how to use the data.

    To map data to the render coordinates defined by the \c QGraphsView axes,
    use the \l mapX and \l mapY functions.

    \note Currently, individual elements in a custom series do not share
    information with one another. For this reason, you can't implement a custom
    line series.
*/

/*!
    \qmltype CustomSeries
    \nativetype QCustomSeries
    \inqmlmodule QtGraphs
    \brief The CustomSeries type allows presenting customized graph types.
    \since 6.11

    Use CustomSeries to create customized graphs. Think of it as a scatter graph
    that lets you access custom data for each element.

    CustomSeries defines a delegate that is used to render every item added to
    the graph. Each item includes user-defined data stored in a \l QVariantMap.
    Index of the item in the graph is also automatically added to the data map.
    In the QML context the variant map is defined by a JavaScript object.
    The series passes this map to each element created based on the delegate.
    The delegate determines how to use the data.

    To map data to the render coordinates defined by the \l GraphsView axes,
    use the \l mapX and \l mapY functions.

    \note Currently, individual elements in a custom series do not share
    information with one another. For this reason, you can't implement a custom
    line series.

    \image graphs2d-custom.png

    The following code implements the depicted custom graph. In this case the
    custom data contains two values: upper and lower. Those are then defined
    in the delegate to affect the height and the y coordinate of the element.

    \qml
    import QtQuick
    import QtGraphs

    Window {
        width: 640
        height: 480
        visible: true

        GraphsView {
            anchors.fill: parent

            axisX: BarCategoryAxis {
                categories: ["ABC", "DEF"]
            }

            axisY: ValueAxis {
                max: 8
                tickInterval: 1
            }

            CustomSeries {
                id: custom

                delegate: Rectangle {
                    property var data
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "lightsteelblue" }
                        GradientStop { position: 1.0; color: "blue" }
                    }
                    border.width: 1
                    x: custom.mapX(data.index + 0.5) - width * 0.5
                    y: custom.mapY((data.upper + data.lower) * 0.5) - height * 0.5
                    width: Math.abs(custom.mapX(1) - custom.mapX(0)) * 0.5
                    height: Math.abs(custom.mapY(data.upper) - custom.mapY(data.lower))
                }

                CustomSeriesData {
                    data: ({
                        upper: 5,
                        lower: 1
                    })
                }

                CustomSeriesData {
                    data: ({
                        upper: 6,
                        lower: 4
                    })
                }
            }
        }
    }
    \endqml
*/

QCustomSeries::QCustomSeries(QObject *parent)
    : QAbstractSeries(*(new QCustomSeriesPrivate()), parent)
{}

QCustomSeries::~QCustomSeries() {}

QCustomSeries::QCustomSeries(QCustomSeriesPrivate &dd, QObject *parent)
    : QAbstractSeries(dd, parent)
{}

QAbstractSeries::SeriesType QCustomSeries::type() const
{
    return QAbstractSeries::SeriesType::Custom;
}

QList<QCustomSeriesData *> QCustomSeries::dataItems() const
{
    Q_D(const QCustomSeries);
    return d->m_dataItems;
}

/*!
    \property QCustomSeries::delegate

    The delegate is used to insert elements to QML corresponding to provided
    custom data. Data is defined in the CustomSeries itself and given to
    the delegated elements through the "data" property.
*/
/*!
    \qmlproperty Component CustomSeries::delegate
    The delegate is used to insert elements to QML corresponding to provided
    custom data. Data is defined in the the CustomSeries itself and given to
    the delegated elements through the "data" property.

    \code
        delegate: Rectangle {
                property var data
                Rectangle {
                    color: "tan"
                    border.color: "black"
                    border.width: 4
                    y: series.mapY(data.index + 1) - height / 2 + 1
                    x: series.mapX(data.median) - width / 2
                    height: 32
                    width: Math.abs(series.mapX(data.upperQuartile) - series.mapX(data.lowerQuartile))
                }
            }
        }
    \endcode
*/
QQmlComponent *QCustomSeries::delegate() const
{
    Q_D(const QCustomSeries);
    return d->m_delegate;
}

void QCustomSeries::setDelegate(QQmlComponent *newDelegate)
{
    Q_D(QCustomSeries);
    if (d->m_delegate == newDelegate) {
        qCDebug(lcProperties2D) << "QCustomSeries::setDelegate. Point delegate is already set to:"
                                << newDelegate;
        return;
    }

    d->m_delegate = newDelegate;
    emit delegateChanged();
    emit update();
}

/*!
    \qmlmethod real CustomSeries::mapX(real x)
    Returns \a x axis-space coordinate converted into render-space.
*/
/*!
    Returns \a x axis-space coordinate converted into render-space.
*/
qreal QCustomSeries::mapX(qreal x)
{
    Q_D(const QCustomSeries);
    if (d->m_graph) {
        auto renderer = d->m_graph->customRenderer();
        if (renderer) {
            auto metaObj = renderer->metaObject();
            auto propIndex = metaObj->indexOfProperty("plotArea");
            if (propIndex >= 0) {
                auto prop = metaObj->property(propIndex);
                qmlEngine(this)->captureProperty(renderer, prop);
            }
        }
        return d->m_graph->mapX(this, x);
    }
    return x;
}

/*!
    \qmlmethod real CustomSeries::mapY(real y)
    Returns \a y axis-space coordinate converted into render-space.
*/
/*!
    Returns \a y axis-space coordinate converted into render-space.
*/
qreal QCustomSeries::mapY(qreal y)
{
    Q_D(const QCustomSeries);
    if (d->m_graph) {
        auto renderer = d->m_graph->customRenderer();
        if (renderer) {
            auto metaObj = renderer->metaObject();
            auto propIndex = metaObj->indexOfProperty("plotArea");
            if (propIndex >= 0) {
                auto prop = metaObj->property(propIndex);
                qmlEngine(this)->captureProperty(renderer, prop);
            }
        }
        return d->m_graph->mapY(this, y);
    }
    return y;
}

/*!
    \qmlmethod void CustomSeries::append()
    Appends an empty data item to the series.
*/
/*!
    Appends an empty data item to the series.
*/
void QCustomSeries::append()
{
    append(QVariantMap());
}

/*!
    \qmlmethod void CustomSeries::append(var data)
    Appends a data item defined by \a data to the series.
*/
/*!
    Appends a data item defined by \a data to the series.
*/
void QCustomSeries::append(const QVariantMap &data)
{
    Q_D(QCustomSeries);
    QCustomSeriesData *dataItem = new QCustomSeriesData(this);
    dataItem->setData(data);
    d->m_dataItems.append(dataItem);
}

/*!
    \qmlmethod void CustomSeries::insert(int index)
    Appends an empty data item to the series at position \a index.
*/
/*!
    Appends an empty data item to the series at position \a index.
*/
void QCustomSeries::insert(qsizetype index)
{
    insert(index, QVariantMap());
}

/*!
    \qmlmethod void CustomSeries::insert(int index, var data)
    Appends a data item defined by \a data to the series at position \a index.
*/
/*!
    Appends a data item defined by \a data to the series at position \a index.
*/
void QCustomSeries::insert(qsizetype index, const QVariantMap &data)
{
    Q_D(QCustomSeries);
    if (d->m_dataItems.size() <= index)
        return;

    QCustomSeriesData *dataItem = new QCustomSeriesData(this);
    dataItem->setData(data);
    d->m_dataItems.insert(index, dataItem);
}

/*!
    \qmlmethod void CustomSeries::remove(int index)
    Remove a data item at position \a index.
*/
/*!
    Remove a data item at position \a index.
*/
void QCustomSeries::remove(qsizetype index)
{
    Q_D(QCustomSeries);
    if (d->m_dataItems.size() <= index)
        return;

    delete d->m_dataItems[index];

    d->m_dataItems.remove(index);
}

/*!
    \qmlmethod void CustomSeries::clear()
    Clear all data items.
*/
/*!
    Clear all data items.
*/
void QCustomSeries::clear()
{
    Q_D(QCustomSeries);

    qDeleteAll(d->m_dataItems);

    d->m_dataItems.clear();
}

void QCustomSeries::componentComplete()
{
    Q_D(QCustomSeries);

    for (auto *child : children()) {
        if (auto dataItem = qobject_cast<QCustomSeriesData *>(child)) {
            if (!d->m_dataItems.contains(dataItem)) {
                d->m_dataItems.append(dataItem);
                qCDebug(lcSeries2D, "append dataItem to customSeries");
            }
        }
    }

    qCDebug(lcEvents2D, "QCustomSeries::componentComplete");

    QAbstractSeries::componentComplete();
}

/*!
    Called by the graph renderer to give access to the \a item
    representing the data, and the \a index corresponding to the
    data item.

    Override to implement custom property values for QQuickItem
    instance(s).
*/
void QCustomSeries::updateDelegate(QQuickItem *item, qsizetype index)
{
    auto &&data = dataItems()[index]->data();
    data[QStringLiteral("index")] = index;
    item->setProperty("data", data);
}

bool QCustomSeries::event(QEvent *event)
{
    return QObject::event(event);
}

QCustomSeriesPrivate::QCustomSeriesPrivate()
    : QAbstractSeriesPrivate(QAbstractSeries::SeriesType::Custom)
{}

QT_END_NAMESPACE

#include "moc_qcustomseries.cpp"
