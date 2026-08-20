// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "graphs2d/qabstractseries_p.h"
#include <QtGraphs/qareaseries.h>
#include <private/qareaseries_p.h>
#include <private/qgraphsview_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QAreaSeries
    \inmodule QtGraphs
    \ingroup graphs_2D
    \brief The QAreaSeries class presents data in area graphs.

    An area graph is used to draw an area composed by points.
    The points are defined by two series: upperSeries and lowerSeries.
    The area between the series is drawn as a graph. If only the upperSeries
    is defined, the area is then between the bottom of the graph and the upper series.
*/
/*!
    \qmltype AreaSeries
    \nativetype QAreaSeries
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D
    \inherits AbstractSeries

    \brief Presents data in area graphs.

    An area graph is used to draw an area composed by points.
    The points are defined by two series: upperSeries and lowerSeries.
    The area between the series is drawn as a graph. If only the upperSeries
    is defined, the area is then between the bottom of the graph and the upper series.

    \image graphs2d-area.png
*/

/*!
    \property QAreaSeries::color
    \brief The fill color of the area.
    The default value is \c Qt::transparent, meaning the color is defined by the theme.
    \note If \l gradient is set to something other than \c null, these
    will take precedence over \c color. The \c color will be ignored in this case.
*/
/*!
    \qmlproperty color AreaSeries::color
    The fill color of the area.
    The default value is \c transparent, meaning the color is defined by the theme.
    \note If \l gradient is set to something other than \c null, these
    will take precedence over \c color. The \c color will be ignored in this case.
*/

/*!
    \property QAreaSeries::selectedColor
    \brief The fill color of the area when selected.
    The default value is \c Qt::transparent, meaning the selected color is defined by the theme.
    \note If \l selectedGradient is set to something other than \c null, these
    will take precedence over \c selectedColor. The \c selectedColor will be ignored in this case.
*/
/*!
    \qmlproperty color AreaSeries::selectedColor
    The fill color of the area when selected.
    The default value is \c transparent, meaning the selected color is defined by the theme.
    \note If \l selectedGradient is set to something other than \c null, these
    will take precedence over \c selectedColor. The \c selectedColor will be ignored in this case.
*/

/*!
    \property QAreaSeries::borderColor
    \brief The border color of the area.
    The default value is \c Qt::transparent, meaning the border color is defined by the theme.
*/
/*!
    \qmlproperty color AreaSeries::borderColor
    The border color of the area.
    The default value is \c transparent, meaning the border color is defined by the theme.
*/

/*!
    \property QAreaSeries::selectedBorderColor
    \brief The border color of the area when selected.
    The default value is \c Qt::transparent, meaning the selected border color is defined by the theme.
*/
/*!
    \qmlproperty color AreaSeries::selectedBorderColor
    The border color of the area when selected.
    The default value is \c transparent, meaning the selected border color is defined by the theme.
*/

/*!
    \property QAreaSeries::borderWidth
    \brief The width of the line that encloses the area.
    The default value is \c -1, meaning the border width is defined by the theme.
*/
/*!
    \qmlproperty real AreaSeries::borderWidth
    The width of the line that encloses the area.
    The default value is \c -1, meaning the border width is defined by the theme.
*/

/*!
    \property QAreaSeries::selected
    \brief Sets this area as selected.
    The default value is \c false.
*/
/*!
    \qmlproperty bool AreaSeries::selected
    Sets this area as selected.
    The default value is \c false.
*/

/*!
    \property QAreaSeries::upperSeries
    \brief Sets the upper boundary of the area. No area is drawn if this is null.
*/
/*!
    \qmlproperty XYSeries AreaSeries::upperSeries
    Sets the upper boundary of the area. No area is drawn if this is null.
*/

/*!
    \property QAreaSeries::lowerSeries
    \brief Sets the lower boundary of the area. If this is null, the graph bottom
    is considered the lower bound.
*/
/*!
    \qmlproperty XYSeries AreaSeries::lowerSeries
    Sets the lower boundary of the area. If this is null, the graph bottom
    is considered the lower bound.
*/

/*!
    \property QAreaSeries::gradient
    \since 6.11
    This property defines the fill gradient. By default no gradient is enabled
    and the value is \c null. In this case the fill will be based on the \l{color} property.

    \note The Gradient type cannot be used here. Rather, prefer using one of
    the advanced subtypes, like LinearGradient.

    \note If set to something other than \c{null}, the \c gradient will take precedence over
    \l color.
*/
/*!
    \qmlproperty ShapeGradient AreaSeries::gradient
    \since 6.11
    This property defines the fill gradient. By default no gradient is enabled
    and the value is \c null. In this case the fill will be based on the \l{color} property.

    \note The Gradient type cannot be used here. Rather, prefer using one of
    the advanced subtypes, like LinearGradient.

    \note If set to something other than \c{null}, the \c gradient will take precedence over
    \l color.
*/

/*!
    \property QAreaSeries::selectedGradient
    \since 6.11
    This property defines the fill gradient of the area when selected. By default no gradient is enabled
    and the value is \c null. In this case the fill will be based on the \l{selectedColor} property.

    \note The Gradient type cannot be used here. Rather, prefer using one of
    the advanced subtypes, like LinearGradient.

    \note If set to something other than \c{null}, the \c selectedGradient will take precedence over
    \l selectedColor.
*/
/*!
    \qmlproperty ShapeGradient AreaSeries::selectedGradient
    \since 6.11
    This property defines the fill gradient of the area when selected. By default no gradient is enabled
    and the value is \c null. In this case the fill will be based on the \l{selectedColor} property.

    \note The Gradient type cannot be used here. Rather, prefer using one of
    the advanced subtypes, like LinearGradient.

    \note If set to something other than \c{null}, the \c selectedGradient will take precedence over
    \l selectedColor.
*/

/*!
    \qmlsignal AreaSeries::colorChanged(color newColor)
    This signal is emitted when the area \l color changes to \a newColor.
*/

/*!
    \qmlsignal AreaSeries::selectedColorChanged(color newSelectedColor)
    This signal is emitted when the color of a selected area changes to
    \a newSelectedColor.
*/

/*!
    \qmlsignal AreaSeries::borderColorChanged(color newBorderColor)
    This signal is emitted when the area border color changes to
    \a newBorderColor.
*/

/*!
    \qmlsignal AreaSeries::selectedBorderColorChanged(color newSelectedBorderColor);
    This signal is emitted when the border color of a selected area changes to
    \a newSelectedBorderColor.
*/

/*!
    \qmlsignal AreaSeries::borderWidthChanged();
    This signal is emitted when the width of the area border width changes.
*/

/*!
    \qmlsignal AreaSeries::selectedChanged();
    This signal is emitted when the current area is selected.
*/

/*!
    \qmlsignal AreaSeries::upperSeriesChanged();
    This signal is emitted when the upper series changes.
*/

/*!
    \qmlsignal AreaSeries::lowerSeriesChanged();
    This signal is emitted when the lower series changes.
*/

/*!
    \qmlsignal AreaSeries::gradientChanged();
    \since 6.11
    This signal is emitted when the gradient changes.
*/

/*!
    \qmlsignal AreaSeries::selectedGradientChanged();
    \since 6.11
    This signal is emitted when the gradient of a selected area changes.
*/

/*!
    \qmlsignal AreaSeries::clicked(point point)
    This signal is emitted when the user clicks or taps an area graph.
    The \a point specifies the event triggered position.
*/

/*!
    \qmlsignal AreaSeries::doubleClicked(point point)
    This signal is emitted when the user double-clicks or double-taps an area graph.
    The \a point specifies the event triggered position.
    This signal always occurs after \l clicked.
*/

/*!
    \qmlsignal AreaSeries::pressed(point point)
    This signal is emitted when the user clicks or taps the area graph and holds down
    the mouse button or gesture.
    The \a point specifies the event triggered position.
*/

/*!
    \qmlsignal AreaSeries::released(point point)
    This signal is emitted when the user releases a pressed click or tap.
    The \a point specifies the event triggered position.
*/

QAreaSeries::QAreaSeries(QObject *parent)
    : QAbstractSeries(*(new QAreaSeriesPrivate()), parent)
{}

QAreaSeries::~QAreaSeries() {}

QAreaSeries::QAreaSeries(QAreaSeriesPrivate &dd, QObject *parent)
    : QAbstractSeries(dd, parent)
{}

QAbstractSeries::SeriesType QAreaSeries::type() const
{
    return QAbstractSeries::SeriesType::Area;
}

QColor QAreaSeries::color() const
{
    Q_D(const QAreaSeries);
    return d->m_color;
}

void QAreaSeries::setColor(QColor newColor)
{
    Q_D(QAreaSeries);
    if (color() == newColor) {
        qCDebug(lcProperties2D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)), qUtf8Printable(newColor.name()));
        return;
    }
    d->m_color = newColor;
    emit colorChanged(newColor);
    emit update();
}

QColor QAreaSeries::selectedColor() const
{
    Q_D(const QAreaSeries);
    return d->m_selectedColor;
}

void QAreaSeries::setSelectedColor(QColor newSelectedColor)
{
    Q_D(QAreaSeries);
    if (selectedColor() == newSelectedColor) {
        qCDebug(lcProperties2D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)),
                qUtf8Printable(newSelectedColor.name()));
        return;
    }
    d->m_selectedColor = newSelectedColor;
    emit selectedColorChanged(newSelectedColor);
    emit update();
}

QColor QAreaSeries::borderColor() const
{
    Q_D(const QAreaSeries);
    return d->m_borderColor;
}

void QAreaSeries::setBorderColor(QColor newBorderColor)
{
    Q_D(QAreaSeries);
    if (d->m_borderColor == newBorderColor) {
        qCDebug(lcProperties2D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)),
                qUtf8Printable(newBorderColor.name()));
        return;
    }
    d->m_borderColor = newBorderColor;
    emit borderColorChanged(newBorderColor);
    emit update();
}

QColor QAreaSeries::selectedBorderColor() const
{
    Q_D(const QAreaSeries);
    return d->m_selectedBorderColor;
}

void QAreaSeries::setSelectedBorderColor(QColor newSelectedBorderColor)
{
    Q_D(QAreaSeries);
    if (d->m_selectedBorderColor == newSelectedBorderColor) {
        qCDebug(lcProperties2D, "%s value is already set to: %s",
                qUtf8Printable(QLatin1String(__FUNCTION__)),
                qUtf8Printable(newSelectedBorderColor.name()));
        return;
    }
    d->m_selectedBorderColor = newSelectedBorderColor;
    emit selectedBorderColorChanged(newSelectedBorderColor);
    emit update();
}

qreal QAreaSeries::borderWidth() const
{
    Q_D(const QAreaSeries);
    return d->m_borderWidth;
}

void QAreaSeries::setBorderWidth(qreal newBorderWidth)
{
    Q_D(QAreaSeries);
    if (QtPrivate::fuzzyCompare(d->m_borderWidth, newBorderWidth)) {
        qCDebug(lcProperties2D, "%s value is already set to: %.1f",
                qUtf8Printable(QLatin1String(__FUNCTION__)), newBorderWidth);
        return;
    }
    d->m_borderWidth = newBorderWidth;
    emit borderWidthChanged();
    emit update();
}

bool QAreaSeries::isSelected() const
{
    Q_D(const QAreaSeries);
    return d->m_selected;
}

void QAreaSeries::setSelected(bool newSelected)
{
    Q_D(QAreaSeries);
    if (d->m_selected == newSelected) {
        qCDebug(lcProperties2D) << __FUNCTION__
            << "value is already set to:" << newSelected;
        return;
    }
    d->m_selected = newSelected;
    emit selectedChanged();
}

QXYSeries *QAreaSeries::upperSeries() const
{
    Q_D(const QAreaSeries);
    return d->m_upperSeries;
}

void QAreaSeries::setUpperSeries(QXYSeries *newUpperSeries)
{
    Q_D(QAreaSeries);
    if (d->m_upperSeries == newUpperSeries) {
        qCDebug(lcProperties2D) << __FUNCTION__
            << "value is already set to:" << newUpperSeries;
        return;
    }

    if (d->m_upperSeries)
        disconnect(newUpperSeries, &QXYSeries::update, this, &QAreaSeries::update);

    d->m_upperSeries = newUpperSeries;

    connect(newUpperSeries, &QXYSeries::update, this, &QAreaSeries::update);

    emit upperSeriesChanged();
    emit update();
}

QXYSeries *QAreaSeries::lowerSeries() const
{
    Q_D(const QAreaSeries);
    return d->m_lowerSeries;
}

void QAreaSeries::setLowerSeries(QXYSeries *newLowerSeries)
{
    Q_D(QAreaSeries);
    if (d->m_lowerSeries == newLowerSeries) {
        qCDebug(lcProperties2D) << __FUNCTION__
            << "value is already set to:" << newLowerSeries;
        return;
    }

    if (d->m_lowerSeries)
        disconnect(newLowerSeries, &QXYSeries::update, this, &QAreaSeries::update);

    d->m_lowerSeries = newLowerSeries;

    connect(newLowerSeries, &QXYSeries::update, this, &QAreaSeries::update);

    emit lowerSeriesChanged();
    emit update();
}

QQuickShapeGradient* QAreaSeries::gradient() const
{
    Q_D(const QAreaSeries);
    return d->m_gradient;
}

void QAreaSeries::setGradient(QQuickShapeGradient* newGradient)
{
    Q_D(QAreaSeries);
    if (d->m_gradient == newGradient) {
        qCDebug(lcProperties2D) << __FUNCTION__
            << "value is already set to:" << newGradient;
        return;
    }
    d->m_gradient = newGradient;
    emit gradientChanged(newGradient);
    emit update();
}

QQuickShapeGradient* QAreaSeries::selectedGradient() const
{
    Q_D(const QAreaSeries);
    return d->m_selectedGradient;
}

void QAreaSeries::setSelectedGradient(QQuickShapeGradient* newSelectedGradient)
{
    Q_D(QAreaSeries);
    if (d->m_selectedGradient == newSelectedGradient) {
        qCDebug(lcProperties2D) << __FUNCTION__
            << "value is already set to:" << newSelectedGradient;
        return;
    }
    d->m_selectedGradient = newSelectedGradient;
    emit selectedGradientChanged(newSelectedGradient);
    emit update();
}

QAreaSeriesPrivate::QAreaSeriesPrivate()
    : QAbstractSeriesPrivate(QAbstractSeries::SeriesType::Area)
{}

QT_END_NAMESPACE

#include "moc_qareaseries.cpp"
