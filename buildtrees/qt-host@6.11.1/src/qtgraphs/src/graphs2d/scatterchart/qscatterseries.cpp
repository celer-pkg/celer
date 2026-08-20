// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "graphs2d/xychart/qxyseries_p.h"
#include <QtGraphs/qscatterseries.h>
#include <private/qxypoint_p.h>
#include <private/qscatterseries_p.h>
#include <private/qgraphsview_p.h>
#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

/*!
    \class QScatterSeries
    \inmodule QtGraphs
    \ingroup graphs_2D
    \brief The QScatterSeries class presents data in scatter graphs.

    The scatter data is displayed as a collection of points on the graph. For
    each point, two values are specified that determine its position on the
    horizontal axis and the vertical axis.
*/
/*!
    \qmltype ScatterSeries
    \nativetype QScatterSeries
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D
    \inherits XYSeries

    \brief The ScatterSeries type presents data in scatter graphs.

    The scatter data is displayed as a collection of points on the graph. For
    each point, two values are specified that determine its position on the
    horizontal axis and the vertical axis.

    \image graphs2d-scatter.png

    You can represent scatter data by creating a ScatterSeries inside
    GraphsView. The data can be added to the graph by creating XYPoints as
    children for the ScatterSeries that define the x and y values of each
    point.

    \code
    GraphsView {
        anchors.fill: parent
        axisX: ValueAxis {
            max: 3
        }
        axisY: ValueAxis {
            max: 3
        }

        ScatterSeries {
            color: "#00ff00"
            XYPoint { x: 0.5; y: 0.5 }
            XYPoint { x: 1; y: 1 }
            XYPoint { x: 2; y: 2 }
            XYPoint { x: 2.5; y: 1.5 }
        }
    }
    \endcode

    Multiple scatter graphs can be created by adding multiple ScatterSeries
    as children of GraphsView. In such cases only one series should define
    the axis used as multiple definitions only override the earlier ones.

    \code
    GraphsView {
        anchors.fill: parent
        ScatterSeries {
            color: "#00ff00"
            axisX: ValueAxis {
                max: 3
            }
            axisY: ValueAxis {
                max: 3
            }

            XYPoint { x: 0.5; y: 0.5 }
            XYPoint { x: 1; y: 1 }
            XYPoint { x: 2; y: 2 }
            XYPoint { x: 2.5; y: 1.5 }
        }

        ScatterSeries {
            color: "#ff0000"
            XYPoint { x: 0.5; y: 3 }
            XYPoint { x: 1; y: 2 }
            XYPoint { x: 2; y: 2.5 }
            XYPoint { x: 2.5; y: 1 }
        }
    }
    \endcode
*/

/*!
    \qmlproperty Component ScatterSeries::pointDelegate
    Marks points with the given QML component.

    \code
        pointDelegate: Image {
            source: "images/happy_box.png"
        }
    \endcode
*/

QScatterSeries::QScatterSeries(QObject *parent)
    : QXYSeries(*(new QScatterSeriesPrivate()), parent)
{}

QScatterSeries::~QScatterSeries() {}

QScatterSeries::QScatterSeries(QScatterSeriesPrivate &dd, QObject *parent)
    : QXYSeries(dd, parent)
{}

void QScatterSeries::componentComplete()
{
    Q_D(QScatterSeries);

    for (auto *child : children()) {
        if (auto point = qobject_cast<QXYPoint *>(child)) {
            append(point->x(), point->y());
            qCDebug(lcSeries2D, "append points x: %f, y: %f to scatterSeries",
                    point->x(),
                    point->y());
        }
    }

    if (d->m_graphTransition)
        d->m_graphTransition->initialize();

    qCDebug(lcEvents2D, "QScatterSeries::componentComplete");

    QAbstractSeries::componentComplete();
}

QAbstractSeries::SeriesType QScatterSeries::type() const
{
    return QAbstractSeries::SeriesType::Scatter;
}

QScatterSeriesPrivate::QScatterSeriesPrivate()
 : QXYSeriesPrivate(QAbstractSeries::SeriesType::Scatter)
{
}

QT_END_NAMESPACE

#include "moc_qscatterseries.cpp"
