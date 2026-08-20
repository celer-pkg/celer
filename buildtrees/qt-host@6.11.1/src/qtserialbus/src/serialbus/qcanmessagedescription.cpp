// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcanmessagedescription.h"
#include "qcanmessagedescription_p.h"
#include "qcansignaldescription.h"

#include <QtCore/QHash>
#include <QtCore/QSharedData>

QT_BEGIN_NAMESPACE

/*!
    \class QCanMessageDescription
    \inmodule QtSerialBus
    \since 6.5
    \preliminary

    \brief The QCanMessageDescription class describes the rules to process a CAN
    message and represent it in an application-defined format.

    A CAN message is basically a \l QCanBusFrame. The description of a CAN
    message includes the following:
    \list
        \li Message ID.
        \li Message name.
        \li Message length in bytes.
        \li Source of the message (transmitter).
        \li Description of signals in the message.
    \endlist

    The QCanMessageDescription class provides methods to control all those
    parameters.

    \section2 Message ID
    The message ID is a unique identifier, which is used to select the proper
    message description when decoding the incoming \l QCanBusFrame or encoding
    a \l QCanBusFrame based on the provided data.

    See \l QCanUniqueIdDescription documentation for more details on the unique
    identifier description.

    \section2 Signal Description
    The signal description is represented by the \l QCanSignalDescription
    class. The QCanMessageDescription class only provides a list of signals that
    belong to the message.

    \sa QCanSignalDescription, QCanUniqueIdDescription
*/

/*!
    Creates an empty message description.
*/
QCanMessageDescription::QCanMessageDescription() : d(new QCanMessageDescriptionPrivate)
{
}

/*!
    Creates a message description with the values copied from \a other.
*/
QCanMessageDescription::QCanMessageDescription(const QCanMessageDescription &other) : d(other.d)
{
}

/*!
    \fn QCanMessageDescription::QCanMessageDescription(QCanMessageDescription &&other) noexcept

    Creates a message description by moving from \a other.

    \note The moved-from QCanMessageDescription object can only be destroyed or
    assigned to. The effect of calling other functions than the destructor or
    one of the assignment operators is undefined.
*/

/*!
    \fn QCanMessageDescription::~QCanMessageDescription()

    Destroys this message description.
*/

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCanMessageDescriptionPrivate)

/*!
    Assigns the values from \a other to this message description.
*/
QCanMessageDescription &QCanMessageDescription::operator=(const QCanMessageDescription &other)
{
    d = other.d;
    return *this;
}

/*!
    \fn QCanMessageDescription &QCanMessageDescription::operator=(QCanMessageDescription &&other) noexcept

    Move-assigns the values from \a other to this message description.

    \note The moved-from QCanMessageDescription object can only be destroyed or
    assigned to. The effect of calling other functions than the destructor or
    one of the assignment operators is undefined.
*/

/*!
    Returns \c true when the message description is valid and \c false
    otherwise.

    A valid message description \e must have at least one signal description.
    All signal descriptions \e must be valid as well.

    \sa signalDescriptions(), QCanSignalDescription::isValid()
*/
bool QCanMessageDescription::isValid() const
{
    if (d->messageSignals.isEmpty())
        return false;

    for (const auto &sigDesc : d->messageSignals) {
        if (!sigDesc.isValid())
            return false;
    }

    return true;
}

/*!
    Returns the unique identifier of the CAN message.

    See the \l {Message ID} section for more information about the unique
    identifier.

    \sa setUniqueId()
*/
QtCanBus::UniqueId QCanMessageDescription::uniqueId() const
{
    return d->id;
}

/*!
    Sets the unique identifier of the CAN message to \a id.

    See the \l {Message ID} section for more information about the unique
    identifier.

    \sa uniqueId()
*/
void QCanMessageDescription::setUniqueId(QtCanBus::UniqueId id)
{
    d.detach();
    d->id = id;
}

/*!
    Returns the name of the CAN message.

//! [qcanmessagedesc-aux-parameter]
    This parameter is introduced only for extra description. It's not used
    during message encoding or decoding.
//! [qcanmessagedesc-aux-parameter]

    \sa setName()
*/
QString QCanMessageDescription::name() const
{
    return d->name;
}

/*!
    Sets the name of the CAN message to \a name.

    \include qcanmessagedescription.cpp qcanmessagedesc-aux-parameter

    \sa name()
*/
void QCanMessageDescription::setName(const QString &name)
{
    d.detach();
    d->name = name;
}

/*!
    Returns the size in bytes of the CAN message.

    \sa setSize()
*/
quint8 QCanMessageDescription::size() const
{
    return d->size;
}

/*!
    Sets the size in bytes of the CAN message to \a size.

    \sa size()
*/
void QCanMessageDescription::setSize(quint8 size)
{
    d.detach();
    d->size = size;
}

/*!
    Returns the transmitter node of the message.

    \include qcanmessagedescription.cpp qcanmessagedesc-aux-parameter

    \sa setTransmitter()
*/
QString QCanMessageDescription::transmitter() const
{
    return d->transmitter;
}

/*!
    Sets the transmitter node of the message to \a transmitter.

    \include qcanmessagedescription.cpp qcanmessagedesc-aux-parameter

    \sa transmitter()
*/
void QCanMessageDescription::setTransmitter(const QString &transmitter)
{
    d.detach();
    d->transmitter = transmitter;
}

/*!
    Returns the comment for the message.

    \include qcanmessagedescription.cpp qcanmessagedesc-aux-parameter

    \sa setComment()
*/
QString QCanMessageDescription::comment() const
{
    return d->comment;
}

/*!
    Sets the comment for the message to \a text.

    \include qcanmessagedescription.cpp qcanmessagedesc-aux-parameter

    \sa comment()
*/
void QCanMessageDescription::setComment(const QString &text)
{
    d.detach();
    d->comment = text;
}

/*!
    Returns the list of signal descriptions that belong to this message
    description.

    \sa signalDescriptionForName(), addSignalDescription(),
    setSignalDescriptions(), clearSignalDescriptions()
*/
QList<QCanSignalDescription> QCanMessageDescription::signalDescriptions() const
{
    return QList<QCanSignalDescription>(d->messageSignals.cbegin(), d->messageSignals.cend());
}

/*!
    Returns the signal description of a signal with the name \a name.

    If the message description does not have such signal description, a
    default-constructed \l QCanSignalDescription object is returned.

    \sa signalDescriptions(), addSignalDescription(), setSignalDescriptions(),
    clearSignalDescriptions()
*/
QCanSignalDescription QCanMessageDescription::signalDescriptionForName(const QString &name) const
{
    return d->messageSignals.value(name);
}

/*!
    Clears all the signal descriptions of this message.

    \sa signalDescriptions(), signalDescriptionForName(),
    addSignalDescription(), setSignalDescriptions()
*/
void QCanMessageDescription::clearSignalDescriptions()
{
    d.detach();
    d->messageSignals.clear();
}

/*!
    Adds a new signal description \a description to this message description.

    If the message description already has a signal description for a signal
    with the same name, it is overwritten.

    \sa signalDescriptions(), signalDescriptionForName(),
    setSignalDescriptions(), clearSignalDescriptions()
*/
void QCanMessageDescription::addSignalDescription(const QCanSignalDescription &description)
{
    d.detach();
    d->messageSignals.insert(description.name(), description);
}

/*!
    Sets the descriptions of the signals belonging to this message description
    to \a descriptions.

    \note Message description \e must have signal descriptions with unique
    signal names, so if the \a descriptions list contains entries with
    duplicated names, only the last entry will be added.

    \sa signalDescriptions(), signalDescriptionForName(),
    addSignalDescription(), clearSignalDescriptions()
*/
void QCanMessageDescription::setSignalDescriptions(const QList<QCanSignalDescription> &descriptions)
{
    d.detach();
    d->messageSignals.clear();
    d->messageSignals.reserve(descriptions.size());
    for (const auto &desc : descriptions)
        d->messageSignals.insert(desc.name(), desc);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug QCanMessageDescription::debugStreaming(QDebug dbg, const QCanMessageDescription &msg)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "QCanMessageDescription(" << msg.name() << ", ID = " << msg.uniqueId()
                  << ", Size = " << msg.size();
    if (!msg.transmitter().isEmpty())
        dbg << ", Transmitter = " << msg.transmitter();
    if (!msg.comment().isEmpty())
        dbg << ", Comment = " << msg.comment();
    const auto msgSignals = msg.signalDescriptions();
    if (!msgSignals.isEmpty()) {
        dbg << ", Signals: {";
        bool first = true;
        for (const auto &sig : msgSignals) {
            if (!first)
                dbg << ", ";
            dbg << sig;
            first = false;
        }
        dbg << "}";
    }
    dbg << ")";
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

QCanMessageDescriptionPrivate *QCanMessageDescriptionPrivate::get(const QCanMessageDescription &desc)
{
    return desc.d.data();
}

QT_END_NAMESPACE
