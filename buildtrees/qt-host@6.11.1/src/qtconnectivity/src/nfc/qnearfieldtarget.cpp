// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qnearfieldtarget.h"
#include "qnearfieldtarget_p.h"
#include "qndefmessage.h"

#include <QtCore/QString>
#include <QtCore/QUrl>

#include <QtCore/QDebug>

#include <QCoreApplication>

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN_TAGGED(QNearFieldTarget::RequestId, QNearFieldTarget__RequestId)

/*!
    \class QNearFieldTarget
    \brief The QNearFieldTarget class provides an interface for communicating with a target
           device.

    \ingroup connectivity-nfc
    \inmodule QtNfc
    \since 5.2

    QNearFieldTarget provides a generic interface for communicating with an NFC target device.
    Both NFC Forum devices and NFC Forum Tag targets are supported by this class.  All target
    specific classes subclass this class.

    The type() function can be used to get the type of the target device.  The uid() function
    returns the unique identifier of the target.  The AccessMethods flags returns from the
    accessMethods() function can be tested to determine which access methods are supported by the
    target.

    If the target supports NdefAccess, hasNdefMessage() can be called to test if the target has a
    stored NDEF message, readNdefMessages() and writeNdefMessages() functions can be used to get
    and set the NDEF message.

    If the target supports TagTypeSpecificAccess, sendCommand() can be used to send a single
    proprietary command to the target and retrieve the response.
*/

/*!
    \enum QNearFieldTarget::Type

    This enum describes the type of tag the target is detected as.

    \value ProprietaryTag   An unidentified proprietary target tag.
    \value NfcTagType1      An NFC tag type 1 target.
    \value NfcTagType2      An NFC tag type 2 target.
    \value NfcTagType3      An NFC tag type 3 target.
    \value NfcTagType4      An NFC tag type 4 target. This value is used if the NfcTagType4
                            cannot be further refined by NfcTagType4A or NfcTagType4B below.
    \value NfcTagType4A     An NFC tag type 4 target based on ISO/IEC 14443-3A.
    \value NfcTagType4B     An NFC tag type 4 target based on ISO/IEC 14443-3B.
    \value MifareTag        A Mifare target.
*/

/*!
    \enum QNearFieldTarget::AccessMethod

    This enum describes the access methods a near field target supports.

    \value UnknownAccess            The target supports an unknown access type.
    \value NdefAccess               The target supports reading and writing NDEF messages using
                                    readNdefMessages() and writeNdefMessages().
    \value TagTypeSpecificAccess    The target supports sending tag type specific commands using
                                    sendCommand().
    \value AnyAccess                The target supports any of the known access types.
*/

/*!
    \enum QNearFieldTarget::Error

    This enum describes the error codes that a near field target reports.

    \value NoError                  No error has occurred.
    \value UnknownError             An unidentified error occurred.
    \value UnsupportedError         The requested operation is unsupported by this near field
                                    target.
    \value TargetOutOfRangeError    The target is no longer within range.
    \value NoResponseError          The target did not respond.
    \value ChecksumMismatchError    The checksum has detected a corrupted response.
    \value InvalidParametersError   Invalid parameters were passed to a tag type specific function.
    \value ConnectionError          Failed to connect to the target.
    \value NdefReadError            Failed to read NDEF messages from the target.
    \value NdefWriteError           Failed to write NDEF messages to the target.
    \value CommandError             Failed to send a command to the target.
    \value TimeoutError             The request could not be completed within the time
                                    specified in waitForRequestCompleted().
    \value UnsupportedTargetError   The target used is unsupported. As example this can occur on missing
                                    required entitlement and/or privacy settings from the client app.
*/

/*!
    \fn void QNearFieldTarget::disconnected()

    This signal is emitted when the near field target moves out of proximity.
*/

/*!
    \fn void QNearFieldTarget::ndefMessageRead(const QNdefMessage &message)

    This signal is emitted when a complete NDEF \a message has been read from the target.

    \sa readNdefMessages()
*/

/*!
    \fn void QNearFieldTarget::requestCompleted(const QNearFieldTarget::RequestId &id)

    This signal is emitted when a request \a id completes.

    \sa sendCommand()
*/

/*!
    \fn void QNearFieldTarget::error(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id)

    This signal is emitted when an error occurs while processing request \a id. The \a error
    parameter describes the error.
*/

/*!
    \class QNearFieldTarget::RequestId
    \inmodule QtNfc
    \inheaderfile QNearFieldTarget
    \brief A request id handle.
*/

/*!
    Constructs a new invalid request id handle.
*/
QNearFieldTarget::RequestId::RequestId()
{
}

/*!
    Constructs a new request id handle that is a copy of \a other.
*/
QNearFieldTarget::RequestId::RequestId(const RequestId &other)
:   d(other.d)
{
}

/*!
    \internal
*/
QNearFieldTarget::RequestId::RequestId(RequestIdPrivate *p)
:   d(p)
{
}

/*!
    Destroys the request id handle.
*/
QNearFieldTarget::RequestId::~RequestId()
{
}

/*!
    Returns \c true if this is a valid request id; otherwise returns \c false.
*/
bool QNearFieldTarget::RequestId::isValid() const
{
    return d;
}

/*!
    Returns the current reference count of the request id.
*/
int QNearFieldTarget::RequestId::refCount() const
{
    if (d)
        return d->ref.loadRelaxed();

    return 0;
}

/*!
    \internal
*/
bool QNearFieldTarget::RequestId::operator<(const RequestId &other) const
{
    return std::less<const RequestIdPrivate*>()(d.constData(), other.d.constData());
}

/*!
    \internal
*/
bool QNearFieldTarget::RequestId::operator==(const RequestId &other) const
{
    return d == other.d;
}

/*!
    \internal
*/
bool QNearFieldTarget::RequestId::operator!=(const RequestId &other) const
{
    return d != other.d;
}

/*!
    Assigns a copy of \a other to this request id and returns a reference to this request id.
*/
QNearFieldTarget::RequestId &QNearFieldTarget::RequestId::operator=(const RequestId &other)
{
    d = other.d;
    return *this;
}

/*!
    Constructs a new near field target with \a parent.
*/
QNearFieldTarget::QNearFieldTarget(QObject *parent)
:   QNearFieldTarget(new QNearFieldTargetPrivate(this), parent)
{
}

/*!
    Destroys the near field target.
*/
QNearFieldTarget::~QNearFieldTarget()
{
    Q_D(QNearFieldTarget);

    d->disconnect();
}

/*!
    Returns the UID of the near field target.

    \note On iOS, this function returns an empty QByteArray for
    a near field target discovered using NdefAccess method.

    \sa QNearFieldTarget::AccessMethod
*/
QByteArray QNearFieldTarget::uid() const
{
    Q_D(const QNearFieldTarget);

    return d->uid();
}

/*!
    Returns the type of tag type of this near field target.
*/
QNearFieldTarget::Type QNearFieldTarget::type() const
{
    Q_D(const QNearFieldTarget);

    return d->type();
}

/*!
    Returns the access methods supported by this near field target.
*/
QNearFieldTarget::AccessMethods QNearFieldTarget::accessMethods() const
{
    Q_D(const QNearFieldTarget);

    return d->accessMethods();
}

/*!
    \since 5.9

    Closes the connection to the target to enable communication with the target
    from a different instance. The connection will also be closed, when the
    QNearFieldTarget is destroyed. A connection to the target device is
    (re)created to process a command or read/write a NDEF messages.

    Returns \c true only if an existing connection was successfully closed;
    otherwise returns \c false.
*/
bool QNearFieldTarget::disconnect()
{
    Q_D(QNearFieldTarget);

    return d->disconnect();
}

/*!
    Returns \c true if at least one NDEF message is stored on the near field
    target; otherwise returns \c false.
*/
bool QNearFieldTarget::hasNdefMessage()
{
    Q_D(QNearFieldTarget);

    return d->hasNdefMessage();
}

/*!
    Starts reading NDEF messages stored on the near field target. Returns a request id which can
    be used to track the completion status of the request. An invalid request id will be returned
    if the target does not support reading NDEF messages.

    An ndefMessageRead() signal will be emitted for each NDEF message. The requestCompleted()
    signal will be emitted when all NDEF messages have been read. The error() signal is emitted if
    an error occurs.

    \note An attempt to read an NDEF message from a tag, that is in INITIALIZED
    state as defined by NFC Forum, will fail with the \l NdefReadError, as the
    tag is formatted to support NDEF but does not contain a message yet.
*/
QNearFieldTarget::RequestId QNearFieldTarget::readNdefMessages()
{
    Q_D(QNearFieldTarget);

    return d->readNdefMessages();
}

/*!
    Writes the NDEF messages in \a messages to the target. Returns a request id which can be used
    to track the completion status of the request. An invalid request id will be returned if the
    target does not support writing NDEF messages.

    The requestCompleted() signal will be emitted when the write operation completes
    successfully; otherwise the error() signal is emitted.
*/
QNearFieldTarget::RequestId QNearFieldTarget::writeNdefMessages(const QList<QNdefMessage> &messages)
{
    Q_D(QNearFieldTarget);

    return d->writeNdefMessages(messages);
}

/*!
    \since 5.9

    Returns the maximum number of bytes that can be sent with sendCommand. 0 will
    be returned if the target does not support sending tag type specific commands.

    \sa sendCommand()
*/
int QNearFieldTarget::maxCommandLength() const
{
    Q_D(const QNearFieldTarget);

    return d->maxCommandLength();
}

/*!
    Sends \a command to the near field target. Returns a request id which can be used to track the
    completion status of the request. An invalid request id will be returned if the target does not
    support sending tag type specific commands.

    The requestCompleted() signal will be emitted on successful completion of the request;
    otherwise the error() signal will be emitted.

    Once the request completes successfully the response can be retrieved from the
    requestResponse() function. The response of this request will be a QByteArray.

    \sa requestCompleted(), waitForRequestCompleted()
*/
QNearFieldTarget::RequestId QNearFieldTarget::sendCommand(const QByteArray &command)
{
    Q_D(QNearFieldTarget);

    return d->sendCommand(command);
}

/*!
    Waits up to \a msecs milliseconds for the request \a id to complete.
    Returns \c true if the request completes successfully and the
    requestCompeted() signal is emitted; otherwise returns \c false.
*/
bool QNearFieldTarget::waitForRequestCompleted(const RequestId &id, int msecs)
{
    Q_D(QNearFieldTarget);

    return d->waitForRequestCompleted(id, msecs);
}

/*!
    Returns the decoded response for request \a id. If the request is unknown or has not yet been
    completed an invalid QVariant is returned.
*/
QVariant QNearFieldTarget::requestResponse(const RequestId &id) const
{
    Q_D(const QNearFieldTarget);

    return d->requestResponse(id);
}

/*!
    \internal
*/
QNearFieldTarget::QNearFieldTarget(QNearFieldTargetPrivate *backend, QObject *parent)
:   QObject(parent), d_ptr(backend)
{
    Q_D(QNearFieldTarget);

    d->q_ptr = this;
    d->setParent(this);

    qRegisterMetaType<QNearFieldTarget::RequestId>();
    qRegisterMetaType<QNearFieldTarget::Error>();
    qRegisterMetaType<QNdefMessage>();

    connect(d, &QNearFieldTargetPrivate::disconnected,
            this, &QNearFieldTarget::disconnected);
    connect(d, &QNearFieldTargetPrivate::ndefMessageRead,
            this, &QNearFieldTarget::ndefMessageRead);
    connect(d, &QNearFieldTargetPrivate::requestCompleted,
            this, &QNearFieldTarget::requestCompleted);
    connect(d, &QNearFieldTargetPrivate::error,
            this, &QNearFieldTarget::error);
}

QT_END_NAMESPACE

#include "moc_qnearfieldtarget.cpp"
