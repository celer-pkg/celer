// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtGraphs/qpieseries.h>
#include <QtQuick/private/qquicktext_p.h>
#include <QtQuickShapes/private/qquickshape_p.h>
#include <private/qpieslice_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QPieSlice
    \inmodule QtGraphs
    \ingroup graphs_2D
    \brief The QPieSlice class represents a single slice in a pie series.

    A pie slice has a value and a label. When the slice is added to a pie series, the
    QPieSeries object calculates the percentage of the slice compared with the sum of
    all slices in the series to determine the actual size of the slice in the graph.

    By default, the label is hidden. If it is visible, it can be either located outside
    the slice and connected to it with an arm or centered inside the slice either
    horizontally or in parallel with the tangential or normal of the slice's arc.

    By default, the visual appearance of the slice is set by a theme, but the theme can be
    overridden by specifying slice properties. However, if the theme is changed after the
    slices are customized, all customization will be lost.

    To enable user interaction with the pie graph, some basic signals are emitted when
    users click pie slices or hover the mouse over them.

    A pie slice can contain sub slices.

    \sa QPieSeries
*/

/*!
    \qmltype PieSlice
    \nativetype QPieSlice
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D
    \brief Represents a single slice in a pie series.

    A pie slice has a value and a label. When the slice is added to a pie series, the
    PieSeries type calculates the percentage of the slice compared with the sum of
    all slices in the series to determine the actual size of the slice in the graph.

    By default, the label is hidden. If it is visible, it can be either located outside
    the slice and connected to it with an arm or centered inside the slice either
    horizontally or in parallel with the tangential or normal of the slice's arc.

    By default, the visual appearance of the slice is set by a theme, but the theme can be
    overridden by specifying slice properties. However, if the theme is changed after the
    slices are customized, all customization will be lost.

    The PieSlice type should be used as a child of a PieSeries type. For example:

    \code
    PieSeries {
        PieSlice {
            label: "example"
            value: 1
        }
    }
    \endcode

    Alternatively, slices can be added to a pie series by using the \l {QtGraphs::PieSeries::append()}
    {PieSeries.append()} method.

    In that case, \l {QtGraphs::PieSeries::at()}{PieSeries.at()} or \l {QtGraphs::PieSeries::find()}
    {PieSeries.find()} can be used to access the properties of an individual PieSlice instance.

    A pie slice can contain sub slices.

    \qml
        PieSeries {
            PieSlice {
                label: "example"
                value: 1
                color: "orange"
                exploded: true
                PieSlice {
                    label: "example2"
                    value: 2
                    color: "red"
                }
                PieSlice {
                    label: "example3"
                    value: 3
                    color: "yellow"
                    exploded: true
                }
            }
            PieSlice {
                label: "example4"
                value: 4
                color: "green"
            }
        }
    \endqml

    \image graphs2d-subpieslice.png

    \sa PieSeries
*/

/*!
    \enum QPieSlice::LabelPosition

    This enum describes the position of the slice label.

    \value Outside
         The label is located outside the slice connected to it with an arm.
         This is the default value.
    \value InsideHorizontal
         The label is centered within the slice and laid out horizontally.
    \value InsideTangential
         The label is centered within the slice and rotated to be parallel with
         the tangential of the slice's arc.
    \value InsideNormal
         The label is centered within the slice and rotated to be parallel with
         the normal of the slice's arc.
*/

/*!
    \property QPieSlice::label
    \brief The label of the slice.
    \note The string can be HTML formatted.

    \sa labelVisible, labelFont, labelArmLengthFactor
*/
/*!
    \qmlproperty string PieSlice::label
    The label of the slice.
    \note The string can be HTML formatted.
*/

/*!
    \qmlsignal PieSlice::labelChanged()
    This signal is emitted when the slice label changes.
    \sa label
*/

/*!
    \property QPieSlice::value
    \brief The value of the slice.
    \note A negative value is converted to a positive value.
    \sa percentage(), QPieSeries::sum()
*/
/*!
    \qmlproperty real PieSlice::value
    The value of the slice.

    \note A negative value is converted to a positive value.
*/

/*!
    \qmlsignal PieSlice::valueChanged()
    This signal is emitted when the slice value changes.
    \sa value
*/

/*!
    \property QPieSlice::labelVisible
    \brief The visibility of the slice label. By default, the label is not visible.
    \sa label, labelFont, labelArmLengthFactor
*/
/*!
    \qmlproperty bool PieSlice::labelVisible
    The visibility of the slice label. By default, the label is not visible.
*/

/*!
    \qmlsignal PieSlice::labelVisibleChanged()
    This signal is emitted when the visibility of the slice label changes.
    \sa labelVisible
*/

/*!
    \property QPieSlice::labelColor
    \brief The color used to draw the slice label.
*/
/*!
    \qmlproperty color PieSlice::labelColor
    The color used to draw the slice label.
*/

/*!
    \qmlsignal PieSlice::labelColorChanged()
    This signal is emitted when the slice label color changes.
    \sa labelColor
*/

/*!
    \property QPieSlice::labelFont
    \brief The font used for drawing the label text.
    \sa label, labelVisible, labelArmLengthFactor
*/

/*!
    \qmlsignal PieSlice::labelFontChanged()
    This signal is emitted when the label font of the slice changes.
    \sa labelFont
*/

/*!
    \qmlproperty font PieSlice::labelFont

    The font used for the slice label.

    For more information, see \l [QML]{font}.

    \sa labelVisible, labelPosition
*/
/*!
    \qmlsignal PieSlice::labelFontChanged()
    This signal is emitted when the label font changes.
    \sa labelFont
*/

/*!
    \property QPieSlice::labelPosition
    \brief The position of the slice label.
    \sa label, labelVisible
*/
/*!
    \qmlproperty enumeration PieSlice::labelPosition

    Describes the position of the slice label.

    \value PieSlice.LabelPosition.Outside
         The label is located outside the slice connected to it with an arm.
         This is the default value.
    \value PieSlice.LabelPosition.InsideHorizontal
         The label is centered within the slice and laid out horizontally.
    \value PieSlice.LabelPosition.InsideTangential
         The label is centered within the slice and rotated to be parallel with
         the tangential of the slice's arc.
    \value PieSlice.LabelPosition.InsideNormal
         The label is centered within the slice and rotated to be parallel with
         the normal of the slice's arc.

    \sa labelVisible
*/
/*!
    \qmlsignal PieSlice::labelPositionChanged()
    This signal is emitted when the label position changes.
    \sa labelPosition
*/

/*!
    \property QPieSlice::labelArmLengthFactor
    \brief The length of the label arm.
    The factor is relative to the pie radius. For example:
    \list
        \li 1.0 means that the length is the same as the radius.
        \li 0.5 means that the length is half of the radius.
    \endlist
    By default, the arm length is 0.15
    \sa label, labelVisible, labelFont
*/
/*!
    \qmlproperty real PieSlice::labelArmLengthFactor
    The length of the label arm.
    The factor is relative to the pie radius. For example:
    \list
        \li 1.0 means that the length is the same as the radius.
        \li 0.5 means that the length is half of the radius.
    \endlist
    By default, the arm length is 0.15

    \sa labelVisible
*/
/*!
    \qmlsignal PieSlice::labelArmLengthFactorChanged()
    This signal is emitted when the label arm length factor changes.
    \sa labelArmLengthFactor
*/

/*!
    \property QPieSlice::color
    \brief The fill color of the slice.
    This is a convenience property for modifying the slice fill color.
*/
/*!
    \qmlproperty color PieSlice::color
    The fill color of the slice.
*/

/*!
    \qmlsignal PieSlice::colorChanged()
    This signal is emitted when the slice color changes.
*/

/*!
    \property QPieSlice::borderColor
    \brief The color used to draw the slice border.
    This is a convenience property for modifying the slice.
    \sa borderWidth
*/
/*!
    \qmlproperty color PieSlice::borderColor
    The color used to draw the slice border.
    \sa borderWidth
*/

/*!
    \qmlsignal PieSlice::borderColorChanged()
    This signal is emitted when the slice border color changes.
    \sa borderColor
*/

/*!
    \property QPieSlice::borderWidth
    \brief The width of the slice border.
    This is a convenience property for modifying the slice border width.
    \sa borderColor
*/
/*!
    \qmlproperty real PieSlice::borderWidth
    The width of the slice border.
    This is a convenience property for modifying the slice border width.
    \sa borderColor
*/

/*!
    \qmlsignal PieSlice::borderWidthChanged()
    This signal is emitted when the slice border width changes.
    \sa borderWidth
*/

/*!
    \property QPieSlice::exploded
    \brief Whether the slice is separated from the pie.
    \sa explodeDistanceFactor
*/
/*!
    \qmlproperty bool PieSlice::exploded
    Whether the slice is separated from the pie.
    \sa explodeDistanceFactor
*/
/*!
    \qmlsignal PieSlice::explodedChanged()
    This signal is emitted when the exploded property changes.
    \sa exploded
*/

/*!
    \property QPieSlice::explodeDistanceFactor
    \brief Determines how far away from the pie the slice is exploded.
    \list
        \li 1.0 means that the distance is the same as the radius.
        \li 0.5 means that the distance is half of the radius.
    \endlist
    By default, the distance is 0.15
    \sa exploded
*/
/*!
    \qmlproperty real PieSlice::explodeDistanceFactor
    Determines how far away from the pie the slice is exploded.
    \list
        \li 1.0 means that the distance is the same as the radius.
        \li 0.5 means that the distance is half of the radius.
    \endlist
    By default, the distance is 0.15

    \sa exploded
*/
/*!
    \qmlsignal PieSlice::explodeDistanceFactorChanged()
    This signal is emitted when the explode distance factor changes.
    \sa explodeDistanceFactor
*/

/*!
    \property QPieSlice::percentage
    \brief The percentage of the slice compared to the sum of all slices in the series.
    The actual value ranges from 0.0 to 1.0.
    Updated automatically once the slice is added to the series.
    \sa value, QPieSeries::sum
*/
/*!
    \qmlproperty real PieSlice::percentage
    The percentage of the slice compared to the sum of all slices in the series.
    The actual value ranges from 0.0 to 1.0.
    Updated automatically once the slice is added to the series.
*/

/*!
    \qmlsignal PieSlice::percentageChanged()
    This signal is emitted when the percentage of the slice changes.
    \sa percentage
*/

/*!
    \property QPieSlice::startAngle
    \brief The starting angle of this slice in the series it belongs to.
    A full pie is 360 degrees, where 0 degrees is at 12 a'clock.
    Updated automatically once the slice is added to the series.
*/
/*!
    \qmlproperty real PieSlice::startAngle
    The starting angle of this slice in the series it belongs to.
    A full pie is 360 degrees, where 0 degrees is at 12 a'clock.
    Updated automatically once the slice is added to the series.
*/

/*!
    \qmlsignal PieSlice::startAngleChanged()
    This signal is emitted when the starting angle of the slice changes.
    \sa startAngle
*/

/*!
    \property QPieSlice::angleSpan
    \brief  The span of the slice in degrees.
    A full pie is 360 degrees, where 0 degrees is at 12 a'clock.
    Updated automatically once the slice is added to the series.
*/
/*!
    \qmlproperty real PieSlice::angleSpan
    The span of the slice in degrees.
    A full pie is 360 degrees, where 0 degrees is at 12 a'clock.
    Updated automatically once the slice is added to the series.
*/

/*!
    \qmlsignal PieSlice::angleSpanChanged()
    This signal is emitted when the angle span of the slice changes.
    \sa angleSpan
*/

/*!
    \property QPieSlice::subSlicesCount

    \brief The number of sub slices in the slice.
*/

/*!
    \qmlproperty int PieSlice::subSlicesCount

    The number of sub slices in the slice.
*/

/*!
    \qmlsignal PieSlice::subSlicesCountChanged(qsizetype count)
    This signal is emitted when the sub slices \a count changes.
    \sa subSlicesCount
*/

/*!
    \property QPieSlice::subSlicesSum

    \brief The sum of all sub slices.

    The slice keeps track of the sum of all the sub slices it holds.
*/

/*!
    \qmlproperty real PieSlice::subSlicesSum

    The sum of all sub slices.

    The slice keeps track of the sum of all the sub slices it holds.
*/

/*!
    \qmlsignal PieSlice::subSlicesSumChanged(qreal sum)
    This signal is emitted when the \a sum of all sub slices changes.
    \sa subSlicesSum
*/

/*!
    \fn void QPieSlice::subSlicesAdded(const QList<QPieSlice *> &slices)
    This signal is emitted when the sub slices specified by \a slices are added to the slice.
    \sa append()
*/

/*!
    \qmlsignal PieSlice::subSlicesAdded(list<PieSlice> slices)
    This signal is emitted when the sub slices specified by \a slices are added to the slice.
*/

/*!
    \fn void QPieSlice::subSlicesRemoved(const QList<QPieSlice *> &slices)
    This signal is emitted when the sub slices specified by \a slices are removed from the slice.
    \sa remove()
*/

/*!
    \qmlsignal PieSlice::subSlicesRemoved(list<PieSlice> slices)
    This signal is emitted when the sub slices specified by \a slices are removed from the slice.
*/

/*!
    \qmlmethod PieSlice PieSlice::at(int index)
    Returns the sub slice at the position specified by \a index. Returns null if the
    index is not valid.
*/

/*!
    \qmlmethod PieSlice PieSlice::find(string label)
    Returns the first sub slice that has the label \a label. Returns null if the label
    is not found.
*/

/*!
    \qmlmethod PieSlice PieSlice::append(string label, real value)
    Adds a new sub slice with the label \a label and the value \a value to the slice.
*/

/*!
    \qmlmethod bool PieSlice::remove(PieSlice slice)
    Removes the sub slice specified by \a slice from the slice. Returns \c true if the
    removal was successful, \c false otherwise.
*/

/*!
    \qmlmethod bool PieSlice::replace(int index, PieSlice slice)
    Replaces the sub slice specified by \a slice from the slice at \a index. Returns \c true if the
    replace was successful, \c false otherwise.
*/

/*!
    \qmlmethod void PieSlice::clear()
    Removes all sub slices from the slice.
*/

/*!
    \qmlmethod void PieSlice::removeMultiple(int index, int count)
    Removes a range of sub slices as specified by the \a index and \a count. The call
    traverses over all sub slices even if removal of one fails.
*/

/*!
    \qmlmethod bool PieSlice::remove(int index)
    Removes the sub slice specified by \a index from the slice. Returns \c true if the
    removal was successful, \c false otherwise.
*/

/*!
    \qmlmethod bool PieSlice::replace(PieSlice oldSlice, PieSlice newSlice)
    Replaces the sub slice specified by \a oldSlice with \a newSlice. Returns
    \c true if the replace was successful, \c false otherwise. \a oldSlice is
    destroyed if it was replaced successfully.
*/

/*!
    \qmlmethod bool PieSlice::replaceAll(list<PieSlice> slices)
    Completely replaces all current sub slices with \a slices. The size does not need
    to match. Returns false if any of the PieSlice in \a slices is invalid.
*/

/*!
    \qmlmethod bool PieSlice::take(PieSlice slice)
    Takes a single sub slice, specified by \a slice, from the slice. Does not delete
    the sub slice object. Returns \c true if successful.
*/

/*!
    Constructs an empty slice with the parent \a parent.
    \sa QPieSeries::append(), QPieSeries::insert()
*/
QPieSlice::QPieSlice(QObject *parent)
    : QObject(*(new QPieSlicePrivate), parent)
{

}

/*!
    Constructs an empty slice with the specified \a value, \a label, and \a parent.
    \sa QPieSeries::append(), QPieSeries::insert()
*/
QPieSlice::QPieSlice(const QString &label, qreal value, QObject *parent)
    : QObject(*(new QPieSlicePrivate), parent)
{
    setLabel(label);
    setValue(value);
}

/*!
    Removes the slice. The slice should not be removed if it has been added to a series.
*/
QPieSlice::~QPieSlice() {}

/*!
    \property QPieSlice::sliceChildren

    The list of sub slices contained within this slice. This is a default property that allows
    sub slices to be specified within a slice element in QML without explicitly using the children
    property name.
*/
QQmlListProperty<QPieSlice> QPieSlice::sliceChildren()
{
    return QQmlListProperty<QPieSlice>(
        this, this,
        [](QQmlListProperty<QPieSlice> *list, QPieSlice *element) {
            static_cast<QPieSlice *>(list->data)->append(element);
        },
        [](QQmlListProperty<QPieSlice> *list) -> qsizetype {
            return static_cast<QPieSlice *>(list->data)->subSlicesCount();
        },
        [](QQmlListProperty<QPieSlice> *list, qsizetype index) -> QPieSlice * {
            return static_cast<QPieSlice *>(list->data)->at(index);
        },
        [](QQmlListProperty<QPieSlice> *list) {
            static_cast<QPieSlice *>(list->data)->clear();
        },
        [](QQmlListProperty<QPieSlice> *list, qsizetype index, QPieSlice *slice) {
            static_cast<QPieSlice *>(list->data)->replace(index, slice);
        },
        [](QQmlListProperty<QPieSlice> *list) {
            const auto size = static_cast<QPieSlice *>(list->data)->subSlicesCount();
            static_cast<QPieSlice *>(list->data)->remove(size - 1);
        }
    );
}

/*!
    Returns the series that this slice belongs to.

    \sa QPieSeries::append()
*/
QPieSeries *QPieSlice::series() const
{
    Q_D(const QPieSlice);
    return d->m_series;
}

qreal QPieSlice::percentage() const
{
    Q_D(const QPieSlice);
    return d->m_percentage;
}

qreal QPieSlice::startAngle() const
{
    Q_D(const QPieSlice);
    return d->m_startAngle;
}

qreal QPieSlice::angleSpan() const
{
    Q_D(const QPieSlice);
    return d->m_angleSpan;
}

void QPieSlice::setLabel(const QString &label)
{
    Q_D(QPieSlice);
    if (d->m_labelText == label)
        return;
    d->m_labelText = label;
    d->m_labelItem->setText(label);
    emit labelChanged();
}

QString QPieSlice::label() const
{
    Q_D(const QPieSlice);
    return d->m_labelText;
}

void QPieSlice::setLabelVisible(bool visible)
{
    Q_D(QPieSlice);
    if (d->m_isLabelVisible == visible)
        return;

    d->setLabelVisible(visible);
    emit labelVisibleChanged();
}

bool QPieSlice::isLabelVisible() const
{
    Q_D(const QPieSlice);
    return d->m_isLabelVisible;
}

void QPieSlice::setLabelPosition(LabelPosition position)
{
    Q_D(QPieSlice);
    if (d->m_labelPosition == position)
        return;

    d->setLabelPosition(position);
    emit labelPositionChanged();
}

QPieSlice::LabelPosition QPieSlice::labelPosition()
{
    Q_D(QPieSlice);
    return d->m_labelPosition;
}

void QPieSlice::setLabelColor(QColor color)
{
    Q_D(QPieSlice);
    if (d->m_labelColor == color)
        return;

    d->m_labelItem->setColor(color);
    d->m_labelColor = color;
    emit labelColorChanged();
}

QColor QPieSlice::labelColor() const
{
    Q_D(const QPieSlice);
    return d->m_labelColor;
}

void QPieSlice::setLabelFont(const QFont &font)
{
    Q_D(QPieSlice);
    d->m_labelFont = font;
    d->m_labelItem->setFont(font);
    emit labelFontChanged();
}

QFont QPieSlice::labelFont() const
{
    Q_D(const QPieSlice);
    return d->m_labelFont;
}

void QPieSlice::setLabelArmLengthFactor(qreal factor)
{
    Q_D(QPieSlice);

    if (QtPrivate::fuzzyCompare(d->m_labelArmLengthFactor, factor))
        return;

    d->m_labelArmLengthFactor = factor;
    emit labelArmLengthFactorChanged();
}

qreal QPieSlice::labelArmLengthFactor() const
{
    Q_D(const QPieSlice);
    return d->m_labelArmLengthFactor;
}

void QPieSlice::setValue(qreal value)
{
    Q_D(QPieSlice);
    value = qAbs(value); // negative values not allowed
    if (qFuzzyCompare(d->m_value + 1, value + 1))
        return;

    d->m_value = value;
    emit sliceChanged();
    emit valueChanged();
}

qreal QPieSlice::value() const
{
    Q_D(const QPieSlice);
    return d->m_value;
}

void QPieSlice::setExploded(bool exploded)
{
    Q_D(QPieSlice);

    if (d->m_isExploded == exploded)
        return;

    d->m_isExploded = exploded;
    emit sliceChanged();
    emit explodedChanged();
}

bool QPieSlice::isExploded() const
{
    Q_D(const QPieSlice);
    return d->m_isExploded;
}

void QPieSlice::setExplodeDistanceFactor(qreal factor)
{
    Q_D(QPieSlice);

    if (QtPrivate::fuzzyCompare(d->m_explodeDistanceFactor, factor))
        return;

    d->m_explodeDistanceFactor = factor;
    emit sliceChanged();
    emit explodeDistanceFactorChanged();
}

qreal QPieSlice::explodeDistanceFactor() const
{
    Q_D(const QPieSlice);
    return d->m_explodeDistanceFactor;
}

void QPieSlice::setColor(QColor color)
{
    Q_D(QPieSlice);
    if (d->m_color == color)
        return;

    d->m_color = color;
    emit colorChanged();
    emit sliceChanged();
}

QColor QPieSlice::color() const
{
    Q_D(const QPieSlice);
    return d->m_color;
}

void QPieSlice::setBorderColor(QColor borderColor)
{
    Q_D(QPieSlice);
    if (d->m_borderColor == borderColor)
        return;

    d->m_borderColor = borderColor;
    emit borderColorChanged();
    emit sliceChanged();
}

QColor QPieSlice::borderColor() const
{
    Q_D(const QPieSlice);
    return d->m_borderColor;
}

void QPieSlice::setBorderWidth(qreal borderWidth)
{
    Q_D(QPieSlice);
    if (d->m_borderWidth == borderWidth)
        return;

    d->m_borderWidth = borderWidth;
    emit borderWidthChanged();
    emit sliceChanged();
}

qreal QPieSlice::borderWidth() const
{
    Q_D(const QPieSlice);
    return d->m_borderWidth;
}

/*!
    Returns the PieSlice at the position \a index. Returns null if no PieSlice was found.
*/
QPieSlice *QPieSlice::at(qsizetype index) const
{
    QList<QPieSlice *> sliceList = subSlices();
    if (index >= 0 && index < sliceList.size())
        return sliceList[index];

    return 0;
}

/*!
    Searches for a PieSlice which contains the label \a label. Returns the PieSlice if found, null otherwise.
*/
QPieSlice *QPieSlice::find(const QString &label) const
{
    auto slicelist = subSlices();
    for (QPieSlice *slice : std::as_const(slicelist)) {
        if (slice->label() == label)
            return slice;
    }
    return 0;
}

/*!
    Replaces the PieSlice at position \a index with the one specified by \a slice.
    The original PieSlice will be permanently deleted. Returns \c false if replacing
    any of the PieSlices fails.
*/
bool QPieSlice::replace(qsizetype index, QPieSlice *slice)
{
    Q_D(QPieSlice);

    if (index < 0)
        index = 0;
    if (!slice || d->m_subSlices.contains(slice))
        return false;
    if (slice->series()) // already added to some series
        return false;
    if (qIsNaN(slice->value()) || qIsInf(slice->value()))
        return false;
    if (d->m_subSlices.size() <= index)
        return false;

    emit subSlicesRemoved(QList<QPieSlice *>() << d->m_subSlices[index]);
    delete d->m_subSlices[index];

    slice->setParent(this);
    slice->d_func()->updateSeries(d->m_series);

    d->m_subSlices[index] = slice;

    d->updateData();

    QObjectPrivate::connect(slice, &QPieSlice::sliceChanged, d, &QPieSlicePrivate::handleSliceChange);
    emit subSlicesReplaced(QList<QPieSlice *>() << slice);

    return true;
}

/*!
    Removes multiple PieSlices from the slice starting from \a index to a number of \a count.
    The PieSlices will be permanently deleted.
*/
void QPieSlice::removeMultiple(qsizetype index, int count)
{
    Q_D(QPieSlice);

    if (index < 0 || count < 1 || index + count > d->m_subSlices.size())
        return;

    QList<QPieSlice *> removedList;

    for (qsizetype i = index; i < index + count; ++i) {
        auto slice = d->m_subSlices[index];
        d->m_subSlices.removeOne(slice);
        d->updateData();

        removedList << slice;
    }

    emit subSlicesRemoved(removedList);

    for (auto slice : std::as_const(removedList))
        delete slice;

    emit subSlicesCountChanged(subSlicesCount());
}

/*!
    Removes the PieSlice at the location \a index. The PieSlice will be permanently deleted.
    Returns \c true if removing is successful.
*/
bool QPieSlice::remove(qsizetype index)
{
    Q_D(QPieSlice);

    if (index >= d->m_subSlices.size())
        return false;
    if (index < 0)
        return false;

    return remove(d->m_subSlices[index]);
}

/*!
    Replaces the PieSlice \a oldSlice with \a newSlice if found in the slice. \a oldSlice will
    be permanently deleted. Returns \c true if replacing is successful.
*/
bool QPieSlice::replace(QPieSlice *oldSlice, QPieSlice *newSlice)
{
    Q_D(QPieSlice);

    if (!oldSlice || !newSlice)
        return false;
    if (oldSlice == newSlice)
        return false;
    if (d->m_subSlices.contains(newSlice))
        return false;
    if (newSlice->series())
        return false;
    if (qIsNaN(newSlice->value()) || qIsInf(newSlice->value()))
        return false;

    for (int i = 0; i < d->m_subSlices.size(); ++i) {
        if (d->m_subSlices[i] == oldSlice) {
            emit subSlicesRemoved(QList<QPieSlice *>() << d->m_subSlices[i]);
            delete d->m_subSlices[i];

            newSlice->setParent(this);
            newSlice->d_func()->updateSeries(d->m_series);

            d->m_subSlices[i] = newSlice;

            d->updateData();

            QObjectPrivate::connect(newSlice, &QPieSlice::sliceChanged, d, &QPieSlicePrivate::handleSliceChange);
            emit subSlicesReplaced(QList<QPieSlice *>() << newSlice);

            return true;
        }
    }

    return false;
}

/*!
    Replaces the entire list of PieSlices in the slice with the list specified by \a slices.
    All the original PieSlices will be permanently deleted. Returns \c true if all PieSlices are
    replaced successfully.
*/
bool QPieSlice::replaceAll(const QList<QPieSlice *> &slices)
{
    Q_D(QPieSlice);

    for (const auto slice : slices) {
        if (!slice || d->m_subSlices.contains(slice))
            return false;
        if (slice->series())
            return false;
        if (qIsNaN(slice->value()) || qIsInf(slice->value()))
            return false;
    }

    emit subSlicesRemoved(d->m_subSlices);
    for (auto &slice : d->m_subSlices) {
        delete slice;
        slice = nullptr;
    }

    for (auto &slice : slices) {
        slice->setParent(this);
        slice->d_func()->updateSeries(d->m_series);
        QObjectPrivate::connect(slice, &QPieSlice::sliceChanged, d, &QPieSlicePrivate::handleSliceChange);
    }

    d->m_subSlices = slices;
    emit subSlicesReplaced(slices);

    return true;
}

/*!
    Appends the sub slice specified by \a slice to the slice.
    Sub slice ownership is passed to the slice.

    Returns \c true if appending succeeds.
*/
bool QPieSlice::append(QPieSlice *slice)
{
    return append(QList<QPieSlice *>() << slice);
}

/*!
    Appends the array of sub slices specified by \a slices to the slice.
    Sub slice ownership is passed to the slice.

    Returns \c true if appending succeeds.
*/
bool QPieSlice::append(const QList<QPieSlice *> &slices)
{
    Q_D(QPieSlice);

    if (slices.size() == 0)
        return false;

    for (auto *s : slices) {
        if (!s || d->m_subSlices.contains(s))
            return false;
        if (s->series()) // already added to some series
            return false;
        if (qIsNaN(s->value()) || qIsInf(s->value()))
            return false;
    }

    for (auto *s : slices) {
        s->setParent(this);
        s->d_func()->updateSeries(d->m_series);
        d->m_subSlices << s;
    }

    d->updateData();

    for (auto *s : slices)
        QObjectPrivate::connect(s, &QPieSlice::sliceChanged, d, &QPieSlicePrivate::handleSliceChange);

    emit subSlicesAdded(slices);
    emit subSlicesCountChanged(subSlicesCount());

    return true;
}

/*!
    Appends a single sub slice with the specified \a value and \a label to the slice.
    Sub slice ownership is passed to the slice.
    Returns null if \a value is \c NaN, \c Inf, or \c -Inf and adds nothing to the
    series.
*/
QPieSlice *QPieSlice::append(const QString &label, qreal value)
{
    if (!(qIsNaN(value) || qIsInf(value))) {
        QPieSlice *slice = new QPieSlice(label, value);
        append(slice);
        return slice;
    } else {
        return nullptr;
    }
}

/*!
    Inserts the sub slice specified by \a slice to the slice before the sub slice at
    the position specified by \a index.
    Sub slice ownership is passed to the slice.

    Returns \c true if inserting succeeds.
*/
bool QPieSlice::insert(qsizetype index, QPieSlice *slice)
{
    Q_D(QPieSlice);

    if (index < 0 || index > d->m_subSlices.size())
        return false;

    if (!slice || d->m_subSlices.contains(slice))
        return false;

    if (slice->series()) // already added to some series
        return false;

    if (qIsNaN(slice->value()) || qIsInf(slice->value()))
        return false;

    slice->setParent(this);
    slice->d_func()->updateSeries(d->m_series);
    d->m_subSlices.insert(index, slice);

    d->updateData();

    QObjectPrivate::connect(slice, &QPieSlice::sliceChanged, d, &QPieSlicePrivate::handleSliceChange);

    emit subSlicesAdded(QList<QPieSlice *>() << slice);
    emit subSlicesCountChanged(subSlicesCount());

    return true;
}

/*!
    Removes a single sub slice, specified by \a slice, from the slice and deletes it
    permanently.

    The pointer cannot be referenced after this call.

    Returns \c true if the removal succeeds.
*/
bool QPieSlice::remove(QPieSlice *slice)
{
    Q_D(QPieSlice);

    if (!d->m_subSlices.removeOne(slice))
        return false;

    d->updateData();

    emit subSlicesRemoved(QList<QPieSlice *>() << slice);
    emit subSlicesCountChanged(subSlicesCount());

    delete slice;
    slice = 0;

    return true;
}

/*!
    Takes a single sub slice, specified by \a slice, from the series. Does not delete
    the slice object.

    \note The slice remains the slice's parent object. You must set the
    parent object to take full ownership.

    Returns \c true if the take operation was successful.
*/
bool QPieSlice::take(QPieSlice *slice)
{
    Q_D(QPieSlice);

    if (!d->m_subSlices.removeOne(slice))
        return false;

    slice->d_func()->updateSeries(nullptr);
    slice->disconnect(this);

    d->updateData();

    emit subSlicesRemoved(QList<QPieSlice *>() << slice);
    emit subSlicesCountChanged(subSlicesCount());

    return true;
}

/*!
    Clears all sub slices from the slice.
*/
void QPieSlice::clear()
{
    Q_D(QPieSlice);
    if (d->m_subSlices.size() == 0)
        return;

    QList<QPieSlice *> slices = d->m_subSlices;
    for (QPieSlice *s : std::as_const(d->m_subSlices))
        d->m_subSlices.removeOne(s);

    d->updateData();

    emit subSlicesRemoved(slices);
    emit subSlicesCountChanged(subSlicesCount());

    for (QPieSlice *s : std::as_const(slices))
        delete s;
}

/*!
    Returns a list of sub slices that belong to this slice.
*/
QList<QPieSlice *> QPieSlice::subSlices() const
{
    Q_D(const QPieSlice);
    return d->m_subSlices;
}

/*!
    Returns the number of the sub slices in this slice.
*/
qsizetype QPieSlice::subSlicesCount() const
{
    Q_D(const QPieSlice);
    return d->m_subSlices.size();
}

/*!
    Returns \c true if the sub slices list is empty.
*/
bool QPieSlice::isEmpty() const
{
    Q_D(const QPieSlice);
    return d->m_subSlices.isEmpty();
}

/*!
    Returns the sum of all sub slices values in this slice.

    \sa QPieSlice::value(), QPieSlice::setValue(), QPieSlice::percentage()
*/
qreal QPieSlice::subSlicesSum() const
{
    Q_D(const QPieSlice);
    return d->m_sum;
}

void QPieSlice::setSubSlicesRatio(qreal subSlicesRatio)
{
    Q_D(QPieSlice);
    if (d->m_subSlicesRatio == subSlicesRatio)
        return;

    d->m_subSlicesRatio = subSlicesRatio;
    emit subSlicesRatioChanged(subSlicesRatio);
}

/*!
    \property QPieSlice::subSlicesRatio

    The relative size of the sub slices compared to the parent slice. The value ranges from 0.0 to 1.0,
    where 1.0 means that the sub slices take up the same amount of space as the parent slice would
    without sub slices. The default value is 0.6.
*/
qreal QPieSlice::subSlicesRatio() const
{
    Q_D(const QPieSlice);
    return d->m_subSlicesRatio;
}

void QPieSlicePrivate::handleSliceChange()
{
    Q_Q(QPieSlice);

    QPieSlice *pSlice = qobject_cast<QPieSlice *>(q->sender());
    Q_ASSERT(m_subSlices.contains(pSlice));
    updateData();
}

QPieSlicePrivate::QPieSlicePrivate()
    : m_isLabelVisible(false)
    , m_labelPosition(QPieSlice::LabelPosition::Outside)
    , m_labelArmLengthFactor(.15)
    , m_value(0.0)
    , m_percentage(0.0)
    , m_startAngle(0.0)
    , m_angleSpan(0.0)
    , m_hideLabel(false)
    , m_isExploded(false)
    , m_explodeDistanceFactor(.15)
    , m_labelDirty(false)
    , m_borderWidth(0.0)
    , m_shapePath(new QQuickShapePath)
    , m_labelItem(new QQuickText)
    , m_labelShape(new QQuickShape)
    , m_labelPath(new QQuickShapePath)
{
    m_labelItem->setColor(Qt::transparent);
    m_labelItem->setVisible(m_isLabelVisible);
    m_labelShape->setVisible(m_isLabelVisible);
    m_labelPath->setParent(m_labelShape);
    auto data = m_labelShape->data();
    data.append(&data, m_labelPath);
    m_labelPath->setFillColor(Qt::transparent);
}

QPieSlicePrivate::~QPieSlicePrivate() {}

void QPieSlicePrivate::updateData(bool clearHidden)
{
    Q_Q(QPieSlice);
    // calculate sum of all slices
    qreal sum = 0;
    for (QPieSlice *s : std::as_const(m_subSlices))
        sum += s->value();

    Q_ASSERT(sum >= 0.0);

    if (!qFuzzyCompare(m_sum + 1, sum + 1)) {
        m_sum = sum;
        emit q->subSlicesSumChanged(m_sum);
    }
    // nothing to show..
    if (qFuzzyIsNull(m_sum))
        return;
    // update slice attributes
    for (QPieSlice *s : std::as_const(m_subSlices)) {
        QPieSlicePrivate *d = s->d_func();
        d->setPercentage(s->value() / m_sum);
        d->setAngleSpan(m_angleSpan * d->m_percentage);
    }
    if (!m_series)
        return;
    auto hideMode = m_series->angleSpanLabelVisibility();
    bool hideNextSmallSlice = false;
    for (QPieSlice *s : std::as_const(m_subSlices)) {
        QPieSlicePrivate *d = s->d_func();
        // Reset hidden status
        if (clearHidden) {
            d->m_hideLabel = false;
            d->setLabelVisible(true);
        }
        // Check if current slice is small, and if the previous slice was also small
        // Hide the label on this one if the mode matches
        if (d->m_angleSpan < m_series->angleSpanVisibleLimit()
            && ((!hideNextSmallSlice && hideMode == QPieSeries::LabelVisibility::Even)
                || (hideNextSmallSlice
                    && (hideMode == QPieSeries::LabelVisibility::Odd
                        || hideMode == QPieSeries::LabelVisibility::First))
                || hideMode == QPieSeries::LabelVisibility::None)) {
            d->setLabelVisible(false, true);
        }
        if (hideMode == QPieSeries::LabelVisibility::First) {
            // Hide every other small slice label after the first shown one
            hideNextSmallSlice = d->m_angleSpan < m_series->angleSpanVisibleLimit();
        } else {
            // Hide only every other odd/even small slice label
            hideNextSmallSlice = (!hideNextSmallSlice && d->m_angleSpan < m_series->angleSpanVisibleLimit());
        }
    }
    emit m_series->update();
}

void QPieSlicePrivate::updateSeries(QPieSeries *series)
{
    m_series = series;

    for (QPieSlice *s : std::as_const(m_subSlices)) {
        QPieSlicePrivate *d = s->d_func();
        d->updateSeries(series);
    }
}

void QPieSlicePrivate::setPercentage(qreal percentage)
{
    Q_Q(QPieSlice);
    if (QtPrivate::fuzzyCompare(m_percentage, percentage))
        return;
    m_percentage = percentage;
    emit q->percentageChanged();
}

void QPieSlicePrivate::setStartAngle(qreal angle)
{
    Q_Q(QPieSlice);
    if (QtPrivate::fuzzyCompare(m_startAngle, angle))
        return;
    m_startAngle = angle;
    emit q->startAngleChanged();
}

void QPieSlicePrivate::setAngleSpan(qreal span)
{
    Q_Q(QPieSlice);
    if (QtPrivate::fuzzyCompare(m_angleSpan, span))
        return;

    m_angleSpan = span;
    emit q->angleSpanChanged();
}

void QPieSlicePrivate::setLabelVisible(bool visible, bool forceHidden)
{
    if (m_hideLabel)
        return;

    m_hideLabel = forceHidden;
    m_isLabelVisible = (visible && !m_hideLabel);
    m_labelItem->setVisible(m_isLabelVisible);
    if (m_labelPosition == QPieSlice::LabelPosition::Outside)
        m_labelShape->setVisible(m_isLabelVisible);
}

void QPieSlicePrivate::setLabelPosition(QPieSlice::LabelPosition position)
{
    m_labelPosition = position;

    if (position == QPieSlice::LabelPosition::Outside) {
        m_labelShape->setVisible(m_isLabelVisible);
        qreal radian = qDegreesToRadians(m_startAngle + (m_angleSpan * .5));
        QQuickText *labelItem = m_labelItem;
        qreal height = labelItem->height();
        qreal labelWidth = radian > M_PI ? -labelItem->width() : labelItem->width();
        if (labelWidth > 0)
            labelItem->setX(m_labelArm.x());
        else
            labelItem->setX(m_labelArm.x() + labelWidth);
        labelItem->setY(m_labelArm.y() - height);
        labelItem->setRotation(0);
    } else {
        m_labelShape->setVisible(false);
        qreal centerX = (m_largeArc.x() + m_centerLine.x()) / 2.0;
        qreal centerY = (m_largeArc.y() + m_centerLine.y()) / 2.0;
        QQuickText *labelItem = m_labelItem;
        centerX -= labelItem->width() * .5;
        centerY -= labelItem->height() * .5;
        labelItem->setPosition(QPointF(centerX, centerY));

        if (position == QPieSlice::LabelPosition::InsideHorizontal) {
            labelItem->setRotation(0);
        } else if (position == QPieSlice::LabelPosition::InsideTangential) {
            labelItem->setRotation(m_startAngle + (m_angleSpan * .5));
        } else if (position == QPieSlice::LabelPosition::InsideNormal) {
            qreal angle = m_startAngle + (m_angleSpan * .5);
            if (angle > 180)
                angle += 90;
            else
                angle -= 90;
            labelItem->setRotation(angle);
        }
    }
}

QT_END_NAMESPACE

#include "moc_qpieslice.cpp"
