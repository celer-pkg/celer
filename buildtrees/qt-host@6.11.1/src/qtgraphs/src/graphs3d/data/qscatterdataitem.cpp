// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


/*!
 * \class QScatterDataItem
 * \inmodule QtGraphs
 * \ingroup graphs_3D
 * \brief The QScatterDataItem class provides a container for resolved data to
 * be added to scatter graphs.
 *
 * A scatter data item holds the data for a single rendered item in a scatter
 * graph. Scatter data proxies parse data into QScatterDataItem instances for
 * scatter graphs.
 *
 * \sa QScatterDataProxy, {Qt Graphs C++ Classes for 3D}
 */

/*!
 * \fn QScatterDataItem::QScatterDataItem()
 * Default constructor for scatter data item.
 */

/*!
 * \fn QScatterDataItem::QScatterDataItem(QVector3D position)
 * Constructs scatter data item with position \a position.
 */

/*!
 * \fn QScatterDataItem::QScatterDataItem(float x, float y, float z)
 * Constructs a scatter data item at the position specified by \a x, \a y, and \a z.
 */

/*!
 * \fn QScatterDataItem::QScatterDataItem(QVector3D position, const QQuaternion &rotation)
 * Constructs scatter data item with position \a position
 * and rotation \a rotation.
 */

/*!
 * \fn QScatterDataItem::QScatterDataItem(QVector3D position, const QVector3D &scale)
 * Constructs scatter data item with position \a position
 * and scale \a scale.
 */

/*!
 * \fn QScatterDataItem::QScatterDataItem(QVector3D position, const QQuaternion &rotation, const QVector3D &scale)
 * Constructs scatter data item with position \a position,
 * rotation \a rotation, and scale \a scale.
 */

/*!
 * \fn void QScatterDataItem::setPosition(QVector3D pos)
 * Sets the position \a pos for this data item.
 */

/*!
 * \fn QVector3D QScatterDataItem::position() const
 * Returns the position of this data item.
 */

/*!
 * \fn void QScatterDataItem::setRotation(const QQuaternion &rot)
 * Sets the rotation \a rot for this data item.
 * The value of \a rot should be a normalized QQuaternion.
 * If the series also has rotation, item rotation is multiplied by it.
 * Defaults to no rotation.
 */

/*!
 * \fn QQuaternion QScatterDataItem::rotation() const
 * Returns the rotation of this data item.
 * \sa setRotation()
 */

/*!
 * \fn void QScatterDataItem::setX(float value)
 * Sets the x-coordinate of the item position to the value \a value.
 */

/*!
 * \fn void QScatterDataItem::setY(float value)
 * Sets the y-coordinate of the item position to the value \a value.
 */

/*!
 * \fn void QScatterDataItem::setZ(float value)
 * Sets the z-coordinate of the item position to the value \a value.
 */

/*!
 * \fn float QScatterDataItem::x() const
 * Returns the x-coordinate of the position of this data item.
 */

/*!
 * \fn float QScatterDataItem::y() const
 * Returns the y-coordinate of the position of this data item.
 */

/*!
 * \fn float QScatterDataItem::z() const
 * Returns the z-coordinate of the position of this data item.
 */
