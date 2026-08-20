// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtGraphs/qbarset.h>
#include <private/qbarset_p.h>
#include <private/charthelpers_p.h>

QT_BEGIN_NAMESPACE

/*!
    \class QBarSet
    \inmodule QtGraphs
    \ingroup graphs_2D
    \brief The QBarSet class represents one set of bars in a bar graph.

    A bar set contains one data value for each category. The first value of a set is assumed to
    belong to the first category, the second one to the second category, and so on. If the set has
    fewer values than there are categories, the missing values are assumed to be located at the end
    of the set. For missing values in the middle of a set, the numerical value of zero is used.
    Labels for zero value sets are not shown.

    \sa QBarSeries
*/
/*!
    \qmltype BarSet
    \nativetype QBarSet
    \inqmlmodule QtGraphs
    \ingroup graphs_qml_2D
    \brief Represents one set of bars in a bar graph.

    A bar set contains one data value for each category. The first value of a set is assumed to
    belong to the first category, the second one to the second category, and so on. If the set has
    fewer values than there are categories, the missing values are assumed to be located at the end
    of the set. For missing values in the middle of a set, the numerical value of zero is used.
    Labels for zero value sets are not shown.

    \sa BarSeries
*/

/*!
    \property QBarSet::label
    \brief The label of the bar set.
*/
/*!
    \qmlproperty string BarSet::label
    The label of the bar set.
*/

/*!
    \property QBarSet::color
    \brief The fill color of the bar set.
*/
/*!
    \qmlproperty color BarSet::color
    The fill color of the bar set.
*/

/*!
    \property QBarSet::selectedColor
    \brief The fill color of the selected set.
*/
/*!
    \qmlproperty color BarSet::selectedColor
    The fill color of the selected set.
*/

/*!
    \property QBarSet::borderColor
    \brief The border color of the bar set.
*/
/*!
    \qmlproperty color BarSet::borderColor
    The border color of the bar set.
*/

/*!
    \property QBarSet::borderWidth
    \brief The width of the border line.
    By default, the width is -1, meaning the border width is defined by the theme.
*/
/*!
    \qmlproperty real BarSet::borderWidth
     By default, the width is -1, meaning the border width is defined by the theme.
*/

/*!
    \property QBarSet::count
    \brief The number of values in the bar set.
*/
/*!
    \qmlproperty int BarSet::count
    The number of values in the bar set.
*/

/*!
    \property QBarSet::labelColor
    \brief The text (label) color of the bar set.
*/
/*!
    \qmlproperty color BarSet::labelColor
    The text (label) color of the bar set.
*/

/*!
    \property QBarSet::selectedBars
    \brief The indexes of the bars which are currently selected.
*/
/*!
    \qmlproperty list BarSet::selectedBars
    The indexes of the bars which are currently selected.
*/

/*!
   \property QBarSet::values
   \brief The values of the bar set.

    You can set a list of either \l [QML]{real} or \l [QML]{point}
    types as values.

    If you set a list of real types as values, they directly define the bar set values.

    If you set a list of point types as values, the x-coordinate of the point specifies its
    zero-based index in the bar set. The size of the bar set is the highest x-coordinate value + 1.
    If a point is missing for any x-coordinate between zero and the highest value,
    it gets the value zero.
 */
/*!
    \qmlproperty list<variant> BarSet::values
    The values of the bar set. You can set a list of either \l [QML]{real} or \l [QML]{point}
    types as values.

    If you set a list of real types as values, they directly define the bar set values.

    If you set a list of point types as values, the x-coordinate of the point specifies its
    zero-based index in the bar set. The size of the bar set is the highest x-coordinate value + 1.
    If a point is missing for any x-coordinate between zero and the highest value,
    it gets the value zero.

    For example, the following bar sets have equal values:
    \code
        myBarSet1.values = [5, 0, 1, 5];
        myBarSet2.values = [Qt.point(0, 5), Qt.point(2, 1), Qt.point(3, 5)];
    \endcode
*/

/*!
    \fn void QBarSet::update()
    This signal is emitted when the barset is updated.
*/
/*!
    \qmlsignal BarSet::update()
    This signal is emitted when the barset is updated.
*/

/*!
    \qmlsignal BarSet::labelChanged()
    This signal is emitted when the label of the bar set changes.
    \sa label
*/

/*!
    \qmlsignal BarSet::colorChanged(color)
    This signal is emitted when the fill color of the bar set changes to \a color.
*/

/*!
    \qmlsignal BarSet::borderColorChanged(color)
    This signal is emitted when the border color of the bar set changes to \a color.
*/

/*!
    \qmlsignal BarSet::labelColorChanged(color)
    This signal is emitted when the text (label) color of the bar set changes to \a color.
*/

/*!
    \qmlsignal BarSet::valuesChanged()
    This signal is emitted when the values of the bar set change.
*/

/*!
    \qmlsignal BarSet::selectedColorChanged(color color)
    This signal is emitted when the selected bar color changes. The new color is
    \a color.
*/

/*!
    \qmlsignal BarSet::countChanged()
    This signal is emitted when the barset's value count changes.
*/

/*!
    \qmlsignal BarSet::borderWidthChanged(real width)
    This signal is emitted when the barset's border width changes.
    The new width is \a width.
*/

/*!
    \fn void QBarSet::valuesAdded(qsizetype index, qsizetype count)
    This signal is emitted when new values are added to the bar set.
    \a index indicates the position of the first inserted value, and \a count is the number
    of inserted values.
    \sa append(), insert()
*/
/*!
    \qmlsignal BarSet::valuesAdded(int index, int count)
    This signal is emitted when new values are added to the bar set.
    \a index indicates the position of the first inserted value, and \a count is the number
    of inserted values.
*/

/*!
    \fn void QBarSet::valuesRemoved(qsizetype index, qsizetype count)
    This signal is emitted when values are removed from the bar set.
    \a index indicates the position of the first removed value, and \a count is the number
    of removed values.
    \sa remove()
*/
/*!
    \qmlsignal BarSet::valuesRemoved(int index, int count)
    This signal is emitted when values are removed from the bar set.
    \a index indicates the position of the first removed value, and \a count is the number
    of removed values.
*/

/*!
    \fn void QBarSet::valueChanged(qsizetype index)
    This signal is emitted when the value at the position specified by \a index is modified.
    \sa at()
*/
/*!
    \qmlsignal BarSet::valueChanged(int index)
    This signal is emitted when the value at the position specified by \a index is modified.
*/

/*!
    \fn void QBarSet::updatedBars()
    This signal is emitted when the bars in this set are updated.
*/
/*!
    \qmlsignal BarSet::updatedBars()
    This signal is emitted when the bars in this set are updated.
*/

/*!
    \fn void QBarSet::valueAdded(qsizetype index, qsizetype count)
    This signal is emitted when new values are added to the bar set.
    \a index indicates the position of the first inserted value, and \a count
    is the number of inserted values.
*/
/*!
    \qmlsignal BarSet::valueAdded(int index, int count)
    This signal is emitted when new values are added to the bar set.
    \a index indicates the position of the first inserted value, and \a count
    is the number of inserted values.
*/

/*!
    \fn void QBarSet::valueRemoved(qsizetype index, qsizetype count)
    This signal is emitted when values are removed from the bar set.
    \a index indicates the position of the first removed value, and \a count
    is the number of removed values.
*/
/*!
    \qmlsignal BarSet::valueRemoved(int index, int count)
    This signal is emitted when values are removed from the bar set.
    \a index indicates the position of the first removed value, and \a count
    is the number of removed values.
*/

/*!
    \fn void QBarSet::selectedBarsChanged(const QList<qsizetype> &indexes)
    This signal is emitted when the selected bar changes. \a indexes is
    a list selected bar indexes.
*/
/*!
    \qmlsignal BarSet::selectedBarsChanged(list<int> indexes)
    This signal is emitted when the selected bar changes. \a indexes is
    a list selected bar indexes.
*/

QBarSet::QBarSet(QObject *parent)
    : QBarSet(QString(), parent)
{}

/*!
    Constructs a bar set with the label \a label and the parent \a parent.
*/
QBarSet::QBarSet(const QString &label, QObject *parent)
    : QObject(*(new QBarSetPrivate(label)), parent)
{}

/*!
    Removes the bar set.
*/
QBarSet::~QBarSet()
{
    // NOTE: d_ptr destroyed by QObject
}

/*!
    Sets \a label as the new label for the bar set.
*/
void QBarSet::setLabel(const QString &label)
{
    Q_D(QBarSet);
    if (d->m_label != label) {
        d->m_label = label;
        d->setLabelsDirty(true);
        emit update();
        emit labelChanged();
    }
}

/*!
    Returns the label of the bar set.
*/
QString QBarSet::label() const
{
    Q_D(const QBarSet);
    return d->m_label;
}

/*!
    \qmlmethod void BarSet::append(real value)
    Appends the new value specified by \a value to the end of the bar set.
*/
/*!
    Appends the new value specified by \a value to the end of the bar set.
*/
void QBarSet::append(qreal value)
{
    Q_D(QBarSet);
    // Convert to QPointF
    qsizetype index = d->m_values.size();
    d->append(QPointF(d->m_values.size(), value));
    emit valuesAdded(index, 1);
    emit countChanged();
    emit update();
}

/*!
    \qmlmethod void BarSet::append(list<real> values)
    Appends the list of real values specified by \a values to the end of the bar set.

    \sa append()
*/
/*!
    Appends the list of real values specified by \a values to the end of the bar set.

    \sa append()
*/
void QBarSet::append(const QList<qreal> &values)
{
    Q_D(QBarSet);
    qsizetype index = d->m_values.size();
    d->append(values);
    emit valuesAdded(index, values.size());
    emit countChanged();
    emit update();
}

/*!
    \qmlmethod void BarSet::insert(int index, real value)
    Inserts \a value in the position specified by \a index.
    The values following the inserted value are moved up one position.

    \sa remove()
*/
/*!
    Inserts \a value in the position specified by \a index.
    The values following the inserted value are moved up one position.

    \sa remove()
*/
void QBarSet::insert(qsizetype index, qreal value)
{
    Q_D(QBarSet);
    d->insert(index, value);

    bool callSignal = false;
    if (!d->m_selectedBars.isEmpty()) {
        // if value was inserted we need to move already selected bars by 1
        QSet<qsizetype> selectedAfterInsert;
        for (const auto &value : std::as_const(d->m_selectedBars)) {
            if (value >= index) {
                selectedAfterInsert << value + 1;
                callSignal = true;
            } else {
                selectedAfterInsert << value;
            }
        }
        d->m_selectedBars = selectedAfterInsert;
        emit update();
    }

    emit valuesAdded(index, 1);
    emit countChanged();
    if (callSignal)
        emit selectedBarsChanged(selectedBars());
}

/*!
    \qmlmethod void BarSet::remove(int index, int count)
    Removes the number of values specified by \a count from the bar set starting
    with the value specified by \a index.

    If you leave out \a count, only the value specified by \a index is removed.
*/
/*!
    Removes the number of values specified by \a count from the bar set starting with
    the value specified by \a index.
    \sa insert()
*/
void QBarSet::remove(qsizetype index, qsizetype count)
{
    Q_D(QBarSet);
    qsizetype removedCount = d->remove(index, count);
    if (removedCount > 0) {
        emit valuesRemoved(index, removedCount);
        emit countChanged();
        emit update();
    }
}

/*!
    \qmlmethod void BarSet::replace(int index, real value)
    Adds the value specified by \a value to the bar set at the position
    specified by \a index.
*/
/*!
    Adds the value specified by \a value to the bar set at the position specified by \a index.
*/
void QBarSet::replace(qsizetype index, qreal value)
{
    Q_D(QBarSet);
    if (index >= 0 && index < d->m_values.size()) {
        d->replace(index, value);
        emit valueChanged(index);
        emit update();
    }
}

/*!
    \qmlmethod real BarSet::at(int index)
    Returns the value specified by \a index from the bar set.
    If the index is out of bounds, 0.0 is returned.
*/
/*!
    Returns the value specified by \a index from the bar set.
    If the index is out of bounds, 0.0 is returned.
*/
qreal QBarSet::at(qsizetype index) const
{
    Q_D(const QBarSet);
    if (index < 0 || index >= d->m_values.size())
        return 0;
    return d->m_values.at(index).y();
}

/*!
    \qmlmethod int BarSet::count()
    Returns the number of values in a bar set.
*/
/*!
    Returns the number of values in a bar set.
*/
qsizetype QBarSet::count() const
{
    Q_D(const QBarSet);
    return d->m_values.size();
}

/*!
    \qmlmethod real BarSet::sum()
    Returns the sum of all values in the bar set.
*/
/*!
    Returns the sum of all values in the bar set.
*/
qreal QBarSet::sum() const
{
    Q_D(const QBarSet);
    qreal total(0);
    for (int i = 0; i < d->m_values.size(); i++)
        total += d->m_values.at(i).y();
    return total;
}

/*!
    \qmlmethod void BarSet::clear()
    Removes all values from the set.
*/
/*!
    Removes all values from the set.
*/
void QBarSet::clear()
{
    Q_D(QBarSet);
    remove(0, d->m_values.size());
}

/*!
    A convenience operator for appending the real value specified by \a value to the end of the
    bar set.

    \sa append()
*/
QBarSet &QBarSet::operator << (qreal value)
{
    append(value);
    return *this;
}

/*!
    Returns the value of the bar set specified by \a index.
    If the index is out of bounds, 0.0 is returned.
*/
qreal QBarSet::operator [](qsizetype index) const
{
    return at(index);
}

/*!
    Returns the fill color for the bar set.
*/
QColor QBarSet::color() const
{
    Q_D(const QBarSet);
    return d->m_color;
}

/*!
    Sets the fill color for the bar set to \a color.
*/
void QBarSet::setColor(QColor color)
{
    Q_D(QBarSet);
    if (d->m_color != color) {
        d->m_color = color;
        emit update();
        emit colorChanged(color);
    }
}

/*!
    Returns the line color for the bar set.
*/
QColor QBarSet::borderColor() const
{
    Q_D(const QBarSet);
    return d->m_borderColor;
}

/*!
    Sets the line color for the bar set to \a color.
*/
void QBarSet::setBorderColor(QColor color)
{
    Q_D(QBarSet);
    if (d->m_borderColor != color) {
        d->m_borderColor = color;
        emit update();
        emit borderColorChanged(color);
    }
}

/*!
    Returns the text color for the bar set.
*/
QColor QBarSet::labelColor() const
{
    Q_D(const QBarSet);
    return d->m_labelColor;
}

/*!
    Sets the text color for the bar set to \a color.
*/
void QBarSet::setLabelColor(QColor color)
{
    Q_D(QBarSet);
    if (d->m_labelColor != color) {
        d->m_labelColor = color;
        emit update();
        emit labelColorChanged(color);
    }
}

/*!
    Returns the color of the selected bars.

    This is the fill (brush) color of bars marked as selected. If not specified,
    value of QBarSet::color is used as default.
    \sa color
*/
QColor QBarSet::selectedColor() const
{
    Q_D(const QBarSet);
    return d->m_selectedColor;
}

/*!
    Sets the \a color of the selected bars.
    \sa selectedColor
*/
void QBarSet::setSelectedColor(QColor color)
{
    Q_D(QBarSet);
    if (d->m_selectedColor != color) {
        d->m_selectedColor = color;
        d->setLabelsDirty(true);
        emit update();
        emit updatedBars();
        emit selectedColorChanged(color);
    }
}


qreal QBarSet::borderWidth() const
{
    Q_D(const QBarSet);
    return d->m_borderWidth;
}

void QBarSet::setBorderWidth(qreal width)
{
    Q_D(QBarSet);
    width = qMax(0.0, width);
    if (!qFuzzyCompare(d->m_borderWidth + 1, width + 1)) {
        d->m_borderWidth = width;
        emit update();
        emit borderWidthChanged(width);
    }
}

QVariantList QBarSet::values() const
{
    QVariantList values;
    for (qsizetype i(0); i < count(); i++)
        values.append(QVariant(QBarSet::at(i)));
    return values;
}

void QBarSet::setValues(const QVariantList &values)
{
    bool valuesUpdated = false;
    // See if we can replace values instead of remove & add all.
    // This way e.g. selections remain.
    const bool doReplace = count() == values.size();

    if (!doReplace) {
        while (count())
            remove(count() - 1);
        valuesUpdated = true;
    }

    if (values.size() > 0 && values.at(0).canConvert<QPoint>()) {
        // Create list of values for appending if the first item is Qt.point
        int maxValue = 0;
        for (int i = 0; i < values.size(); i++) {
            if (values.at(i).canConvert<QPoint>() &&
                values.at(i).toPoint().x() > maxValue) {
                maxValue = values.at(i).toPoint().x();
            }
        }

        QList<qreal> indexValueList;
        indexValueList.resize(maxValue + 1);

        for (int i = 0; i < values.size(); i++) {
            if (values.at(i).canConvert<QPoint>())
                indexValueList.replace(values.at(i).toPoint().x(), values.at(i).toPointF().y());
        }

        for (int i = 0; i < indexValueList.size(); i++) {
            if (doReplace)
                QBarSet::replace(i, indexValueList.at(i));
            else
                QBarSet::append(indexValueList.at(i));
            valuesUpdated = true;
        }

    } else {
        for (int i(0); i < values.size(); i++) {
            if (values.at(i).canConvert<double>()) {
                if (doReplace)
                    QBarSet::replace(i, values[i].toDouble());
                else
                    QBarSet::append(values[i].toDouble());
                valuesUpdated = true;
            }
        }
    }
    emit update();
    if (valuesUpdated)
        emit valuesChanged();
}


/*!
    \qmlmethod bool BarSet::isBarSelected(int index)
    Returns \c true if the bar at the given \a index is among selected bars and \c false otherwise.
    \note Selected bars are drawn using the selected color if it was specified using BarSet::setSelectedColor.
    \sa selectedBars, setBarSelected(), selectedColor
 */
/*!
    Returns \c true if the bar at the given \a index is among selected bars and \c false otherwise.
    \note Selected bars are drawn using the selected color if it was specified using QBarSet::setSelectedColor.
    \sa selectedBars(), setBarSelected(), setSelectedColor()
 */
bool QBarSet::isBarSelected(qsizetype index) const
{
    Q_D(const QBarSet);
    return d->isBarSelected(index);
}

/*!
    \qmlmethod void BarSet::selectBar(int index)
    Marks the bar at \a index as selected.
    \note Emits BarSet::selectedBarsChanged.
    \sa setBarSelected()
 */
/*!
    Marks the bar at \a index as selected.
    \note Emits QBarSet::selectedBarsChanged.
    \sa setBarSelected()
 */
void QBarSet::selectBar(qsizetype index)
{
    setBarSelected(index, true);
}

/*!
    \qmlmethod void BarSet::deselectBar(int index)
    Deselects the bar at \a index.
    \note Emits BarSet::selectedBarsChanged.
    \sa setBarSelected()
 */
/*!
    Deselects the bar at \a index.
    \note Emits QBarSet::selectedBarsChanged.
    \sa setBarSelected()
 */
void QBarSet::deselectBar(qsizetype index)
{
    setBarSelected(index, false);
}

/*!
    \qmlmethod void BarSet::setBarSelected(int index, bool selected)
    Marks the bar at \a index as either selected or deselected as specified by \a selected.
    \note Selected bars are drawn using the selected color if it was specified. Emits BarSet::selectedBarsChanged.
    \sa selectedColor
 */
/*!
    Marks the bar at \a index as either selected or deselected as specified by \a selected.
    \note Selected bars are drawn using the selected color if it was specified. Emits QBarSet::selectedBarsChanged.
    \sa setSelectedColor()
 */
void QBarSet::setBarSelected(qsizetype index, bool selected)
{
    Q_D(QBarSet);
    bool callSignal = false;
    d->setBarSelected(index, selected, callSignal);

    if (callSignal)
        emit selectedBarsChanged(selectedBars());
    emit update();
}

/*!
    \qmlmethod void BarSet::selectAllBars()
    Marks all bars in the set as selected.
    \note Emits BarSet::selectedBarsChanged.
    \sa setBarSelected()
 */
/*!
    Marks all bars in the set as selected.
    \note Emits QBarSet::selectedBarsChanged.
    \sa setBarSelected()
 */
void QBarSet::selectAllBars()
{
    Q_D(QBarSet);
    bool callSignal = false;
    for (int i = 0; i < d->m_values.size(); ++i)
        d->setBarSelected(i, true, callSignal);

    if (callSignal)
        emit selectedBarsChanged(selectedBars());
    emit update();
}

/*!
    \qmlmethod void BarSet::deselectAllBars()
    Deselects all bars in the set.
    \note Emits BarSet::selectedBarsChanged.
    \sa setBarSelected()
 */
/*!
    Deselects all bars in the set.
    \note Emits QBarSet::selectedBarsChanged.
    \sa setBarSelected()
 */
void QBarSet::deselectAllBars()
{
    Q_D(QBarSet);
    bool callSignal = false;
    for (int i = 0; i < d->m_values.size(); ++i)
        d->setBarSelected(i, false, callSignal);

    if (callSignal)
        emit selectedBarsChanged(selectedBars());
    emit update();
}

/*!
    \qmlmethod void BarSet::selectBars(list<int> indexes)
    Marks multiple bars passed in an \a indexes list as selected.
    \note Emits BarSet::selectedBarsChanged.
    \sa setBarSelected()
 */
/*!
    Marks multiple bars passed in an \a indexes list as selected.
    \note Emits QBarSet::selectedBarsChanged.
    \sa setBarSelected()
 */
void QBarSet::selectBars(const QList<qsizetype> &indexes)
{
    Q_D(QBarSet);
    bool callSignal = false;
    for (const qsizetype &index : indexes)
        d->setBarSelected(index, true, callSignal);

    if (callSignal)
        emit selectedBarsChanged(selectedBars());
    emit update();
}

/*!
    \qmlmethod void BarSet::deselectBars(list<int> indexes)
    Marks multiple bars passed in an \a indexes list as deselected.
    \note Emits BarSet::selectedBarsChanged.
    \sa setBarSelected()
 */
/*!
    Marks multiple bars passed in an \a indexes list as deselected.
    \note Emits QBarSet::selectedBarsChanged.
    \sa setBarSelected()
 */
void QBarSet::deselectBars(const QList<qsizetype> &indexes)
{
    Q_D(QBarSet);
    bool callSignal = false;
    for (const qsizetype &index : indexes)
        d->setBarSelected(index, false, callSignal);

    if (callSignal)
        emit selectedBarsChanged(selectedBars());
    emit update();
}

/*!
    \qmlmethod void BarSet::toggleSelection(list<int> indexes)
    Changes the selection state of bars at the given \a indexes to the opposite one.
    \note Emits BarSet::selectedBarsChanged.
    \sa setBarSelected()
 */
/*!
    Changes the selection state of bars at the given \a indexes to the opposite one.
    \note Emits QBarSet::selectedBarsChanged.
    \sa setBarSelected()
 */
void QBarSet::toggleSelection(const QList<qsizetype> &indexes)
{
    Q_D(QBarSet);
    bool callSignal = false;
    for (const qsizetype &index : indexes)
        d->setBarSelected(index, !isBarSelected(index), callSignal);

    if (callSignal)
        emit selectedBarsChanged(selectedBars());
    emit update();
}

/*!
  Returns a list of bars marked as selected.
  \sa setBarSelected()
 */
QList<qsizetype> QBarSet::selectedBars() const
{
    Q_D(const QBarSet);
    return QList<qsizetype>(d->m_selectedBars.begin(), d->m_selectedBars.end());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QBarSetPrivate::QBarSetPrivate(const QString &label)
    : m_label(label)
    , m_visualsDirty(true)
{}

QBarSetPrivate::~QBarSetPrivate() {}

void QBarSetPrivate::append(QPointF value)
{
    if (isValidValue(value)) {
        Q_Q(QBarSet);
        m_values.append(value);
        emit q->valueAdded(m_values.size() - 1, 1);
    }
}

void QBarSetPrivate::append(const QList<QPointF> &values)
{
    qsizetype originalIndex = m_values.size();
    for (const auto &value : values) {
        if (isValidValue(value))
            m_values.append(value);
    }
    Q_Q(QBarSet);
    emit q->valueAdded(originalIndex, values.size());
}

void QBarSetPrivate::append(const QList<qreal> &values)
{
    qsizetype originalIndex = m_values.size();
    qsizetype index = originalIndex;
    for (const auto value : values) {
        if (isValidValue(value)) {
            m_values.append(QPointF(index, value));
            index++;
        }
    }
    Q_Q(QBarSet);
    emit q->valueAdded(originalIndex, values.size());
}

void QBarSetPrivate::insert(qsizetype index, qreal value)
{
    m_values.insert(index, QPointF(index, value));
    Q_Q(QBarSet);
    emit q->valueAdded(index, 1);
}

void QBarSetPrivate::insert(qsizetype index, QPointF value)
{
    m_values.insert(index, value);
    Q_Q(QBarSet);
    emit q->valueAdded(index, 1);
}

qsizetype QBarSetPrivate::remove(qsizetype index, qsizetype count)
{
    qsizetype removeCount = count;

    if ((index < 0) || (m_values.size() == 0))
        return 0; // Invalid index or not values in list, remove nothing.
    else if ((index + count) > m_values.size())
        removeCount = m_values.size() - index; // Trying to remove more items than list has. Limit amount to be removed.

    int c = 0;
    while (c < removeCount) {
        m_values.removeAt(index);
        c++;
    }

    bool callSignal = false;
    if (!m_selectedBars.empty()) {
        QSet<qsizetype> selectedAfterRemoving;

        for (const qsizetype &selectedBarIndex : std::as_const(m_selectedBars)) {
            if (selectedBarIndex < index) {
                selectedAfterRemoving << selectedBarIndex;
            } else if (selectedBarIndex >= index + removeCount) {
                selectedAfterRemoving << selectedBarIndex - removeCount;
                callSignal = true;
            } else {
                callSignal = true;
            }
        }

        m_selectedBars = selectedAfterRemoving;
    }
    Q_Q(QBarSet);
    emit q->valueRemoved(index, removeCount);
    if (callSignal)
        emit q->selectedBarsChanged(q->selectedBars());

    return removeCount;
}

void QBarSetPrivate::replace(qsizetype index, qreal value)
{
    if (index < 0 || index >= m_values.size())
        return;

    m_values.replace(index, QPointF(index, value));
}

qreal QBarSetPrivate::pos(qsizetype index) const
{
    if (index < 0 || index >= m_values.size())
        return 0;
    return m_values.at(index).x();
}

qreal QBarSetPrivate::value(qsizetype index) const
{
    if (index < 0 || index >= m_values.size())
        return 0;
    return m_values.at(index).y();
}

void QBarSetPrivate::setBarSelected(qsizetype index, bool selected, bool &callSignal)
{
    if (index < 0 || index > m_values.size() - 1)
        return;

    if (selected) {
        if (!isBarSelected(index)) {
            m_selectedBars << index;
            callSignal = true;
        }
    } else {
        if (isBarSelected(index)) {
            m_selectedBars.remove(index);
            callSignal = true;
        }
    }

    if (callSignal)
        setVisualsDirty(true);
}

bool QBarSetPrivate::isBarSelected(qsizetype index) const
{
    return m_selectedBars.contains(index);
}

QT_END_NAMESPACE

#include "moc_qbarset.cpp"
