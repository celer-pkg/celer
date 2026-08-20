// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <private/qspline3dseries_p.h>
#include "qgraphs3dlogging_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QSpline3DSeries
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \since 6.9
 * \brief The QSpline3DSeries class represents a data series as a spline.
 *
 * Spline graphs are used to show information as a series of data points connected
 * by a curved or straight Catmull-Rom spline.
 *
 * This class manages the spline specific visual elements.
 *
 * Spline3DSeries extends the Scatter3DSeries API.
 */

/*!
 * \qmltype Spline3DSeries
 * \nativetype QSpline3DSeries
 * \inqmlmodule QtGraphs
 * \ingroup graphs_qml_3D
 * \inherits Scatter3DSeries
 * \since 6.9
 * \brief Represents a data series in a 3D spline graph.
 *
 * Spline graphs are used to show information as a series of data points connected
 * by a curved or straight Catmull-Rom spline.
 *
 * This type manages the spline specific visual elements.
 *
 */

/*!
 * \qmlproperty bool Spline3DSeries::splineVisible
 *
 * Visibility of the spline. The default value is \c true.
 */

/*!
 * \qmlproperty real Spline3DSeries::splineTension
 *
 * The tension of the spline.
 *
 * The spline uses maximum curvature for segments at a value of \c 0.0
 * Segments are completely straight at a value of \c 1.0
 * Must be between \c 0.0 and \c 1.0
 * The default value is \c 0.0
 *
 */

/*!
 * \qmlproperty real Spline3DSeries::splineKnotting
 *
 * The knot parametrization of the spline.
 *
 * This parameter can change the profile of the curve.
 * The spline is classified as a uniform Catmull-Rom spline at a value of \c 0.0,
 * a centripetal Catmull-Rom spline at a value of \c 0.5,
 * and a chordal Catmull-Rom spline at a value of \c 1.0.
 *
 * The value must be between \c 0.0 and \c 1.0.
 * The default value is \c 0.5.
 *
 */

/*!
 * \qmlproperty bool Spline3DSeries::splineLooping
 *
 * Determines whether the spline loops.
 *
 * This adds a spline segment between the first and last points of the series
 * connecting the spline into a loop.
 *
 * The default value is \c false
 *
 */

/*!
 * \qmlproperty int Spline3DSeries::splineResolution
 *
 * The resolution of the segments spline.
 * 
 * The number of vertices per spline segment,
 * which is defined as the part between two points.
 * 
 * Must be a value above \c 2. 
 * The default value is \c 10.
 */

/*!
 * \qmlproperty color Spline3DSeries::splineColor
 *
 * The color of the spline.
 *
 */

/*!
    \qmlsignal Scatter3DSeries::splineVisibilityChanged(bool visible)

    This signal is emitted when splineVisible changes to \a visible.
*/

/*!
    \qmlsignal Scatter3DSeries::splineTensionChanged(real tension)

    This signal is emitted when splineTension changes to \a tension.
*/

/*!
    \qmlsignal Scatter3DSeries::splineKnottingChanged(real knotting)

    This signal is emitted when splineKnotting changes to \a knotting.
*/

/*!
    \qmlsignal Scatter3DSeries::splineLoopingChanged(bool looping)

    This signal is emitted when splineLooping changes to \a looping.
*/

/*!
    \qmlsignal Scatter3DSeries::splineColorChanged(color color)

    This signal is emitted when splineColor changes to \a color.
*/

/*!
    \qmlsignal Scatter3DSeries::splineResolutionChanged(int resolution)

    This signal is emitted when splineResolution changes to \a resolution.
*/

/*!
 * Constructs a spline 3D series with the parent \a parent.
 */
QSpline3DSeries ::QSpline3DSeries(QObject *parent)
    : QScatter3DSeries(*(new QSpline3DSeriesPrivate()), parent)
{
    Q_D(QScatter3DSeries);
    // Default proxy
    d->setDataProxy(new QScatterDataProxy);
}

/*!
 * Constructs a spline 3D series with the data proxy \a dataProxy and the
 * parent \a parent.
 */
QSpline3DSeries ::QSpline3DSeries(QScatterDataProxy *dataProxy, QObject *parent)
    : QScatter3DSeries(*(new QSpline3DSeriesPrivate()), parent)
{
    Q_D(QScatter3DSeries);
    // Default proxy
    d->setDataProxy(dataProxy);
}

/*!
 * \internal
 */
QSpline3DSeries ::QSpline3DSeries(QSpline3DSeriesPrivate &dd, QObject *parent)
    : QScatter3DSeries(dd, parent)
{}

/*!
 * Deletes the spline 3D series.
 */
QSpline3DSeries::~QSpline3DSeries() {}

/*!
 * \property QSpline3DSeries::splineVisible
 *
 * \brief Visibility of the spline.
 *
 * Visibility of the spline.
 * The default value is \c true.
 *
 */
void QSpline3DSeries::setSplineVisible(bool visible)
{
    Q_D(QSpline3DSeries);
    if (d->m_splineVisible == visible) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << visible;
        return;
    }
    d->m_splineVisible = visible;
    emit splineVisibilityChanged(visible);
}

bool QSpline3DSeries::isSplineVisible() const
{
    const Q_D(QSpline3DSeries);
    return d->m_splineVisible;
}

/*!
 * \property QSpline3DSeries::splineTension
 *
 * \brief The tension of the spline.
 *
 * The spline uses maximum curvature for segments at a value of \c 0.0
 * Segments are completely straight at a value of \c 1.0
 * Must be between \c 0.0 and \c 1.0
 * The default value is \c 0.0
 *
 */
void QSpline3DSeries::setSplineTension(qreal tension)
{
    Q_D(QSpline3DSeries);
    if (tension < 0.0f || tension > 1.0f) {
        qCWarning(lcProperties3D, "%s invalid tension. Valid range for tension is 0.0f...1.0f",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    } else if (d->m_tension == tension) {
        qCDebug(lcProperties3D, "%s value is already set to: %f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), tension);
        return;
    }
    d->m_tension = tension;
    emit splineTensionChanged(tension);
}

qreal QSpline3DSeries::splineTension() const
{
    const Q_D(QSpline3DSeries);
    return d->m_tension;
}

/*!
 * \property QSpline3DSeries::splineKnotting
 *
 * \brief The knot parametrization of the spline.
 *
 * This parameter can change the profile of the curve.
 * The spline is classified as a uniform Catmull-Rom spline at a value of \c 0.0,
 * a centripetal Catmull-Rom spline at a value of \c 0.5,
 * and a chordal Catmull-Rom spline at a value of \c 1.0.
 *
 * The value must be between \c 0.0 and \c 1.0.
 * The default value is \c 0.5.
 *
 */
void QSpline3DSeries::setSplineKnotting(qreal knotting)
{
    Q_D(QSpline3DSeries);
    if (knotting < 0.0f || knotting > 1.0f) {
        qCWarning(lcProperties3D, "%s invalid knotting. Valid range for knotting is 0.0f...1.0f",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    } else if (d->m_knotting == knotting) {
        qCDebug(lcProperties3D, "%s value is already set to: %f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), knotting);
        return;
    }
    d->m_knotting = knotting;
    emit splineKnottingChanged(knotting);
}

qreal QSpline3DSeries::splineKnotting() const
{
    const Q_D(QSpline3DSeries);
    return d->m_knotting;
}

/*!
 * \property QSpline3DSeries::splineLooping
 *
 * \brief Determines whether the spline loops.
 *
 * This adds a spline segment between the first and last points of the series
 * connecting the spline into a loop.
 *
 * The default value is \c false
 *
 */
void QSpline3DSeries::setSplineLooping(bool looping)
{
    Q_D(QSpline3DSeries);
    if (d->m_looping == looping) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << looping;
        return;
    }
    d->m_looping = looping;
    emit splineLoopingChanged(looping);
}

bool QSpline3DSeries::isSplineLooping() const
{
    const Q_D(QSpline3DSeries);
    return d->m_looping;
}

/*!
 * \property QSpline3DSeries::splineColor
 *
 * \brief The color of the spline.
 *
 */
void QSpline3DSeries::setSplineColor(QColor color)
{
    Q_D(QSpline3DSeries);
    if (d->m_splineColor == color) {
        qCDebug(lcProperties3D) << __FUNCTION__
            << "value is already set to:" << color;
        return;
    }
    d->m_splineColor = color;
    emit splineColorChanged(color);
}

QColor QSpline3DSeries::splineColor() const
{
    const Q_D(QSpline3DSeries);
    return d->m_splineColor;
}

/*!
 * \property QSpline3DSeries::splineResolution
 *
 * \brief The resolution of the segments spline.
 * 
 * The number of vertices per spline segment,
 * which is defined as the part between two points.
 * 
 * Must be a value above \c 2. 
 * The default value is \c 10.
 */

void QSpline3DSeries::setSplineResolution(int resolution)
{
    Q_D(QSpline3DSeries);
    if (resolution < 2) {
        qCWarning(lcProperties3D, "%s invalid resolution. The resolution must be 2 or above",
                  qUtf8Printable(QLatin1String(__FUNCTION__)));
        return;
    } else if (d->m_resolution == resolution) {
        qCDebug(lcProperties3D, "%s value is already set to: %d",
                qUtf8Printable(QLatin1String(__FUNCTION__)), resolution);
        return;
    }
    d->m_resolution = resolution;
    emit splineResolutionChanged(resolution);
}

int QSpline3DSeries::splineResolution() const
{
    const Q_D(QSpline3DSeries);
    return d->m_resolution;
}

// QSpline3DSeriesPrivate

QSpline3DSeriesPrivate::QSpline3DSeriesPrivate()
    : QScatter3DSeriesPrivate()
{}

QSpline3DSeriesPrivate::~QSpline3DSeriesPrivate() {}

QT_END_NAMESPACE

#include "moc_qspline3dseries.cpp"
