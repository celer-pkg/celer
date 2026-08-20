// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtGraphsWidgets/q3dscatterwidgetitem.h>
#include <private/q3dscatterwidgetitem_p.h>
#include <private/qquickgraphsscatter_p.h>

QT_BEGIN_NAMESPACE

/*!
 * \class Q3DScatterWidgetItem
 * \inmodule QtGraphsWidgets
 * \ingroup graphs_3D_widgets
 * \brief The Q3DScatterWidgetItem class provides methods for rendering 3D scatter graphs.
 *
 * This class enables developers to render 3D scatter graphs and view them by freely
 * rotating the scene. Rotation is achieved by holding down the right mouse button
 * and moving the mouse, while zooming is accomplished using the mouse wheel. If
 * enabled, selection is performed with the left mouse button. The scene can be
 * reset to the default camera view by clicking the mouse wheel. On touch devices,
 * rotation is achieved by tap-and-move, selection by tap-and-hold, and zooming
 * by pinch.
 *
 * If no axes are set explicitly to Q3DScatterWidgetItem, temporary default axes with no
 * labels are created. These default axes can be modified via axis accessors,
 * but as soon any axis is set explicitly for the orientation, the default axis
 * for that orientation is destroyed.
 *
 * Q3DScatterWidgetItem supports more than one series visible at the same time.
 *
 * Q3DScatterWidgetItem has transparency support. This feature allows you to adjust
 * the opacity of the scatter points, making them partially see-through,
 * fully transparent, or opaque.
 *
 * \section1 How to construct a minimal Q3DScatterWidgetItem graph
 *
 * First, construct Q3DScatterWidgetItem. Since we are running the graph as the top-level
 * window in this example, we need to clear the \c Qt::FramelessWindowHint flag,
 * which is set by default:
 *
 * \snippet doc_src_q3dscatter_construction.cpp 0
 *
 * Now Q3DScatterWidgetItem is ready to receive data to be rendered. Add one series of 3
 * QVector3D items:
 *
 * \note In the new proxy-series relationship, data is held in series.
 * Therefore, for the proxy to be able to add, delete, or edit the data, it is
 * a prerequisite to create a series first.
 *
 * \snippet doc_src_q3dscatter_construction.cpp 1
 *
 * Finally you will need to set it visible:
 *
 * \snippet doc_src_q3dscatter_construction.cpp 2
 *
 * The complete code needed to create and display this graph is:
 *
 * \snippet doc_src_q3dscatter_construction.cpp 3
 *
 * And this is what those few lines of code produce:
 *
 * \image q3dscatter-minimal.png
 *
 * The scene can be rotated, zoomed into, and an item can be selected to view
 * its position, but no other interactions are included in this minimal code
 * example. You can learn more by familiarizing yourself with the examples
 * provided, like the \l{Simple Scatter Graph}.
 *
 * \sa Q3DBarsWidgetItem, Q3DSurfaceWidgetItem, {Qt Graphs C++ Classes for 3D}
 */

/*!
 * Constructs a new 3D scatter graph with the optional \a parent.
 */
Q3DScatterWidgetItem::Q3DScatterWidgetItem(QObject *parent)
    : Q3DGraphsWidgetItem(*(new Q3DScatterWidgetItemPrivate()), parent, QStringLiteral("Scatter3D"))
{}

/*!
 * Destroys the 3D scatter graph.
 */
Q3DScatterWidgetItem::~Q3DScatterWidgetItem() {}

/*!
 * Adds the \a series to the graph. A graph can contain multiple series, but has
 * only one set of axes. If the newly added series has specified a selected
 * item, it will be highlighted and any existing selection will be cleared. Only
 * one added series can have an active selection.
 *
 * \sa Q3DGraphsWidgetItem::hasSeries()
 */
void Q3DScatterWidgetItem::addSeries(QScatter3DSeries *series)
{
    graphScatter()->addSeries(series);
}

/*!
 * Removes the \a series from the graph.
 *
 * \sa Q3DGraphsWidgetItem::hasSeries()
 */
void Q3DScatterWidgetItem::removeSeries(QScatter3DSeries *series)
{
    graphScatter()->removeSeries(series);
}

/*!
 * Returns the list of series added to this graph.
 *
 * \sa Q3DGraphsWidgetItem::hasSeries()
 */
QList<QScatter3DSeries *> Q3DScatterWidgetItem::seriesList() const
{
    QList<QScatter3DSeries *> scatterSeriesList;
    for (QAbstract3DSeries *abstractSeries : graphScatter()->m_seriesList) {
        QScatter3DSeries *scatterSeries = qobject_cast<QScatter3DSeries *>(abstractSeries);
        if (scatterSeries)
            scatterSeriesList.append(scatterSeries);
    }

    return scatterSeriesList;
}

/*!
 * \property Q3DScatterWidgetItem::axisX
 *
 * \brief The active x-axis.
 *
 * Sets \a axis as the active x-axis. Implicitly calls addAxis() to transfer the
 * ownership of the axis to this graph.
 *
 * If \a axis is null, a temporary default axis with no labels and an
 * automatically adjusting range is created. This temporary axis is destroyed if
 * another axis is set explicitly to the same orientation.
 *
 * \sa addAxis(), releaseAxis()
 */
void Q3DScatterWidgetItem::setAxisX(QValue3DAxis *axis)
{
    graphScatter()->setAxisX(axis);
}

QValue3DAxis *Q3DScatterWidgetItem::axisX() const
{
    return static_cast<QValue3DAxis *>(graphScatter()->axisX());
}

/*!
 * \property Q3DScatterWidgetItem::axisY
 *
 * \brief The active y-axis.
 *
 * Sets \a axis as the active y-axis. Implicitly calls addAxis() to transfer the
 * ownership of the axis to this graph.
 *
 * If \a axis is null, a temporary default axis with no labels and an
 * automatically adjusting range is created. This temporary axis is destroyed if
 * another axis is set explicitly to the same orientation.
 *
 * \sa addAxis(), releaseAxis()
 */
void Q3DScatterWidgetItem::setAxisY(QValue3DAxis *axis)
{
    graphScatter()->setAxisY(axis);
}

QValue3DAxis *Q3DScatterWidgetItem::axisY() const
{
    return static_cast<QValue3DAxis *>(graphScatter()->axisY());
}

/*!
 * \property Q3DScatterWidgetItem::axisZ
 *
 * \brief The active z-axis.
 *
 * Sets \a axis as the active z-axis. Implicitly calls addAxis() to transfer the
 * ownership of the axis to this graph.
 *
 * If \a axis is null, a temporary default axis with no labels and an
 * automatically adjusting range is created. This temporary axis is destroyed if
 * another axis is set explicitly to the same orientation.
 *
 * \sa addAxis(), releaseAxis()
 */
void Q3DScatterWidgetItem::setAxisZ(QValue3DAxis *axis)
{
    graphScatter()->setAxisZ(axis);
}

QValue3DAxis *Q3DScatterWidgetItem::axisZ() const
{
    return static_cast<QValue3DAxis *>(graphScatter()->axisZ());
}

/*!
 * \property Q3DScatterWidgetItem::selectedSeries
 * \readonly
 *
 * \brief The selected series or null.
 */
QScatter3DSeries *Q3DScatterWidgetItem::selectedSeries() const
{
    return graphScatter()->selectedSeries();
}

bool Q3DScatterWidgetItem::event(QEvent *event)
{
    return Q3DGraphsWidgetItem::event(event);
}

/*!
 * Adds \a axis to the graph. The axes added via addAxis are not yet taken to
 * use, addAxis is simply used to give the ownership of the \a axis to the
 * graph. The \a axis must not be null or added to another graph.
 *
 * \sa releaseAxis(), setAxisX(), setAxisY(), setAxisZ()
 */
void Q3DScatterWidgetItem::addAxis(QValue3DAxis *axis)
{
    graphScatter()->addAxis(axis);
}

/*!
 * Releases the ownership of the \a axis back to the caller, if it is added to
 * this graph. If the released \a axis is in use, a new default axis will be
 * created and set active.
 *
 * If the default axis is released and added back later, it behaves as any other
 * axis would.
 *
 * \sa addAxis(), setAxisX(), setAxisY(), setAxisZ()
 */
void Q3DScatterWidgetItem::releaseAxis(QValue3DAxis *axis)
{
    graphScatter()->releaseAxis(axis);
}

/*!
 * Returns the list of all added axes.
 *
 * \sa addAxis()
 */
QList<QValue3DAxis *> Q3DScatterWidgetItem::axes() const
{
    QList<QAbstract3DAxis *> abstractAxes = graphScatter()->axes();
    QList<QValue3DAxis *> retList;
    for (QAbstract3DAxis *axis : std::as_const(abstractAxes))
        retList.append(static_cast<QValue3DAxis *>(axis));

    return retList;
}

/*!
 * \internal
 */
QQuickGraphsScatter *Q3DScatterWidgetItem::graphScatter()
{
    Q_D(Q3DScatterWidgetItem);
    return static_cast<QQuickGraphsScatter *>(d->m_graphsItem.get());
}

/*!
 * \internal
 */
const QQuickGraphsScatter *Q3DScatterWidgetItem::graphScatter() const
{
    Q_D(const Q3DScatterWidgetItem);
    return static_cast<const QQuickGraphsScatter *>(d->m_graphsItem.get());
}

QT_END_NAMESPACE
