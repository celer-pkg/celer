// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtProtobuf/qprotobufmessage.h>
#include <QtProtobuf/qprotobufregistration.h>
#include <QtProtobuf/qprotobufrepeatediterator.h>

/*!
    \class QProtobufRepeatedIterator
    \inmodule QtProtobuf
    \since 6.8

    \brief Allows iterating over repeated protobuf types.

    Allows iterating over repeated protobuf types and access the repeated field elements as
    reference to QProtobufMessage.
*/

/*!
    \fn QProtobufRepeatedIterator::~QProtobufRepeatedIterator()
    Destroys iterator.
*/

/*!
    \fn QProtobufRepeatedIterator::QProtobufRepeatedIterator(QProtobufRepeatedIterator &&other) noexcept

    Move-constructs a new QProtobufRepeatedIterator from \a other.
*/

/*!
    \fn QProtobufRepeatedIterator &QProtobufRepeatedIterator::operator=(QProtobufRepeatedIterator &&other) noexcept

    Move-assigns \a other to this QProtobufRepeatedIterator and returns a reference to
    the updated object.
*/

/*!
    \fn bool QProtobufRepeatedIterator::isValid() const noexcept

    Returns \c true if the iterator points to a valid data object.
*/

/*!
    \fn bool QProtobufRepeatedIterator::hasNext() const noexcept

    Returns \c true if the iterator can read the next element from the repeated field.
*/

/*!
    \fn QProtobufMessage *QProtobufRepeatedIterator::next()

    Returns the next element under from the repeated field.
*/

/*!
    \fn QProtobufMessage *QProtobufRepeatedIterator::addNext()

    Returns a new temporary element in the repeated field.
*/

/*!
    \fn void QProtobufRepeatedIterator::push()

    Adds the element, created by addNext function, to the repeated field.
*/

/*!
    \fn template<typename T> static QProtobufRepeatedIterator fromList(QList<T> &list)

    Creates a QProtobufRepeatedIterator instance from a \a list.
*/

/*!
    \fn template<typename K, typename V> static QProtobufRepeatedIterator fromHash(QHash<K, V> &hash)

    Creates a QProtobufRepeatedIterator instance from a \a hash.
*/
