// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquickgraphsscatternode_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \qmltype Scatter3DNode
 * \inherits GraphsNode
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \brief 3D scatter graph node.
 *
 * This type enables developers to render a scatter graph node in 3D with Qt Quick.
 *
 * You will need to import Qt Graphs module to use this type:
 *
 * \snippet doc_src_qmlnodes.qml 0
 *
 * After that you can use Scatter3DNode in your qml files:
 *
 * \snippet doc_src_qmlnodes.qml 2
 *
 * \sa Scatter3DSeries, Spline3DSeries, ScatterDataProxy, Bars3DNode, Surface3DNode,
 * {Qt Graphs C++ Classes for 3D}
 */

/*!
 * \qmlproperty Value3DAxis Scatter3DNode::axisX
 * The active x-axis.
 *
 * If an axis is not given, a temporary default axis with no labels and an
 * automatically adjusting range is created.
 * This temporary axis is destroyed if another axis is explicitly set to the
 * same orientation.
 */

/*!
 * \qmlproperty Value3DAxis Scatter3DNode::axisY
 * The active y-axis.
 *
 * If an axis is not given, a temporary default axis with no labels and an
 * automatically adjusting range is created.
 * This temporary axis is destroyed if another axis is explicitly set to the
 * same orientation.
 */

/*!
 * \qmlproperty Value3DAxis Scatter3DNode::axisZ
 * The active z-axis.
 *
 * If an axis is not given, a temporary default axis with no labels and an
 * automatically adjusting range is created.
 * This temporary axis is destroyed if another axis is explicitly set to the
 * same orientation.
 */

/*!
 * \qmlproperty Scatter3DSeries Scatter3DNode::selectedSeries
 * \readonly
 *
 * The selected series or null.
 */

/*!
 * \qmlproperty list<Scatter3DSeries> Scatter3DNode::seriesList
 * \qmldefault
 * This property holds the series of the graph.
 * By default, this property contains an empty list.
 * To set the series, either use the addSeries() method or define them as
 * children of the graph.
 */

/*!
 * \qmlmethod void Scatter3DNode::addSeries(Scatter3DSeries series)
 * Adds the \a series to the graph. A graph can contain multiple series, but has
 * only one set of axes. If the newly added series has specified a selected
 * item, it will be highlighted and any existing selection will be cleared. Only
 * one added series can have an active selection.
 * \sa GraphsNode::hasSeries()
 */

/*!
 * \qmlmethod void Scatter3DNode::removeSeries(Scatter3DSeries series)
 * Remove the \a series from the graph.
 * \sa GraphsNode::hasSeries()
 */

/*!
 * \qmlsignal Scatter3DNode::axisXChanged(ValueAxis3D axis)
 *
 * This signal is emitted when axisX changes to \a axis.
 */

/*!
 * \qmlsignal Scatter3DNode::axisYChanged(ValueAxis3D axis)
 *
 * This signal is emitted when axisY changes to \a axis.
 */

/*!
 * \qmlsignal Scatter3DNode::axisZChanged(ValueAxis3D axis)
 *
 * This signal is emitted when axisZ changes to \a axis.
 */

/*!
 * \qmlsignal Scatter3DNode::selectedSeriesChanged(Scatter3DSeries series)
 *
 * This signal is emitted when selectedSeries changes to \a series.
 */

QQuickGraphsScatterNode::QQuickGraphsScatterNode(QQuick3DNode *parent)
    : QQuickGraphsNode(parent)
{}

QQuickGraphsScatterNode::~QQuickGraphsScatterNode() {}

void QQuickGraphsScatterNode::componentComplete()
{
    const QString qmlData = QLatin1StringView(R"QML(
        import QtQuick;
        import QtGraphs;

        Scatter3D{}
    )QML");

    QQmlComponent *component = new QQmlComponent(qmlEngine(this), this);
    component->setData(qmlData.toUtf8(), QUrl());
    m_graph.reset(qobject_cast<QQuickGraphsItem *>(component->create()));
    setGraphParent();
    QQuickGraphsNode::componentComplete();

    //initialize components
    if (m_axisX)
        graphScatter()->setAxisX(static_cast<QValue3DAxis *>(m_axisX));
    if (m_axisY)
        graphScatter()->setAxisY(static_cast<QValue3DAxis *>(m_axisY));
    if (m_axisZ)
        graphScatter()->setAxisZ(static_cast<QValue3DAxis *>(m_axisZ));

    graphScatter()->setSelectionMode(m_selectionMode);

    for (auto series : std::as_const(m_seriesList))
        graphScatter()->addSeries(static_cast<QScatter3DSeries *>(series));

    //connect signals
    connect(graphScatter(),
            &QQuickGraphsScatter::axisXChanged,
            this,
            &QQuickGraphsScatterNode::axisXChanged);
    connect(graphScatter(),
            &QQuickGraphsScatter::axisYChanged,
            this,
            &QQuickGraphsScatterNode::axisYChanged);
    connect(graphScatter(),
            &QQuickGraphsScatter::axisZChanged,
            this,
            &QQuickGraphsScatterNode::axisZChanged);
}

QValue3DAxis *QQuickGraphsScatterNode::axisX() const
{
    if (graphScatter())
        return graphScatter()->axisX();
    else
        return static_cast<QValue3DAxis *>(m_axisX);
}

void QQuickGraphsScatterNode::setAxisX(QValue3DAxis *axis)
{
    if (m_axisX == axis)
        return;
    m_axisX = axis;
    if (graphScatter())
        graphScatter()->setAxisX(axis);
}

QValue3DAxis *QQuickGraphsScatterNode::axisY() const
{
    if (graphScatter())
        return graphScatter()->axisY();
    else
        return static_cast<QValue3DAxis *>(m_axisY);
}

void QQuickGraphsScatterNode::setAxisY(QValue3DAxis *axis)
{
    if (m_axisY == axis)
        return;
    m_axisY = axis;
    if (graphScatter())
        graphScatter()->setAxisY(axis);
}

QValue3DAxis *QQuickGraphsScatterNode::axisZ() const
{
    if (graphScatter())
        return graphScatter()->axisZ();
    else
        return static_cast<QValue3DAxis *>(m_axisZ);
}

void QQuickGraphsScatterNode::setAxisZ(QValue3DAxis *axis)
{
    if (m_axisZ == axis)
        return;
    m_axisZ = axis;
    if (graphScatter())
        graphScatter()->setAxisZ(axis);
}

QScatter3DSeries *QQuickGraphsScatterNode::selectedSeries() const
{
    if (graphScatter())
        return graphScatter()->selectedSeries();
    else
        return nullptr;
}

void QQuickGraphsScatterNode::setSelectionMode(QtGraphs3D::SelectionFlags mode)
{
    if (graphScatter())
        graphScatter()->setSelectionMode(mode);
    else
        setSelectionMode(mode);
}

QList<QScatter3DSeries *> QQuickGraphsScatterNode::scatterSeriesList()
{
    if (graphScatter()) {
        return graphScatter()->scatterSeriesList();
    } else {
        QList<QScatter3DSeries *> scatterSeriesList;
        for (QAbstract3DSeries *abstractSeries : std::as_const(m_seriesList)) {
            QScatter3DSeries *scatterSeries = qobject_cast<QScatter3DSeries *>(abstractSeries);
            if (scatterSeries)
                scatterSeriesList.append(scatterSeries);
        }
        return scatterSeriesList;
    }
}

QQmlListProperty<QScatter3DSeries> QQuickGraphsScatterNode::seriesList()
{
    if (graphScatter()) {
        return graphScatter()->seriesList();
    } else {
        return QQmlListProperty<QScatter3DSeries>(this,
                                                  this,
                                                  &QQuickGraphsScatterNode::appendSeriesFunc,
                                                  &QQuickGraphsScatterNode::countSeriesFunc,
                                                  &QQuickGraphsScatterNode::atSeriesFunc,
                                                  &QQuickGraphsScatterNode::clearSeriesFunc);
    }
}

void QQuickGraphsScatterNode::appendSeriesFunc(QQmlListProperty<QScatter3DSeries> *list,
                                               QScatter3DSeries *series)
{
    reinterpret_cast<QQuickGraphsScatterNode *>(list->data)->addSeries(series);
}

qsizetype QQuickGraphsScatterNode::countSeriesFunc(QQmlListProperty<QScatter3DSeries> *list)
{
    return reinterpret_cast<QQuickGraphsScatterNode *>(list->data)->scatterSeriesList().size();
}

QScatter3DSeries *QQuickGraphsScatterNode::atSeriesFunc(QQmlListProperty<QScatter3DSeries> *list,
                                                        qsizetype index)
{
    return reinterpret_cast<QQuickGraphsScatterNode *>(list->data)->scatterSeriesList().at(index);
}

void QQuickGraphsScatterNode::clearSeriesFunc(QQmlListProperty<QScatter3DSeries> *list)
{
    QQuickGraphsScatterNode *declScatterNode = reinterpret_cast<QQuickGraphsScatterNode *>(
        list->data);
    QList<QScatter3DSeries *> realList = declScatterNode->scatterSeriesList();
    qsizetype count = realList.size();
    for (qsizetype i = 0; i < count; i++)
        declScatterNode->removeSeries(realList.at(i));
}

void QQuickGraphsScatterNode::addSeries(QScatter3DSeries *series)
{
    Q_ASSERT(series && series->type() == QAbstract3DSeries::SeriesType::Scatter);

    if (graphScatter())
        graphScatter()->addSeries(series);

    QQuickGraphsNode::addSeriesInternal(series);
}

void QQuickGraphsScatterNode::removeSeries(QScatter3DSeries *series)
{
    if (graphScatter())
        graphScatter()->removeSeries(series);
    QQuickGraphsNode::removeSeriesInternal(series);
}

void QQuickGraphsScatterNode::clearSelection()
{
    if (graphScatter())
        graphScatter()->clearSelection();
}

bool QQuickGraphsScatterNode::doPicking(QPointF point)
{
    if (graphScatter())
        return graphScatter()->doPicking(point);
    else
        return false;
}

bool QQuickGraphsScatterNode::doRayPicking(const QVector3D &origin, const QVector3D &direction)
{
    if (graphScatter())
        return graphScatter()->doRayPicking(origin, direction);
    else
        return false;
}
/*!
 * \internal
 */
QQuickGraphsScatter *QQuickGraphsScatterNode::graphScatter()
{
    return static_cast<QQuickGraphsScatter *>(m_graph.get());
}

/*!
 * \internal
 */
const QQuickGraphsScatter *QQuickGraphsScatterNode::graphScatter() const
{
    return static_cast<QQuickGraphsScatter *>(m_graph.get());
}

QT_END_NAMESPACE

#include "moc_qquickgraphsscatternode_p.cpp"
