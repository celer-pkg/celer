// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qnearfieldmanager.h"
#include "qnearfieldmanager_p.h"

#if defined(QT_SIMULATOR)
#include "qnearfieldmanager_simulator_p.h"
#elif defined(NEARD_NFC)
#include "qnearfieldmanager_neard_p.h"
#elif defined(ANDROID_NFC)
#include "qnearfieldmanager_android_p.h"
#elif defined(IOS_NFC)
#include "qnearfieldmanager_ios_p.h"
#elif defined(PCSC_NFC)
#include "qnearfieldmanager_pcsc_p.h"
#else
#include "qnearfieldmanager_generic_p.h"
#endif

#include <QtCore/QMetaType>
#include <QtCore/QMetaMethod>

QT_BEGIN_NAMESPACE

/*!
    \class QNearFieldManager
    \brief The QNearFieldManager class provides access to notifications for NFC events.

    \ingroup connectivity-nfc
    \inmodule QtNfc
    \since 5.2

    NFC Forum devices support two modes of communications.  The first mode, peer-to-peer
    communications, is used to communicate between two NFC Forum devices.  The second mode,
    master/slave communications, is used to communicate between an NFC Forum device and an NFC
    Forum Tag or Contactless Card. The targetDetected() signal is emitted when a target device
    enters communications range. Communications can be initiated from the slot connected to this
    signal.

    NFC Forum devices generally operate as the master in master/slave communications. Some devices
    are also capable of operating as the slave, so called Card Emulation mode. In this mode the
    local NFC device emulates a NFC Forum Tag or Contactless Card.

    Applications can connect to the targetDetected() and targetLost() signals to get notified when
    an NFC Forum Tag enters or leaves proximity. Before these signals are emitted target detection
    must be started with the startTargetDetection() function. Target detection can be stopped with
    the stopTargetDetection() function. When the target is no longer required the target should be
    deleted as other applications may be blocked from accessing the target.

    \section3 NFC on Linux
    The \l{https://github.com/linux-nfc/neard}{Linux NFC project} provides software to support NFC
    on Linux platforms. The neard daemon will allow access to the supported hardware via DBus
    interfaces. QtNfc requires neard version 0.14 which can be built from source or installed via
    the appropriate Linux package manager. Not all API features are currently supported.
    To allow QtNfc to access the DBus interfaces the neard daemon has to be running. In case of
    problems debug output can be enabled by enabling categorized logging for 'qt.nfc.neard'.
*/

/*!
    \enum QNearFieldManager::AdapterState

    \since 5.12

    This enum describes the different states a NFC adapter can have.

    \value Offline      The nfc adapter is offline.
    \value TurningOn    The nfc adapter is turning on.
    \value Online       The nfc adapter is online.
    \value TurningOff   The nfc adapter is turning off.
*/

/*!
    \fn void QNearFieldManager::adapterStateChanged(AdapterState state)

    \since 5.12

    This signal is emitted whenever the \a state of the NFC adapter changed.

    \note Currently, this signal is only emitted on Android.
*/

/*!
    \fn void QNearFieldManager::targetDetectionStopped()

    \since 6.2

    This signal is emitted whenever the target detection is stopped.

    \note Mostly this signal is emitted when \l stopTargetDetection() has been called.
    Additionally the user is able to stop the detection on iOS within a popup shown
    by the system during the scan, which also leads to emitting this signal.
*/

/*!
    \fn void QNearFieldManager::targetDetected(QNearFieldTarget *target)

    This signal is emitted whenever a target is detected. The \a target parameter represents the
    detected target.

    This signal will be emitted for all detected targets.

    QNearFieldManager maintains ownership of \a target, however, it will not be destroyed until
    the QNearFieldManager destructor is called. Ownership may be transferred by calling
    setParent().

    Do not delete \a target from the slot connected to this signal, instead call deleteLater().

    \note that if \a target is deleted before it moves out of proximity the targetLost() signal
    will not be emitted.

    \sa targetLost()
*/

/*!
    \fn void QNearFieldManager::targetLost(QNearFieldTarget *target)

    This signal is emitted whenever a target moves out of proximity. The \a target parameter
    represents the lost target.

    Do not delete \a target from the slot connected to this signal, instead use deleteLater().

    \sa QNearFieldTarget::disconnected()
*/

/*!
    Constructs a new near field manager with \a parent.
*/
QNearFieldManager::QNearFieldManager(QObject *parent)
:   QObject(parent), d_ptr(new QNearFieldManagerPrivateImpl)
{
    qRegisterMetaType<AdapterState>();

    connect(d_ptr, &QNearFieldManagerPrivate::adapterStateChanged,
            this, &QNearFieldManager::adapterStateChanged);
    connect(d_ptr, &QNearFieldManagerPrivate::targetDetectionStopped,
            this, &QNearFieldManager::targetDetectionStopped);
    connect(d_ptr, &QNearFieldManagerPrivate::targetDetected,
            this, &QNearFieldManager::targetDetected);
    connect(d_ptr, &QNearFieldManagerPrivate::targetLost,
            this, &QNearFieldManager::targetLost);
}

/*!
    \internal

    Constructs a new near field manager with the specified \a backend and with \a parent.

    \note: This constructor is only enable for internal builds and is used for testing the
    simulator backend.
*/
QNearFieldManager::QNearFieldManager(QNearFieldManagerPrivate *backend, QObject *parent)
:   QObject(parent), d_ptr(backend)
{
    qRegisterMetaType<AdapterState>();

    connect(d_ptr, &QNearFieldManagerPrivate::adapterStateChanged,
            this, &QNearFieldManager::adapterStateChanged);
    connect(d_ptr, &QNearFieldManagerPrivate::targetDetectionStopped,
            this, &QNearFieldManager::targetDetectionStopped);
    connect(d_ptr, &QNearFieldManagerPrivate::targetDetected,
            this, &QNearFieldManager::targetDetected);
    connect(d_ptr, &QNearFieldManagerPrivate::targetLost,
            this, &QNearFieldManager::targetLost);
}

/*!
    Destroys the near field manager.
*/
QNearFieldManager::~QNearFieldManager()
{
    delete d_ptr;
}

/*!
    \since 6.2

    Returns \c true if the device has a NFC adapter and
    it is turned on; otherwise returns \c false.

    \sa isSupported()
*/
bool QNearFieldManager::isEnabled() const
{
    Q_D(const QNearFieldManager);

    return d->isEnabled();
}

/*!
    \since 5.12

    Returns \c true if the underlying device has a NFC adapter; otherwise
    returns \c false. If an \a accessMethod is given, the function returns
    \c true only if the NFC adapter supports the given \a accessMethod.

    \sa isEnabled()
*/
bool QNearFieldManager::isSupported(QNearFieldTarget::AccessMethod accessMethod) const
{
    Q_D(const QNearFieldManager);

    return d->isSupported(accessMethod);
}

/*!
    \fn bool QNearFieldManager::startTargetDetection(QNearFieldTarget::AccessMethod accessMethod)

    Starts detecting targets and returns \c true if target detection started successfully;
    otherwise returns \c false. Causes the targetDetected() signal to be emitted
    when a target is within proximity. Only tags with the given \a accessMethod will be reported.
    Target detection continues until \l stopTargetDetection() is called.

    To detect targets with a different \a accessMethod, stopTargetDetection() must be called first.

    \note On iOS, it is impossible to start target detection for both NdefAccess and TagTypeSpecificAccess
    at the same time. So if AnyAccess is selected, NdefAccess will be used instead.

    \note On platforms using neard, target detection will stop as soon as a tag has been detected.

    \sa stopTargetDetection()
*/
bool QNearFieldManager::startTargetDetection(QNearFieldTarget::AccessMethod accessMethod)
{
    Q_D(QNearFieldManager);

    return d->startTargetDetection(accessMethod);
}

/*!
    Stops detecting targets. The \l targetDetected() signal will no longer be emitted until another
    call to \l startTargetDetection() is made. Targets detected before are still valid.

    \note On iOS, detected targets become invalid after this call (e.g. an attempt to write or
    read NDEF messages will result in an error).

    If an \a errorMessage is provided, it is a hint to the system that the application's goal
    was not achieved. The \a errorMessage and a matching error icon are shown to the user.
    Calling this function with an empty \a errorMessage implies a successful end of operation;
    otherwise, an \a errorMessage should be passed to this function.

    \note Currently, \a errorMessage only has an effect on iOS because the system shows a popup
    during the scan where the \a errorMessage is visible. Other platforms will ignore this
    parameter.

    \sa setUserInformation()
*/
void QNearFieldManager::stopTargetDetection(const QString &errorMessage)
{
    Q_D(QNearFieldManager);

    d->stopTargetDetection(errorMessage);
}

/*!
    \since 6.2

    Sets the message that the system shows to the user. If target detection is running, the
    \a message will be updated immediately and can be used as a progress message. The last message
    set before a call to \l startTargetDetection() without an error message is used as a success
    message. If target detection is not running, the \a message will be used as the initial
    message when the next detection is started. By default, no message is shown to the user.

    \note Currently, this function only has an effect on iOS because the system shows a popup
    during the scan. On iOS, this \a message is mapped to the alert message which is shown upon
    successful completion of the scan. Other platforms will ignore \a message.

    \sa startTargetDetection(), stopTargetDetection()
*/
void QNearFieldManager::setUserInformation(const QString &message)
{
    Q_D(QNearFieldManager);

    d->setUserInformation(message);
}

QT_END_NAMESPACE

#include "moc_qnearfieldmanager_p.cpp"

#include "moc_qnearfieldmanager.cpp"
