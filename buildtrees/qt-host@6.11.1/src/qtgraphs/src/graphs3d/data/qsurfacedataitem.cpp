// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


/*!
 * \class QSurfaceDataItem
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QSurfaceDataItem class provides a container for resolved data to
 * be added to surface graphs.
 *
 * A surface data item holds the data for a single vertex in a surface graph.
 * Surface data proxies parse data into QSurfaceDataItem instances for
 * surface graphs.
 *
 * \sa QSurfaceDataProxy, {Qt Graphs C++ Classes for 3D}
 */

/*!
 * \fn constexpr QSurfaceDataItem::QSurfaceDataItem() noexcept
 * Constructs a surface data item.
 */

/*!
 * \fn explicit constexpr QSurfaceDataItem::QSurfaceDataItem(QVector3D position) noexcept
 * Constructs a surface data item at the position \a position.
 */

/*!
 * \fn constexpr QSurfaceDataItem::QSurfaceDataItem(float x, float y, float z) noexcept
 * Constructs a surface data item at the position specified by \a x, \a y, and \a z.
 */

/*!
 * \fn void QSurfaceDataItem::setPosition(QVector3D pos)
 * Sets the position \a pos to this data item.
 */

/*!
 * \fn QVector3D QSurfaceDataItem::position() const
 * Returns the position of this data item.
 */

/*!
 * \fn void QSurfaceDataItem::setX(float value)
 * Sets the x-coordinate of the item position to the value \a value.
 */

/*!
 * \fn void QSurfaceDataItem::setY(float value)
 * Sets the y-coordinate of the item position to the value \a value.
 */

/*!
 * \fn void QSurfaceDataItem::setZ(float value)
 * Sets the z-coordinate of the item position to the value \a value.
 */

/*!
 * \fn float QSurfaceDataItem::x() const
 * Returns the x-coordinate of the position of this data item.
 */

/*!
 * \fn float QSurfaceDataItem::y() const
 * Returns the y-coordinate of the position of this data item.
 */

/*!
 * \fn float QSurfaceDataItem::z() const
 * Returns the z-coordinate of the position of this data item.
 */
