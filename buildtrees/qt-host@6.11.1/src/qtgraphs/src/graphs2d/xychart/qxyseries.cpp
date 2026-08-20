// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "graphs2d/qabstractseries.h"
#include <QtGraphs/qxyseries.h>
#include <private/qxyseries_p.h>
#include <private/charthelpers_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QXYSeries
    \inmodule QtGraphs
    \ingroup graphs_2D
    \brief The QXYSeries class is a parent class for all x & y series classes.

    In QXYSeries, data points are defined as a list of QPointF, defining X and Y positions.

 \sa QLineSeries, QScatterSeries
*/
/*!
    \qmltype XYSeries
    \nativetype QXYSeries
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D
    \inherits AbstractSeries
    \brief A parent type for all x & y series types.

    In XYSeries, data points are defined as a list of point types, defining X and Y positions.
*/

/*!
    \fn void QXYSeries::pointReplaced(qsizetype index)
    This signal is emitted when a point is replaced at the position specified by
    \a index.
    \sa replace()
*/
/*!
    \qmlsignal XYSeries::pointReplaced(int index)
    This signal is emitted when a point is replaced at the position specified by
    \a index.
*/

/*!
    \fn void QXYSeries::pointsReplaced()
    This signal is emitted when all points are replaced.
*/
/*!
    \qmlsignal XYSeries::pointsReplaced()
    This signal is emitted when all points are replaced.
*/

/*!
    \fn void QXYSeries::pointsAdded(qsizetype start, qsizetype end)
    This signal is emitted when a list of points is appended.
    The indexes of the new added points are between \a start and \a end.
    \since 6.9
*/
/*!
    \qmlsignal XYSeries::pointsAdded(int start, int end)
    This signal is emitted when a list of points is appended.
    The indexes of the new added points are between \a start and \a end.
    \since 6.9
*/

/*!
    \qmlsignal XYSeries::colorChanged(color color)
    This signal is emitted when the line color changes to \a color.
*/

/*!
    \qmlsignal XYSeries::selectedColorChanged(color color)
    This signal is emitted when the color of selected series changes to \a color.
*/

/*!
    \qmlsignal XYSeries::selectedPointsChanged()
    This signal is emitted when the set of selected points changes.
*/

/*!
    \qmlsignal XYSeries::pointMarkerChanged()
    This signal is emitted when a point is changed.
*/

/*!
    \qmlsignal XYSeries::draggableChanged()
    This signal is emitted when a series becomes draggable by a mouse/touch or
    becomes fixed.
*/

/*!
    \qmlsignal XYSeries::clicked(point point)
    This signal is emitted when the user clicks or taps the \a point in the graph.
*/

/*!
    \qmlsignal XYSeries::doubleClicked(point point)
    This signal is emitted when the user double-clicks or double-taps the data \a point in
    the graph. The \a point is the point where the first press was triggered.
    This signal always occurs after \l clicked.
*/

/*!
    \qmlsignal XYSeries::pressed(point point)
    This signal is emitted when the user presses the data \a point in the graph
    and holds down the mouse button or gesture.
*/

/*!
    \qmlsignal XYSeries::released(point point)
    This signal is emitted when the user releases the previously pressed mouse
    button or gesture on the data \a point.
*/

QXYSeries::QXYSeries(QXYSeriesPrivate &dd, QObject *parent)
    : QAbstractSeries(dd, parent)
{
    QObject::connect(this, &QXYSeries::selectedPointsChanged, this, &QAbstractSeries::update);
    QObject::connect(this, &QXYSeries::pointAdded, this, &QAbstractSeries::update);
    QObject::connect(this, &QXYSeries::pointsAdded, this, &QAbstractSeries::update);
    QObject::connect(this, &QXYSeries::pointReplaced, this, &QAbstractSeries::update);
    QObject::connect(this, &QXYSeries::pointsReplaced, this, &QAbstractSeries::update);
    QObject::connect(this, &QXYSeries::pointRemoved, this, &QAbstractSeries::update);
    QObject::connect(this, &QXYSeries::pointsRemoved, this, &QAbstractSeries::update);
}

/*!
    \qmlmethod void XYSeries::append(real x, real y)
    Appends a point with the coordinates \a x and \a y to the series.
*/
/*!
    Appends a point with the coordinates \a x and \a y to the series.
*/
void QXYSeries::append(qreal x, qreal y)
{
    append(QPointF(x, y));
}

/*!
    \qmlmethod void XYSeries::append(point point)
    Appends a point with the coordinates \a point to the series.
*/
/*!
    Appends a point with the coordinates \a point to the series.
*/
void QXYSeries::append(QPointF point)
{
    Q_D(QXYSeries);
    if (d->m_graphTransition && d->m_graphTransition->initialized()
        && d->m_graphTransition->contains(QGraphAnimation::GraphAnimationType::GraphPoint)) {
        d->m_graphTransition->stop();
        if (isValidValue(point)) {
            d->m_graphTransition->onPointChanged(QGraphTransition::TransitionType::PointAdded,
                                                 d->m_points.size(),
                                                 point);
        }
    } else {
        d->m_points << point;
        emit pointAdded(d->m_points.size() - 1);
        emit countChanged();
    }
}

/*!
    \qmlmethod void XYSeries::append(list<point> points)
    Appends points with the coordinates \a points to the series.
    \note This is much faster than appending data points one by one.
    Emits \l pointsAdded when the points have been added.
*/
/*!
    Appends points with the coordinates \a points to the series.
    \note This is much faster than appending data points one by one.
    Emits \l pointsAdded when the points have been added.
*/
void QXYSeries::append(const QList<QPointF> &points)
{
    Q_D(QXYSeries);
    d->append(points);
}

/*!
    \qmlmethod void XYSeries::replace(real oldX, real oldY, real newX, real newY)
    Replaces the point with the coordinates \a oldX and \a oldY with the point
    with the coordinates \a newX and \a newY. Does nothing if the old point does
    not exist.
*/
/*!
    Replaces the point with the coordinates \a oldX and \a oldY with the point
    with the coordinates \a newX and \a newY. Does nothing if the old point does
    not exist.
*/
void QXYSeries::replace(qreal oldX, qreal oldY, qreal newX, qreal newY)
{
    replace(QPointF(oldX, oldY), QPointF(newX, newY));
}

/*!
    \qmlmethod void XYSeries::replace(point oldPoint, point newPoint)
    Replaces the point with the coordinates \a oldPoint with the point
    with the coordinates \a newPoint. Does nothing if the old point does
    not exist.
*/
/*!
    Replaces the point with the coordinates \a oldPoint with the point
    with the coordinates \a newPoint. Does nothing if the old point does
    not exist.
*/
void QXYSeries::replace(QPointF oldPoint, QPointF newPoint)
{
    Q_D(QXYSeries);
    qsizetype index = d->m_points.indexOf(oldPoint);
    if (index == -1)
        return;
    replace(index, newPoint);
}

/*!
    \qmlmethod void XYSeries::replace(int index, real newX, real newY)
    Replaces the point at the position specified by \a index with the point
    that has the coordinates \a newX and \a newY.
*/
/*!
    Replaces the point at the position specified by \a index with the point
    that has the coordinates \a newX and \a newY.
*/
void QXYSeries::replace(qsizetype index, qreal newX, qreal newY)
{
    replace(index, QPointF(newX, newY));
}

/*!
    \qmlmethod void XYSeries::replace(int index, point newPoint)
    Replaces the point at the position specified by \a index with the point
    that has the coordinates \a newPoint.
*/
/*!
    Replaces the point at the position specified by \a index with the point
    that has the coordinates \a newPoint.
*/
void QXYSeries::replace(qsizetype index, QPointF newPoint)
{
    Q_D(QXYSeries);

    if (index < 0 || index >= d->m_points.size())
        return;

    if (d->m_graphTransition && d->m_graphTransition->initialized()
        && d->m_graphTransition->contains(QGraphAnimation::GraphAnimationType::GraphPoint)) {
        d->m_graphTransition->stop();
        if (isValidValue(newPoint)) {
            d->m_graphTransition->onPointChanged(QGraphTransition::TransitionType::PointReplaced,
                                                 index,
                                                 newPoint);
        }
    } else {
        d->m_points[index] = newPoint;
        emit pointReplaced(index);
    }
}

/*!
    \qmlmethod void XYSeries::replace(list<point> points)
    Replaces the current points with the points specified by \a points
    \note This is much faster than replacing data points one by one, or first
    clearing all data, and then appending the new data. Emits \l pointsReplaced
    when the points have been replaced.
*/
/*!
    Replaces the current points with the points specified by \a points
    \note This is much faster than replacing data points one by one, or first
    clearing all data, and then appending the new data. Emits \l pointsReplaced
    when the points have been replaced.
*/
void QXYSeries::replace(const QList<QPointF> &points)
{
    Q_D(QXYSeries);
    bool hasDifferentSize = d->m_points.size() != points.size();
    d->m_points = points;
    emit pointsReplaced();
    if (hasDifferentSize)
        emit countChanged();
}

/*!
    \qmlmethod void XYSeries::remove(real x, real y)
    Removes the point with the coordinates \a x and \a y from the series. Does
    nothing if the point does not exist.
*/
/*!
    Removes the point with the coordinates \a x and \a y from the series. Does
    nothing if the point does not exist.
*/
void QXYSeries::remove(qreal x, qreal y)
{
    remove(QPointF(x, y));
}

/*!
    \qmlmethod void XYSeries::remove(point point)
    Removes the point with the coordinates \a point from the series. Does
    nothing if the point does not exist.
*/
/*!
    Removes the point with the coordinates \a point from the series. Does
    nothing if the point does not exist.
*/
void QXYSeries::remove(QPointF point)
{
    Q_D(QXYSeries);
    qsizetype index = d->m_points.indexOf(point);
    if (index == -1)
        return;
    remove(index);
}

/*!
    \qmlmethod void XYSeries::remove(int index)
    Removes the point at the position specified by \a index from the series.
*/
/*!
    Removes the point at the position specified by \a index from the series.
*/
void QXYSeries::remove(qsizetype index)
{
    Q_D(QXYSeries);

    if (index < 0 || index >= d->m_points.size())
        return;

    if (d->m_graphTransition && d->m_graphTransition->initialized()
        && d->m_graphTransition->contains(QGraphAnimation::GraphAnimationType::GraphPoint)) {
        d->m_graphTransition->stop();
        d->m_graphTransition->onPointChanged(QGraphTransition::TransitionType::PointRemoved,
                                             index,
                                             {});
    } else {
        d->m_points.remove(index);
        bool callSignal = false;
        d->setPointSelected(index, false, callSignal);

        emit pointRemoved(index);
        emit countChanged();
        if (callSignal)
            emit selectedPointsChanged();
    }
}

/*!
    \qmlmethod void XYSeries::removeMultiple(int index, int count)
    Removes the number of points specified by \a count from the series starting
    at the position specified by \a index.
*/
/*!
    Removes the number of points specified by \a count from the series starting
    at the position specified by \a index.
*/
void QXYSeries::removeMultiple(qsizetype index, qsizetype count)
{
    // This function doesn't overload remove as there is chance for it to get mixed up with
    // remove(qreal, qreal) overload in some implicit casting cases.
    Q_D(QXYSeries);

    if (index < 0 || count < 1 || index + count > d->m_points.size())
        return;

    if (count > 0) {
        if (d->m_graphTransition && d->m_graphTransition->initialized()
            && d->m_graphTransition->contains(QGraphAnimation::GraphAnimationType::GraphPoint)) {
            d->m_graphTransition->stop();
        }

        d->m_points.remove(index, count);

        bool callSignal = false;
        if (!d->m_selectedPoints.empty()) {
            QSet<qsizetype> selectedAfterRemoving;

            for (const qsizetype &selectedPointIndex : std::as_const(d->m_selectedPoints)) {
                if (selectedPointIndex < index) {
                    selectedAfterRemoving << selectedPointIndex;
                } else if (selectedPointIndex >= index + count) {
                    selectedAfterRemoving << selectedPointIndex - int(count);
                    callSignal = true;
                } else {
                    callSignal = true;
                }
            }

            d->m_selectedPoints = selectedAfterRemoving;
        }

        emit pointsRemoved(index, count);
        emit countChanged();
        if (callSignal)
            emit selectedPointsChanged();
    }
}

/*!
    \qmlmethod bool XYSeries::take(point point)
    Takes a point, specified by \a point, out of the series if found. Returns \c true if
    the operation is successful.
*/
/*!
    Takes a point, specified by \a point, out of the series if found. Returns \c true if
    the operation is successful.
*/
bool QXYSeries::take(QPointF point)
{
    Q_D(QXYSeries);

    for (int i = 0; i < d->m_points.size(); ++i) {
        if (d->m_points[i] == point) {
            d->m_points.removeAt(i);
            return true;
        }
    }

    return false;
}

/*!
    \qmlmethod void XYSeries::insert(int index, point point)
    Inserts a point with the coordinates \a point to the position specified
    by \a index in the series. If the index is 0 or less than 0, the point is
    prepended to the list of points. If the index is equal to or greater than
    than the number of points in the series, the point is appended to the
    list of points.
*/
/*!
    Inserts a point with the coordinates \a point to the position specified
    by \a index in the series. If the index is 0 or less than 0, the point is
    prepended to the list of points. If the index is equal to or greater than
    than the number of points in the series, the point is appended to the
    list of points.
*/
void QXYSeries::insert(qsizetype index, QPointF point)
{
    Q_D(QXYSeries);

    index = qMax(0, qMin(index, d->m_points.size()));

    d->m_points.insert(index, point);

    bool callSignal = false;
    if (!d->m_selectedPoints.isEmpty()) {
        // if point was inserted we need to move already selected points by 1
        QSet<qsizetype> selectedAfterInsert;
        for (const auto &value : std::as_const(d->m_selectedPoints)) {
            if (value >= index) {
                selectedAfterInsert << value + 1;
                callSignal = true;
            } else {
                selectedAfterInsert << value;
            }
        }
        d->m_selectedPoints = selectedAfterInsert;
    }

    emit pointAdded(index);
    if (callSignal)
        emit selectedPointsChanged();
}

/*!
    \qmlmethod void XYSeries::clear()
    Removes all points from the series.
*/
/*!
    Removes all points from the series.
*/
void QXYSeries::clear()
{
    Q_D(QXYSeries);
    removeMultiple(0, d->m_points.size());
}

/*!
    \qmlmethod bool XYSeries::isPointSelected(int index)
    Returns true if point at given \a index is among selected points and false otherwise.
    \note Selected points are drawn using the selected color if it was specified.
    \sa selectedPoints, setPointSelected(), selectedColor
 */
/*!
    Returns true if point at given \a index is among selected points and false otherwise.
    \note Selected points are drawn using the selected color if it was specified.
    \sa selectedPoints, setPointSelected(), setSelectedColor()
 */
bool QXYSeries::isPointSelected(qsizetype index) const
{
    Q_D(const QXYSeries);
    return d->isPointSelected(index);
}

/*!
    \qmlmethod void XYSeries::selectPoint(int index)
    Marks point at \a index as selected.
    \note Emits QXYSeries::selectedPointsChanged
    \sa setPointSelected()
 */
/*!
    Marks point at \a index as selected.
    \note Emits QXYSeries::selectedPointsChanged
    \sa setPointSelected()
 */
void QXYSeries::selectPoint(qsizetype index)
{
    setPointSelected(index, true);
}

/*!
    \qmlmethod void XYSeries::deselectPoint(int index)
    Deselects point at given \a index.
    \note Emits QXYSeries::selectedPointsChanged
    \sa setPointSelected()
 */
/*!
    Deselects point at given \a index.
    \note Emits QXYSeries::selectedPointsChanged
    \sa setPointSelected()
 */
void QXYSeries::deselectPoint(qsizetype index)
{
    setPointSelected(index, false);
}

/*!
    \qmlmethod void XYSeries::setPointSelected(int index, bool selected)
    Marks point at given \a index as either selected or deselected as specified by \a selected.
    \note Selected points are drawn using the selected color if it was specified. Emits QXYSeries::selectedPointsChanged
    \sa selectAllPoints(), selectedColor
 */
/*!
    Marks point at given \a index as either selected or deselected as specified by \a selected.
    \note Selected points are drawn using the selected color if it was specified. Emits QXYSeries::selectedPointsChanged
    \sa selectAllPoints(), setSelectedColor()
 */
void QXYSeries::setPointSelected(qsizetype index, bool selected)
{
    Q_D(QXYSeries);

    bool callSignal = false;
    d->setPointSelected(index, selected, callSignal);

    if (callSignal)
        emit selectedPointsChanged();
}

/*!
    \qmlmethod void XYSeries::selectAllPoints()
    Marks all points in the series as selected,
    \note Emits QXYSeries::selectedPointsChanged
    \sa setPointSelected()
 */
/*!
    Marks all points in the series as selected,
    \note Emits QXYSeries::selectedPointsChanged
    \sa setPointSelected()
 */
void QXYSeries::selectAllPoints()
{
    Q_D(QXYSeries);

    bool callSignal = false;
    for (int i = 0; i < d->m_points.size(); ++i)
        d->setPointSelected(i, true, callSignal);

    if (callSignal)
        emit selectedPointsChanged();
}

/*!
    \qmlmethod void XYSeries::deselectAllPoints()
    Deselects all points in the series.
    \note Emits QXYSeries::selectedPointsChanged
    \sa setPointSelected()
 */
/*!
    Deselects all points in the series.
    \note Emits QXYSeries::selectedPointsChanged
    \sa setPointSelected()
 */
void QXYSeries::deselectAllPoints()
{
    Q_D(QXYSeries);

    bool callSignal = false;
    for (int i = 0; i < d->m_points.size(); ++i)
        d->setPointSelected(i, false, callSignal);

    if (callSignal)
        emit selectedPointsChanged();
}

/*!
    \qmlmethod void XYSeries::selectPoints(list<int> indexes)
    Marks multiple points passed in a \a indexes list as selected.
    \note Emits QXYSeries::selectedPointsChanged
    \sa setPointSelected()
 */
/*!
    Marks multiple points passed in a \a indexes list as selected.
    \note Emits QXYSeries::selectedPointsChanged
    \sa setPointSelected()
 */
void QXYSeries::selectPoints(const QList<qsizetype> &indexes)
{
    Q_D(QXYSeries);

    bool callSignal = false;
    for (const qsizetype &index : indexes)
        d->setPointSelected(index, true, callSignal);

    if (callSignal)
        emit selectedPointsChanged();
}

/*!
    \qmlmethod void XYSeries::deselectPoints(list<int> indexes)
    Marks multiple points passed in a \a indexes list as deselected.
    \note Emits QXYSeries::selectedPointsChanged
    \sa setPointSelected()
 */
/*!
    Marks multiple points passed in a \a indexes list as deselected.
    \note Emits QXYSeries::selectedPointsChanged
    \sa setPointSelected()
 */
void QXYSeries::deselectPoints(const QList<qsizetype> &indexes)
{
    Q_D(QXYSeries);

    bool callSignal = false;
    for (const qsizetype &index : indexes)
        d->setPointSelected(index, false, callSignal);

    if (callSignal)
        emit selectedPointsChanged();
}

/*!
    \qmlmethod void XYSeries::toggleSelection(list<int> indexes)
    Changes selection state of points at given \a indexes to the opposite one.
    \note Emits QXYSeries::selectedPointsChanged
    \sa setPointSelected()
 */
/*!
    Changes selection state of points at given \a indexes to the opposite one.
    \note Emits QXYSeries::selectedPointsChanged
    \sa setPointSelected()
 */
void QXYSeries::toggleSelection(const QList<qsizetype> &indexes)
{
    Q_D(QXYSeries);

    bool callSignal = false;
    for (const qsizetype &index : indexes)
        d->setPointSelected(index, !isPointSelected(index), callSignal);

    if (callSignal)
        emit selectedPointsChanged();
}

/*!
    \property QXYSeries::selectedPoints
    \brief The indexes of the points which are currently selected.
*/
/*!
    \qmlproperty list<int> XYSeries::selectedPoints
    The indexes of the points which are currently selected.
*/

/*!
    Returns a list of points indexes marked as selected.
    Selected points are visible regardless of points visibility.
    \sa setPointSelected()
 */
QList<qsizetype> QXYSeries::selectedPoints() const
{
    Q_D(const QXYSeries);
    return QList<qsizetype>(d->m_selectedPoints.begin(), d->m_selectedPoints.end());
}

/*!
    \property QXYSeries::count
    \brief Returns the number of data points in a series.
*/
/*!
    \qmlproperty int XYSeries::count
    Returns the number of data points in a series.
*/
qsizetype QXYSeries::count() const
{
    Q_D(const QXYSeries);
    return d->m_points.size();
}

/*!
    Returns the points in the series.
*/
QList<QPointF> QXYSeries::points() const
{
    Q_D(const QXYSeries);
    return d->m_points;
}

/*!
    \qmlmethod point XYSeries::at(int index)
    Returns the point at the position specified by \a index. Returns (0, 0) if
    the index is not valid.
*/
/*!
    Returns the point at the position specified by \a index. Returns (0, 0) if
    the index is not valid.
*/
QPointF QXYSeries::at(qsizetype index) const
{
    Q_D(const QXYSeries);
    return d->m_points.at(index);
}

/*!
    \qmlmethod int XYSeries::find(point point)
    Finds and returns the index of the first matching point found as defined by \a point.
    Returns -1 if the point is not found.
*/
/*!
    Finds and returns the index of the first matching point found as defined by \a point.
    Returns -1 if the point is not found.
*/
qsizetype QXYSeries::find(QPointF point) const
{
    Q_D(const QXYSeries);

    for (qsizetype i = 0; i < d->m_points.size(); ++i) {
        if (d->m_points[i] == point)
            return i;
    }

    return -1;
}

QXYSeries::~QXYSeries() {}

/*!
    \property QXYSeries::color
    \brief The main color of the series. For QLineSeries this means the line color and
    for QScatterSeries the color of the point.
*/
/*!
    \qmlproperty color XYSeries::color
    The main color of the series. For LineSeries this means the line color and
    for ScatterSeries the color of the point
*/
void QXYSeries::setColor(QColor newColor)
{
    Q_D(QXYSeries);
    if (color() != newColor) {
        d->m_color = newColor;
        emit colorChanged(newColor);
    } else {
        qCDebug(lcProperties2D) << "QXYSeries::setColor. Color is already set to:"
                                << newColor;
    }
}

QColor QXYSeries::color() const
{
    Q_D(const QXYSeries);
    return d->m_color;
}

/*!
    \property QXYSeries::selectedColor
    \brief The color of selected points.
*/
/*!
    \qmlproperty color XYSeries::selectedColor
    The color of selected points.
*/
void QXYSeries::setSelectedColor(QColor color)
{
    Q_D(QXYSeries);
    if (selectedColor() != color) {
        d->m_selectedColor = color;
        emit selectedColorChanged(color);
    } else {
        qCDebug(lcProperties2D) << "QXYSeries::setSelectedColor. Selected color is already set to:"
                                << color;
    }
}

QColor QXYSeries::selectedColor() const
{
    Q_D(const QXYSeries);
    return d->m_selectedColor;
}

/*!
    \property QXYSeries::pointDelegate
    \brief A custom QML Component used as a marker for data points.

    The dynamic properties available for this component are:

    \table
    \header
        \li Type
        \li Name
        \li Description
    \row
        \li bool
        \li pointSelected
        \li This value is true when the point is selected, meaning that the point index
        is in \l{QXYSeries::selectedPoints}.
    \row
        \li QColor
        \li pointColor
        \li The color of the series. This value comes either from the \l QGraphsTheme
        or from \l{QXYSeries::color} if the \l QXYSeries overrides the color.
    \row
        \li QColor
        \li pointBorderColor
        \li The border color of the series. This value comes from the \l QGraphsTheme.
    \row
        \li QColor
        \li pointSelectedColor
        \li The selected color of the series. This value comes either from the \l QGraphsTheme
        or from \l{QXYSeries::selectedColor} if the \l QXYSeries overrides the color.
    \row
        \li qreal
        \li pointBorderWidth
        \li The border width of the series. This value comes from the \l QGraphsTheme.
    \row
        \li qreal
        \li pointValueX
        \li The value of the \l{QXYPoint::x} at this position.
    \row
        \li qreal
        \li pointValueY
        \li The value of the \l{QXYPoint::y} at this position.
    \row
        \li int
        \li pointIndex
        \li The index of the point, from 0 to the amount of points - 1. [since 6.9]
    \endtable

    To use any of these, add property with the defined name into your custom component.
    For example \c{"property color pointColor"} and \c{"property real pointValueX"}.
*/
/*!
    \qmlproperty Component XYSeries::pointDelegate
    A custom QML Component used as a marker for data points.

    The dynamic properties available for this component are:

    \table
    \header
        \li Type
        \li Name
        \li Description
    \row
        \li bool
        \li pointSelected
        \li This value is true when the point is selected.
    \row
        \li Color
        \li pointColor
        \li The color of the series. This value comes either from the \l GraphsTheme
        or from \l{XYSeries::color} if the \l XYSeries overrides the color.
    \row
        \li Color
        \li pointBorderColor
        \li The border color of the series. This value comes from the \l GraphsTheme.
    \row
        \li Color
        \li pointSelectedColor
        \li The selected color of the series. This value comes either from the \l GraphsTheme
        or from \l{XYSeries::selectedColor} if the \l XYSeries overrides the color.
    \row
        \li real
        \li pointBorderWidth
        \li The border width of the series. This value comes from the \l GraphsTheme.
    \row
        \li real
        \li pointValueX
        \li The value of the \l{XYPoint::x} at this position.
    \row
        \li real
        \li pointValueY
        \li The value of the \l{XYPoint::y} at this position.
    \row
        \li int
        \li pointIndex
        \li The index of the point, from 0 to the amount of points - 1. [since 6.9]
    \endtable

    To use any of these, add property with the defined name into your custom component.
    For example \c{"property color pointColor"} and \c{"property real pointValueX"}.
*/
QQmlComponent *QXYSeries::pointDelegate() const
{
    Q_D(const QXYSeries);
    return d->m_pointDelegate;
}

void QXYSeries::setPointDelegate(QQmlComponent *newPointDelegate)
{
    Q_D(QXYSeries);
    if (d->m_pointDelegate == newPointDelegate) {
        qCDebug(lcProperties2D) << "QXYSeries::setPointDelegate. Point delegate is already set to:"
                                << newPointDelegate;
        return;
    }

    d->m_pointDelegate = newPointDelegate;
    emit pointDelegateChanged();
    emit update();
}

/*!
    \property QXYSeries::draggable
    \brief Controls if the series is draggable.

    Controls if the series can be dragged with mouse/touch.
    By default, \a draggable is set to \c false.
*/
/*!
    \qmlproperty bool XYSeries::draggable
    Controls if the series can be dragged with mouse/touch.
    By default, \a draggable is set to \c false.
*/
bool QXYSeries::isDraggable() const
{
    Q_D(const QXYSeries);
    return d->m_draggable;
}

void QXYSeries::setDraggable(bool newDraggable)
{
    Q_D(QXYSeries);
    if (d->m_draggable == newDraggable) {
        qCDebug(lcProperties2D) << "QXYSeries::setDraggable. Cannot set draggable, it is already set to:"
                                << newDraggable;
        return;
    }
    d->m_draggable = newDraggable;
    emit draggableChanged();
}

QXYSeries &QXYSeries::operator<<(QPointF point)
{
    append(point);
    return *this;
}

QXYSeries &QXYSeries::operator<< (const QList<QPointF>& points)
{
    append(points);
    return *this;
}

QXYSeriesPrivate::QXYSeriesPrivate(QAbstractSeries::SeriesType type)
    : QAbstractSeriesPrivate(type)
{}

void QXYSeriesPrivate::setPointSelected(qsizetype index, bool selected, bool &callSignal)
{
    if (index < 0 || index > m_points.size() - 1) {
        qCWarning(lcProperties2D, "tried to use invalid index: %" PRIdQSIZETYPE, index);
        return;
    }

    if (selected) {
        if (!isPointSelected(index)) {
            m_selectedPoints << index;
            callSignal = true;
        }
    } else {
        if (isPointSelected(index)) {
            m_selectedPoints.remove(index);
            callSignal = true;
        }
    }
}

bool QXYSeriesPrivate::isPointSelected(qsizetype index) const
{
    return m_selectedPoints.contains(index);
}

void QXYSeriesPrivate::append(const QList<QPointF> &points)
{
    bool anim = m_graphTransition && m_graphTransition->initialized()
                && m_graphTransition->contains(QGraphAnimation::GraphAnimationType::GraphPoint);

    if (anim) {
        m_graphTransition->stop();
        qsizetype index = m_points.size();

        for (auto point : points) {
            if (isValidValue(point)) {
                m_graphTransition->stop();
                m_graphTransition->onPointChanged(QGraphTransition::TransitionType::PointAdded,
                                                  index,
                                                  point);
                index++;
            }
        }
    } else {
        qsizetype start = m_points.size();
        m_points.append(points);

        Q_Q(QXYSeries);
        Q_EMIT q->pointsAdded(start, m_points.size() - 1);
        Q_EMIT q->countChanged();
    }
}

QT_END_NAMESPACE

#include "moc_qxyseries.cpp"
