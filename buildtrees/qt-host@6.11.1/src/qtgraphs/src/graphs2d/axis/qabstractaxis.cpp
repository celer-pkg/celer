// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtGraphs/QAbstractAxis>
#include <private/qabstractaxis_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QAbstractAxis
    \inmodule QtGraphs
    \ingroup graphs_2D
    \brief The QAbstractAxis class is a base class used for specialized axis classes.

    Each series can be bound to only one horizontal and vertical axis.

    The properties and visibility of various axis elements, such as axis line, title, labels,
    and grid lines, can be individually controlled.
*/

/*!
    \qmltype AbstractAxis
    \nativetype QAbstractAxis
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D

    \brief A base type used for specialized axis types.

    Each series can be bound to only one horizontal and vertical axis.

    The properties and visibility of various axis elements, such as axis line, title, labels,
    and grid lines, can be individually controlled.
*/

/*!
    \enum QAbstractAxis::AxisType

    This enum type specifies the type of the axis object.

    \value Value
    \value BarCategory
    \value DateTime
*/

/*!
    \fn void QAbstractAxis::type() const
    Returns the type of the axis.
*/

/*!
    \property QAbstractAxis::visible
    \brief The visibility of the axis.
    By default, the value is \c true.
*/
/*!
    \qmlproperty bool AbstractAxis::visible
    The visibility of the axis. By default, the value is \c true.
*/

/*!
    \property QAbstractAxis::lineVisible
    \brief The visibility of the axis line.
    By default, the value is \c true.
*/
/*!
    \qmlproperty bool AbstractAxis::lineVisible
    The visibility of the axis line. By default, the value is \c true.
*/

/*!
    \property QAbstractAxis::labelsVisible
    \brief Whether axis labels are visible.
    By default, the value is \c true.
*/
/*!
    \qmlproperty bool AbstractAxis::labelsVisible
    The visibility of axis labels. By default, the value is \c true.
*/

/*!
    \property QAbstractAxis::labelsAngle
    \brief The angle of the axis labels in degrees.
*/
/*!
    \qmlproperty real AbstractAxis::labelsAngle
    The angle of the axis labels in degrees.
*/

/*!
    \property QAbstractAxis::labelDelegate
    \brief A custom QML Component used as a label for the axis.
    This component should either be a \a Text component or contain "property string text",
    so that this property will be assigned the value of the label.
*/
/*!
    \qmlproperty Component AbstractAxis::labelDelegate
    A custom QML Component used as a label for the axis.
    This component should either be a \a Text component or contain "property string text",
    so that this property will be assigned the value of the label.
*/

/*!
    \property QAbstractAxis::gridVisible
    \brief The visibility of the grid lines.
    By default, the value is \c true.
*/
/*!
    \qmlproperty bool AbstractAxis::gridVisible
    The visibility of the grid lines. By default, the value is \c true.
*/

/*!
    \property QAbstractAxis::subGridVisible
    \brief The visibility of the subgrid lines.

    Applies only to axes that support subgrid lines.
    By default, the value is \c true.
*/
/*!
    \qmlproperty bool AbstractAxis::subGridVisible
    The visibility of the subgrid lines. Applies only to axes that support
    subgrid lines. By default, the value is \c true.
*/

/*!
    \property QAbstractAxis::titleText
    \brief The title of the axis.

    Empty by default. Axis titles support HTML formatting.
*/
/*!
    \qmlproperty string AbstractAxis::titleText
    The title of the axis. Empty by default. Axis titles support HTML formatting.
*/

/*!
    \property QAbstractAxis::titleColor
    \brief The color used to draw the title text.
*/
/*!
    \qmlproperty color AbstractAxis::titleColor
    The color used to draw the title text.
*/

/*!
    \property QAbstractAxis::titleVisible
    \brief The visibility of the axis title.

    By default, the value is \c true.
*/
/*!
    \qmlproperty bool AbstractAxis::titleVisible
    The visibility of the axis title. By default, the value is \c true.
*/

/*!
    \property QAbstractAxis::titleFont
    \brief The font of the title of the axis.
*/
/*!
    \qmlproperty font AbstractAxis::titleFont
    The font of the title of the axis.
*/


/*!
    \property QAbstractAxis::alignment
    \since 6.9
    \brief The alignment of the axis.

    Can be Qt::AlignLeft, Qt::AlignRight, Qt::AlignBottom, or Qt::AlignTop.
*/
/*!
    \qmlproperty alignment AbstractAxis::alignment
    \since 6.9
    The alignment of the axis. Can be \l{Qt::AlignLeft}{Qt.AlignLeft},
    \l{Qt::AlignRight}{Qt.AlignRight}, \l{Qt::AlignBottom}{Qt.AlignBottom}, or
    \l{Qt::AlignTop}{Qt.AlignTop}.
*/

/*!
    \property QAbstractAxis::textElideMode
    \since 6.10
    \brief Property specifies ellipsis placement for overflowing text.

    Can be Qt::ElideLeft, Qt::ElideRight, Qt::ElideMiddle, or Qt::ElideNone.
    By default, the value is \c Qt::ElideNone.
*/
/*!
    \qmlproperty enumeration AbstractAxis::textElideMode
    \since 6.10
    This enum specifies ellipsis placement for overflowing text.
    Can be \l{Qt::ElideLeft}{Qt.ElideLeft}, \l{Qt::ElideRight}{Qt.ElideRight},
    \l{Qt::ElideMiddle}{Qt.ElideMiddle}, or \l{Qt::ElideNone}{Qt.ElideNone}.
    By default, the value is \c Qt.ElideNone.
*/

/*!
    \property QAbstractAxis::color
    \brief The color used to draw the main ticks and the axis line.

    It overrides the theme color even if the theme changes.
*/
/*!
    \qmlproperty color AbstractAxis::color
    The color used to draw the main ticks and the axis line.
    It overrides the theme color even if the theme changes.
*/

/*!
    \property QAbstractAxis::subColor
    \brief The color used to draw the sub ticks.

    It overrides the theme color even if the theme changes.
*/
/*!
    \qmlproperty color AbstractAxis::subColor
    The color used to draw the sub ticks.
    It overrides the theme color even if the theme changes.
*/

/*!
    \fn void QAbstractAxis::update()
    This signal is emitted when the axis needs to be updated.
*/

/*!
    \qmlsignal AbstractAxis::visibleChanged(bool visible)
    This signal is emitted when the visibility of the axis changes to \a visible.
*/

/*!
    \qmlsignal AbstractAxis::lineVisibleChanged(bool visible)
    This signal is emitted when the visibility of the axis line changes to \a visible.
*/

/*!
    \qmlsignal AbstractAxis::labelsVisibleChanged(bool visible)
    This signal is emitted when the visibility of the labels of the axis changes to \a visible.
*/

/*!
    \qmlsignal AbstractAxis::labelsAngleChanged(string angle)
    This signal is emitted when the angle of the axis labels changes to \a angle.
*/

/*!
    \qmlsignal AbstractAxis::gridVisibleChanged(bool visible)
    This signal is emitted when the visibility of the grid lines of the axis changes to \a visible.
*/

/*!
    \qmlsignal AbstractAxis::minorGridVisibleChanged(bool visible)
    This signal is emitted when the visibility of the minor grid lines of the axis
    changes to \a visible.
*/

/*!
    \qmlsignal AbstractAxis::titleTextChanged(string text)
    This signal is emitted when the text of the axis title changes to \a text.
*/

/*!
    \qmlsignal AbstractAxis::titleColorChanged(color color)
    This signal is emitted when the color used to draw the axis title changes to \a color.
*/

/*!
    \qmlsignal AbstractAxis::titleVisibleChanged(bool visible)
    This signal is emitted when the visibility of the title text of the axis changes to \a visible.
*/

/*!
    \qmlsignal AbstractAxis::titleFontChanged(Font font)
    This signal is emitted when the font of the axis title changes to \a font.
*/

/*!
    \qmlsignal AbstractAxis::rangeChanged(string min, string max)
    This signal is emitted when the axis range changes. \a min and \a max are
    the min and max of the new range.
*/

/*!
    \qmlsignal AbstractAxis::textElideModeChanged(enumeration elideMode)
    This signal is emitted when the \l textElideMode changes. \a elideMode is the
    new value of the \l Qt::TextElideMode type.
*/

/*!
    \qmlsignal AbstractAxis::colorChanged(color color)
    This signal is emitted when the color used to draw the axis main ticks and line changes to
    \a color.
*/

/*!
    \qmlsignal AbstractAxis::subColorChanged(color color)
    This signal is emitted when the color used to draw the axis sub ticks changes to \a color.
*/

/*!
    \internal
    Constructs a new axis object that is a child of \a parent. The ownership is taken by
    graph when the axis is added.
*/

Q_LOGGING_CATEGORY(lcAxis2D, "qt.graphs2d.axis.properties")

QAbstractAxis::QAbstractAxis(QAbstractAxisPrivate &dd, QObject *parent)
    : QObject(dd, parent)
{}

/*!
  Destructs the axis object. When the axis is added to a graph, the graph object takes ownership.
*/

QAbstractAxis::~QAbstractAxis()
{
}

/*!
  Determines whether the axis line and tick marks are \a visible.
 */
void QAbstractAxis::setLineVisible(bool visible)
{
    Q_D(QAbstractAxis);
    if (d->m_lineVisible != visible) {
        d->m_lineVisible = visible;
        emit update();
        emit lineVisibleChanged(visible);
    } else {
        qCDebug(lcAxis2D) << "QAbstractAxis::setLineVisible. Visibility is already set to:"
                          << visible;
    }
}

bool QAbstractAxis::isLineVisible() const
{
    Q_D(const QAbstractAxis);
    return d->m_lineVisible;
}

void QAbstractAxis::setGridVisible(bool visible)
{
    Q_D(QAbstractAxis);
    if (d->m_gridVisible != visible) {
        d->m_gridVisible = visible;
        emit update();
        emit gridVisibleChanged(visible);
    } else {
        qCDebug(lcAxis2D) << "QAbstractAxis::setGridVisible. Visibility is already set to:"
                          << visible;
    }
}

bool QAbstractAxis::isGridVisible() const
{
    Q_D(const QAbstractAxis);
    return d->m_gridVisible;
}

void QAbstractAxis::setSubGridVisible(bool visible)
{
    Q_D(QAbstractAxis);
    if (d->m_subGridVisible != visible) {
        d->m_subGridVisible = visible;
        emit update();
        emit subGridVisibleChanged(visible);
    } else {
        qCDebug(lcAxis2D) << "QAbstractAxis::setSubGridVisible. Visibility is already set to:"
                          << visible;
    }
}

bool QAbstractAxis::isSubGridVisible() const
{
    Q_D(const QAbstractAxis);
    return d->m_subGridVisible;
}

void QAbstractAxis::setLabelsVisible(bool visible)
{
    Q_D(QAbstractAxis);
    if (d->m_labelsVisible != visible) {
        d->m_labelsVisible = visible;
        emit update();
        emit labelsVisibleChanged(visible);
    } else {
        qCDebug(lcAxis2D) << "QAbstractAxis::setLabelsVisible. Visibility is already set to:"
                          << visible;
    }
}

bool QAbstractAxis::labelsVisible() const
{
    Q_D(const QAbstractAxis);
    return d->m_labelsVisible;
}

void QAbstractAxis::setLabelsAngle(qreal angle)
{
    Q_D(QAbstractAxis);
    if (d->m_labelsAngle != angle) {
        d->m_labelsAngle = angle;
        emit update();
        emit labelsAngleChanged(angle);
    } else {
        qCDebug(lcAxis2D, "QAbstractAxis::setLabelsAngle. Angle is already set to: %f",
                angle);
    }
}

qreal QAbstractAxis::labelsAngle() const
{
    Q_D(const QAbstractAxis);
    return d->m_labelsAngle;
}

QQmlComponent *QAbstractAxis::labelDelegate() const
{
    Q_D(const QAbstractAxis);
    return d->m_labelDelegate;
}

void QAbstractAxis::setLabelDelegate(QQmlComponent *newLabelDelegate)
{
    Q_D(QAbstractAxis);
    if (d->m_labelDelegate == newLabelDelegate) {
        qCDebug(lcAxis2D) << "QAbstractAxis::setLabelDelegate. LabelDelegate is already set to:"
                          << newLabelDelegate;
        return;
    }

    d->m_labelDelegate = newLabelDelegate;
    emit labelDelegateChanged();
    emit update();
}

void QAbstractAxis::setTitleVisible(bool visible)
{
    Q_D(QAbstractAxis);
    if (d->m_titleVisible != visible) {
        d->m_titleVisible = visible;
        emit update();
        emit titleVisibleChanged(visible);
    } else {
        qCDebug(lcAxis2D) << "QAbstractAxis::setTitleVisible. visibility is already set to:"
                          << visible;
    }
}

bool QAbstractAxis::isTitleVisible() const
{
    Q_D(const QAbstractAxis);
    return d->m_titleVisible;
}

/*!
  Sets the color used to draw titles to \a color.
 */
void QAbstractAxis::setTitleColor(QColor color)
{
    Q_D(QAbstractAxis);
    if (!color.isValid())
        qCWarning(lcAxis2D) << "QAbstractAxis::setTitleColor. Tried to use invalid color.";

    if (d->m_titleColor != color) {
        d->m_titleColor = color;
        emit update();
        emit titleColorChanged(color);
    } else {
        qCDebug(lcAxis2D) << "QAbstractAxis::setTitleColor. Color is already set to:"
                          << color;
    }
}

/*!
  Returns the color used to draw titles.
*/
QColor QAbstractAxis::titleColor() const
{
    Q_D(const QAbstractAxis);
    return d->m_titleColor;
}

/*!
  Sets the font used to draw titles to \a font.
*/
void QAbstractAxis::setTitleFont(const QFont &font)
{
    Q_D(QAbstractAxis);
    if (d->m_titleFont != font) {
        d->m_titleFont = font;
        emit update();
        emit titleFontChanged(font);
    } else {
        qCDebug(lcAxis2D) << "QAbstractAxis::setTitleFont. Font is already set to:"
                          << font;
    }
}

/*!
  Returns the font used to draw titles.
*/
QFont QAbstractAxis::titleFont() const
{
    Q_D(const QAbstractAxis);
    return d->m_titleFont;
}

void QAbstractAxis::setTitleText(const QString &title)
{
    Q_D(QAbstractAxis);
    if (d->m_title != title) {
        d->m_title = title;
        emit update();
        emit titleTextChanged(title);
    } else {
        qCDebug(lcAxis2D, "QAbstractAxis::setTitleText. Title is already set to: %s",
                qPrintable(title));
    }
}

QString QAbstractAxis::titleText() const
{
    Q_D(const QAbstractAxis);
    return d->m_title;
}

bool QAbstractAxis::isVisible() const
{
    Q_D(const QAbstractAxis);
    return d->m_visible;
}

/*!
  Sets the visibility of the axis, labels, and grid lines to \a visible.
*/
void QAbstractAxis::setVisible(bool visible)
{
    Q_D(QAbstractAxis);
    if (d->m_visible != visible) {
        d->m_visible = visible;
        emit update();
        emit visibleChanged(visible);
    } else {
        qCDebug(lcAxis2D) << "QAbstractAxis::setVisible. axis, labels and grid visibility is already set to:"
                          << visible;
    }
}


/*!
  Makes the axis, labels, and grid lines visible.
*/
void QAbstractAxis::show()
{
    setVisible(true);
}

/*!
  Makes the axis, labels, and grid lines invisible.
*/
void QAbstractAxis::hide()
{
    setVisible(false);
}

/*!
  Sets the minimum value shown on the axis.
  Depending on the actual axis type, the \a min parameter is converted to the appropriate type
  of value. If the conversion is impossible, the function call does nothing.
*/
void QAbstractAxis::setMin(const QVariant &min)
{
    Q_D(QAbstractAxis);
    d->setMin(min);
}

/*!
  Sets the maximum value shown on the axis.
  Depending on the actual axis type, the \a max parameter is converted to the appropriate type
  of value. If the conversion is impossible, the function call does nothing.
*/
void QAbstractAxis::setMax(const QVariant &max)
{
    Q_D(QAbstractAxis);
    d->setMax(max);
}

/*!
  Sets the range shown on the axis.
  Depending on the actual axis type, the \a min and \a max parameters are converted to
  appropriate types of values. If the conversion is impossible, the function call does nothing.
*/
void QAbstractAxis::setRange(const QVariant &min, const QVariant &max)
{
    Q_D(QAbstractAxis);
    d->setRange(min, max);
}

Qt::Alignment QAbstractAxis::alignment() const
{
    const Q_D(QAbstractAxis);
    return d->m_alignment;
}

void QAbstractAxis::setAlignment(Qt::Alignment alignment)
{
    Q_D(QAbstractAxis);
    if (d->m_alignment == alignment) {
        qCDebug(lcAxis2D) << "QAbstractAxis::setAlignment. Alignment is already set to:"
                          << alignment;
        return;
    }
    switch (alignment) {
    case Qt::AlignTop:
    case Qt::AlignBottom:
    case Qt::AlignLeft:
    case Qt::AlignRight:
        d->m_alignment = alignment;
        emit update();
        emit alignmentChanged(alignment);
        break;
    default:
        qCWarning(lcAxis2D, "invalid alignment.");
        break;
    }
}

Qt::TextElideMode QAbstractAxis::textElideMode() const
{
    Q_D(const QAbstractAxis);
    return d->m_textElide;
}

void QAbstractAxis::setTextElideMode(Qt::TextElideMode elide)
{
    Q_D(QAbstractAxis);
    if (d->m_textElide == elide) {
        qCDebug(lcAxis2D) << "QAbstractAxis::setTextElide. Text elide is already set to:"
            << elide;
        return;
    }

    d->m_textElide = elide;
    Q_EMIT textElideModeChanged(d->m_textElide);
}

void QAbstractAxis::setSubColor(QColor color)
{
    Q_D(QAbstractAxis);
    if (!color.isValid())
        qCWarning(lcAxis2D) << "QAbstractAxis::setSubColor. Tried to use invalid color.";

    if (d->m_subColor != color) {
        d->m_subColor = color;
        emit update();
        emit subColorChanged(color);
    } else {
        qCDebug(lcAxis2D) << "QAbstractAxis::setSubColor. Color is already set to:" << color;
    }
}

QColor QAbstractAxis::subColor() const
{
    Q_D(const QAbstractAxis);
    return d->m_subColor;
}

void QAbstractAxis::setColor(QColor color)
{
    Q_D(QAbstractAxis);
    if (!color.isValid())
        qCWarning(lcAxis2D) << "QAbstractAxis::setColor. Tried to use invalid color.";

    if (d->m_color != color) {
        d->m_color = color;
        emit update();
        emit colorChanged(color);
    } else {
        qCDebug(lcAxis2D) << "QAbstractAxis::setColor. Color is already set to:" << color;
    }
}

QColor QAbstractAxis::color() const
{
    Q_D(const QAbstractAxis);
    return d->m_color;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QAbstractAxisPrivate::QAbstractAxisPrivate() {}

QAbstractAxisPrivate::~QAbstractAxisPrivate()
{
    Q_Q(QAbstractAxis);
    if (m_graph)
        m_graph->removeAxis(q, true);
}

void QAbstractAxisPrivate::setGraph(QGraphsView *graph)
{
    if (m_graph && graph)
        qCWarning(lcAxis2D, "%p axis already associated with %p", this, m_graph);

    m_graph = graph;
}

void QAbstractAxisPrivate::handleRangeChanged(qreal min, qreal max)
{
    setRange(min,max);
}

QT_END_NAMESPACE

#include "moc_qabstractaxis.cpp"
