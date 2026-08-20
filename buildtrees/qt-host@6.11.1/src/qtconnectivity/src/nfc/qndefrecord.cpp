// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#include "qndefrecord.h"
#include "qndefrecord_p.h"

#include <QtCore/QHash>

QT_BEGIN_NAMESPACE

/*!
    \class QNdefRecord
    \brief The QNdefRecord class provides an NFC NDEF record.

    \ingroup connectivity-nfc
    \inmodule QtNfc
    \since 5.2

    QNdefRecord and derived classes are used to parse the contents of
    \l {QNdefMessage}{NDEF messages} and create new NDEF messages.

    Use typeNameFormat() and setTypeNameFormat() to
    get and set the type name format of the NDEF record.

    Use type() and setType() to get and set the type of the NDEF record.

    Use id() and setId() to get and set the id of the NDEF record.

    Use payload() and setPayload() to get and set the NDEF record payload.  isEmpty() can be used
    to test if the payload is empty.

    QNdefRecord is an implicitly shared class.  This means you can efficiently convert between
    QNdefRecord and specialized record classes.  The isRecordType() template function can be used
    to test if a conversion is possible.  The following example shows how to test if a QNdefRecord
    is an NFC RTD Text record and extract the text information from it.

    \snippet nfc.cpp Record conversion

    \section1 Creating Specialized NDEF Record Classes

    Specialized NDEF record classes can be easily created with the Q_DECLARE_NDEF_RECORD() and
    Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD() macros.  The following example shows the class
    declaration of the hypothetical \e {example.com:f} record type that encapsulates a single int
    property foo.

    \snippet nfc.cpp Specialized class definition

    The developer only needs to provide implementations for the \c {foo()} and \c {setFoo()}
    functions that parse and set the contents of the NDEF record's payload.
*/

/*!
    \enum QNdefRecord::TypeNameFormat

    This enum describes the type name format of an NDEF record.

    \value Empty        An empty NDEF record, the record does not contain a payload
    \value NfcRtd       The NDEF record type is defined by an NFC RTD Specification
    \value Mime         The NDEF record type follows the construct described in RFC 2046
    \value Uri          The NDEF record type follows the construct described in RFC 3986
    \value ExternalRtd  The NDEF record type follows the construct for external type names
                        described the NFC RTD Specification
    \value Unknown      The type of the record is unknown and should be treated similar to content
                        with MIME type 'application/octet-stream' without further context
*/

/*!
    \fn template <typename T> bool QNdefRecord::isRecordType() const

    Returns \c true if the NDEF record is of the specified record type;
    otherwise returns \c false.
*/

/*!
    \fn bool QNdefRecord::operator!=(const QNdefRecord &other) const

    Returns \c true if this NDEF record does not equal \a other;
    otherwise return \c false.
*/

/*!
    \macro Q_DECLARE_NDEF_RECORD(className, typeNameFormat, type, initialPayload)
    \relates QNdefRecord

    This macro declares default and copy constructors for specialized NDEF record classes.

    \a className is the name of the specialized class, \a typeNameFormat is the appropriate
    QNdefRecord::TypeNameFormat for the custom type and \a type is the type without the NID or NSS
    prefixes. That is \e {example.com:f} not \e {urn:nfc:ext:example.com:f}.  \a initialPayload
    is the initial payload of an empty record, it must be a QByteArray or a type that can be
    implicitly converted into a QByteArray.

    See the section on \l {Creating specialized NDEF record classes} for details.

    \sa Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD()
*/

/*!
    \macro Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(className, typeNameFormat, type)
    \relates QNdefRecord

    This macro declares a template specialization for the QNdefRecord::isRecordType() function.

    This macro should be used in the header file directly after the definition of a specialized
    NDEF record class.

    \a className is the name of the specialized class, \a typeNameFormat is the appropriate
    QNdefRecord::TypeNameFormat for the custom type and \a type is the type without the NID or NSS
    prefixes.  That is \e {example.com:f} not \e {urn:nfc:ext:example.com:f}.

    See the section on \l {Creating specialized NDEF record classes} for details.

    \sa Q_DECLARE_NDEF_RECORD()
*/

size_t qHash(const QNdefRecord &key)
{
    return qHash(key.type() + key.id() + key.payload());
}

/*!
    Constructs a new empty NDEF record.
*/
QNdefRecord::QNdefRecord()
{
}

/*!
    Constructs a new NDEF record that is a copy of \a other.
*/
QNdefRecord::QNdefRecord(const QNdefRecord &other)
{
    d = other.d;
}

/*!
    \internal

    Constructs an NDEF record that is a copy of \a other if \a other is of the expected type name
    format identified by \a typeNameFormat and type as identified by \a type; otherwise an empty
    NDEF record of the expected type name format and type is created.
*/
QNdefRecord::QNdefRecord(const QNdefRecord &other, TypeNameFormat typeNameFormat,
                         const QByteArray &type)
{
    if (other.d->typeNameFormat == quint8(typeNameFormat) && other.d->type == type) {
        d = other.d;
    } else {
        d = new QNdefRecordPrivate;
        d->typeNameFormat = typeNameFormat;
        d->type = type;
    }
}

/*!
    \internal

    Constructs an NDEF record that is a copy of \a other if \a other is of the expected type name
    format identified by \a typeNameFormat; otherwise an empty NDEF record of the expected type
    name format and type is created.
*/
QNdefRecord::QNdefRecord(const QNdefRecord &other, TypeNameFormat typeNameFormat)
{
    if (other.d->typeNameFormat == quint8(typeNameFormat)) {
        d = other.d;
    } else {
        d = new QNdefRecordPrivate;
        d->typeNameFormat = typeNameFormat;
    }
}

/*!
    \internal

    Constructs an NDEF record with a type name format identified by \a typeNameFormat and type as
    identified by \a type.
*/
QNdefRecord::QNdefRecord(TypeNameFormat typeNameFormat, const QByteArray &type)
:   d(new QNdefRecordPrivate)
{
    d->typeNameFormat = typeNameFormat;
    d->type = type;
}

/*!
    Destroys the NDEF record.
*/
QNdefRecord::~QNdefRecord()
{
}

/*!
    Assigns this NDEF record to \a other.
*/
QNdefRecord &QNdefRecord::operator=(const QNdefRecord &other)
{
    if (this != &other)
        d = other.d;

    return *this;
}

/*!
    Sets the type name format of the NDEF record to \a typeNameFormat.
*/
void QNdefRecord::setTypeNameFormat(TypeNameFormat typeNameFormat)
{
    if (!d)
        d = new QNdefRecordPrivate;

    d->typeNameFormat = typeNameFormat;
}

/*!
    Returns the type name format of the NDEF record.
*/
QNdefRecord::TypeNameFormat QNdefRecord::typeNameFormat() const
{
    if (!d)
        return Empty;

    if (d->typeNameFormat > 0x05)
        return Unknown;

    return TypeNameFormat(d->typeNameFormat);
}

/*!
    Sets the type of the NDEF record to \a type.
*/
void QNdefRecord::setType(const QByteArray &type)
{
    if (!d)
        d = new QNdefRecordPrivate;

    d->type = type;
}

/*!
    Returns the type of the NDEF record.
*/
QByteArray QNdefRecord::type() const
{
    if (!d)
        return QByteArray();

    return d->type;
}

/*!
    Sets the id of the NDEF record to \a id.
*/
void QNdefRecord::setId(const QByteArray &id)
{
    if (!d)
        d = new QNdefRecordPrivate;

    d->id = id;
}

/*!
    Returns the id of the NDEF record.
*/
QByteArray QNdefRecord::id() const
{
    if (!d)
        return QByteArray();

    return d->id;
}

/*!
    Sets the payload of the NDEF record to \a payload.
*/
void QNdefRecord::setPayload(const QByteArray &payload)
{
    if (!d)
        d = new QNdefRecordPrivate;

    d->payload = payload;
}

/*!
    Returns the payload of the NDEF record.
*/
QByteArray QNdefRecord::payload() const
{
    if (!d)
        return QByteArray();

    return d->payload;
}

/*!
    Returns \c true if the NDEF record contains an empty payload;
    otherwise returns \c false.

    This is equivalent to calling \c {payload().isEmpty()}.
*/
bool QNdefRecord::isEmpty() const
{
    if (!d)
        return true;

    return d->payload.isEmpty();
}

/*!
    Returns \c true if \a other and this NDEF record are the same.
    Otherwise returns \c false.
*/
bool QNdefRecord::operator==(const QNdefRecord &other) const
{
    if (d == other.d)
        return true;

    if (!d || !other.d)
        return false;

    if (d->typeNameFormat != other.d->typeNameFormat)
        return false;

    if (d->type != other.d->type)
        return false;

    if (d->id != other.d->id)
        return false;

    if (d->payload != other.d->payload)
        return false;

    return true;
}

/*!
    \since 6.2

    Clear the NDEF record.

    An \l isEmpty() call returns \c true for a cleared record. The record
    \l type() is set to \l {QNdefRecord::}{Empty}.
*/
void QNdefRecord::clear()
{
    if (d) {
        d->typeNameFormat = 0;
        d->type.clear();
        d->id.clear();
        d->payload.clear();
    }
}

QT_END_NAMESPACE
