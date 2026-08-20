// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qscatterdataitem_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class QScatterDataItem
 * \inmodule QtDataVisualization
 * \brief The QScatterDataItem class provides a container for resolved data to be added to scatter
 * graphs.
 * \since QtDataVisualization 1.0
 *
 * A scatter data item holds the data for a single rendered item in a scatter
 * graph. Scatter data proxies parse data into QScatterDataItem instances for
 * visualization.
 *
 * \sa QScatterDataProxy, {Qt Data Visualization C++ Classes}
 */

/*!
 * Constructs a scatter data item.
 */
QScatterDataItem::QScatterDataItem()
    : d_ptr(0) // private data doesn't exist by default (optimization)

{
}

/*!
 * Constructs a scatter data item at the position \a position.
 */
QScatterDataItem::QScatterDataItem(const QVector3D &position)
    : d_ptr(0),
      m_position(position)
{
}

/*!
 * Constructs a scatter data item at the position \a position with the rotation
 * \a rotation.
 */
QScatterDataItem::QScatterDataItem(const QVector3D &position, const QQuaternion &rotation)
    : d_ptr(0),
      m_position(position),
      m_rotation(rotation)
{
}

/*!
 * Constructs a copy of \a other.
 */
QScatterDataItem::QScatterDataItem(const QScatterDataItem &other)
{
    operator=(other);
}

/*!
 * Deletes a scatter data item.
 */
QScatterDataItem::~QScatterDataItem()
{
}

/*!
 *  Assigns a copy of \a other to this object.
 */
QScatterDataItem &QScatterDataItem::operator=(const QScatterDataItem &other)
{
    m_position = other.m_position;
    m_rotation = other.m_rotation;

    if (other.d_ptr)
        createExtraData();
    else
        d_ptr = 0;

    return *this;
}

/*!
 * \fn void QScatterDataItem::setPosition(const QVector3D &pos)
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

/*!
 * \internal
 */
void QScatterDataItem::createExtraData()
{
    if (!d_ptr)
        d_ptr = new QScatterDataItemPrivate;
}

QScatterDataItemPrivate::QScatterDataItemPrivate()
{
}

QScatterDataItemPrivate::~QScatterDataItemPrivate()
{
}

QT_END_NAMESPACE
