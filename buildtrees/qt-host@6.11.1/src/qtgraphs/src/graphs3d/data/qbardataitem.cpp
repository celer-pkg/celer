// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


/*!
 * \class QBarDataItem
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QBarDataItem class provides a container for resolved data to be
 * added to bar graphs.
 *
 * A bar data item holds the data for a single rendered bar in a graph.
 * Bar data proxies parse data into QBarDataItem instances for bar graphs.
 *
 * \sa QBarDataProxy, {Qt Graphs C++ Classes for 3D}
 */

/*!
 * \fn constexpr QBarDataItem::QBarDataItem() noexcept
 * Constructs a bar data item.
 */

/*!
 * \fn explicit constexpr QBarDataItem::QBarDataItem(float value) noexcept
 * Constructs a bar data item with the value \a value.
 */

/*!
 * \fn explicit constexpr QBarDataItem::QBarDataItem(float value, float angle) noexcept
 * Constructs a bar data item with the value \a value and angle \a angle.
 */

/*!
 * \fn void QBarDataItem::setValue(float val)
 * Sets the value \a val to this data item.
 */

/*!
 * \fn float QBarDataItem::value() const
 * Returns the value of this data item.
 */

/*!
 * \fn void QBarDataItem::setRotation(float angle)
 * Sets the rotation angle \a angle in degrees for this data item.
 */

/*!
 * \fn float QBarDataItem::rotation() const
 * Returns the rotation angle in degrees for this data item.
 */
