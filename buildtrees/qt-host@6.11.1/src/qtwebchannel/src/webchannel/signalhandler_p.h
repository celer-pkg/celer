// Copyright (C) 2019 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTWEBCHANNEL_SIGNALHANDLER_P_H
#define QTWEBCHANNEL_SIGNALHANDLER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QHash>
#include <QList>
#include <QMetaMethod>
#include <QDebug>
#include <QThread>

QT_BEGIN_NAMESPACE

static const int s_destroyedSignalIndex = QObject::staticMetaObject.indexOfMethod("destroyed(QObject*)");

/**
 * The signal handler is similar to QSignalSpy, but geared towards the usecase of the web channel.
 *
 * It allows connecting to any number of signals of arbitrary objects and forwards the signal
 * invocations to the Receiver by calling its signalEmitted function, which takes the object,
 * signal index and a QVariantList of arguments.
 */
template<class Receiver>
class SignalHandler : public QObject
{
    Q_DISABLE_COPY(SignalHandler)
public:
    SignalHandler(Receiver *receiver, QObject *parent = 0);

    /**
     * Connect to a signal of @p object identified by @p signalIndex.
     *
     * If the handler is already connected to the signal, an internal counter is increased,
     * i.e. the handler never connects multiple times to the same signal.
     */
    void connectTo(const QObject *object, const int signalIndex);

    /**
     * Decrease the connection counter for the connection to the given signal.
     *
     * When the counter drops to zero, the connection is disconnected.
     */
    void disconnectFrom(const QObject *object, const int signalIndex);

    /**
     * @internal
     *
     * Custom implementation of qt_metacall which calls dispatch() for connected signals.
     */
    int qt_metacall(QMetaObject::Call call, int methodId, void **args) override;

    /**
     * Fully remove and disconnect an object from handler
     */
    void remove(const QObject *object);

private:
    /**
     * Exctract the arguments of a signal call and pass them to the receiver.
     *
     * The @p argumentData is converted to a QVariantList and then passed to the receiver's
     * signalEmitted method.
     */
    void dispatch(const QObject *object, const int signalIdx, void **argumentData);

    void setupSignalArgumentTypes(const QMetaObject *metaObject, const QMetaMethod &signal);

    Receiver *m_receiver;

    // maps meta object -> signalIndex -> list of arguments
    // NOTE: This data is "leaked" on disconnect until deletion of the handler, is this a problem?
    typedef QList<int> ArgumentTypeList;
    typedef QHash<int, ArgumentTypeList> SignalArgumentHash;
    QHash<const QMetaObject *, SignalArgumentHash > m_signalArgumentTypes;

    /*
     * Tracks how many connections are active to object signals.
     *
     * Maps object -> signalIndex -> pair of connection and number of connections
     *
     * Note that the handler is connected to the signal only once, whereas clients
     * may have connected multiple times.
     *
     * TODO: Move more of this logic to the HTML client side, esp. the connection counting.
     */
    typedef QPair<QMetaObject::Connection, int> ConnectionPair;
    typedef QHash<int, ConnectionPair> SignalConnectionHash;
    typedef QHash<const QObject*, SignalConnectionHash> ConnectionHash;
    ConnectionHash m_connectionsCounter;
};

template<class Receiver>
SignalHandler<Receiver>::SignalHandler(Receiver *receiver, QObject *parent)
    : QObject(parent)
    , m_receiver(receiver)
{
    // we must know the arguments of a destroyed signal for the global static meta object of QObject
    // otherwise, we might end up with missing m_signalArgumentTypes information in dispatch
    setupSignalArgumentTypes(&QObject::staticMetaObject, QObject::staticMetaObject.method(s_destroyedSignalIndex));
}

/**
 * Find and return the signal of index @p signalIndex in the meta object of @p object and return it.
 *
 * The return value is also verified to ensure it is a signal.
 */
inline QMetaMethod findSignal(const QMetaObject *metaObject, const int signalIndex)
{
    QMetaMethod signal = metaObject->method(signalIndex);
    if (!signal.isValid()) {
        qWarning("Cannot find signal with index %d of object %s", signalIndex, metaObject->className());
        return QMetaMethod();
    }
    Q_ASSERT(signal.methodType() == QMetaMethod::Signal);
    return signal;
}

template<class Receiver>
void SignalHandler<Receiver>::connectTo(const QObject *object, const int signalIndex)
{
    const QMetaObject *metaObject = object->metaObject();
    const QMetaMethod &signal = findSignal(metaObject, signalIndex);
    if (!signal.isValid()) {
        return;
    }

    ConnectionPair &connectionCounter = m_connectionsCounter[object][signalIndex];
    if (connectionCounter.first) {
        // increase connection counter if already connected
        ++connectionCounter.second;
        return;
    } // otherwise not yet connected, do so now

    static const int memberOffset = QObject::staticMetaObject.methodCount();
    QMetaObject::Connection connection = QMetaObject::connect(object, signal.methodIndex(), this, memberOffset + signalIndex, Qt::AutoConnection, 0);
    if (!connection) {
        qWarning() << "SignalHandler: QMetaObject::connect returned false. Unable to connect to" << object << signal.name() << signal.methodSignature();
        return;
    }
    connectionCounter.first = std::move(connection);
    connectionCounter.second = 1;

    setupSignalArgumentTypes(metaObject, signal);
}

template<class Receiver>
void SignalHandler<Receiver>::setupSignalArgumentTypes(const QMetaObject *metaObject, const QMetaMethod &signal)
{
    if (m_signalArgumentTypes.value(metaObject).contains(signal.methodIndex())) {
        return;
    }
    // find the type ids of the signal parameters, see also QSignalSpy::initArgs
    QList<int> args;
    args.reserve(signal.parameterCount());
    for (int i = 0; i < signal.parameterCount(); ++i) {
        int tp = signal.parameterType(i);
        if (tp == QMetaType::UnknownType) {
            qWarning("Don't know how to handle '%s', use qRegisterMetaType to register it.",
                    signal.parameterNames().at(i).constData());
        }
        args << tp;
    }

    m_signalArgumentTypes[metaObject][signal.methodIndex()] = args;
}

template<class Receiver>
void SignalHandler<Receiver>::dispatch(const QObject *object, const int signalIdx, void **argumentData)
{
    Q_ASSERT(m_signalArgumentTypes.contains(object->metaObject()));
    const QHash<int, QList<int>> &objectSignalArgumentTypes = m_signalArgumentTypes.value(object->metaObject());
    QHash<int, QList<int>>::const_iterator signalIt = objectSignalArgumentTypes.constFind(signalIdx);
    if (signalIt == objectSignalArgumentTypes.constEnd()) {
        // not connected to this signal, skip
        return;
    }
    const QList<int> &argumentTypes = *signalIt;
    QVariantList arguments;
    arguments.reserve(argumentTypes.size());
    // TODO: basic overload resolution based on number of arguments?
    for (int i = 0; i < argumentTypes.size(); ++i) {
        const QMetaType::Type type = static_cast<QMetaType::Type>(argumentTypes.at(i));
        QVariant arg;
        if (type == QMetaType::QVariant) {
            arg = *reinterpret_cast<QVariant *>(argumentData[i + 1]);
        } else {
            arg = QVariant(QMetaType(type), argumentData[i + 1]);
        }
        arguments.append(arg);
    }
    m_receiver->signalEmitted(object, signalIdx, arguments);
}

template<class Receiver>
void SignalHandler<Receiver>::disconnectFrom(const QObject *object, const int signalIndex)
{
    Q_ASSERT(m_connectionsCounter.value(object).contains(signalIndex));
    ConnectionPair &connection = m_connectionsCounter[object][signalIndex];
    --connection.second;
    if (!connection.second || !connection.first) {
        QObject::disconnect(connection.first);
        m_connectionsCounter[object].remove(signalIndex);
        if (m_connectionsCounter[object].isEmpty()) {
            m_connectionsCounter.remove(object);
        }
    }
}

template<class Receiver>
int SignalHandler<Receiver>::qt_metacall(QMetaObject::Call call, int methodId, void **args)
{
    methodId = QObject::qt_metacall(call, methodId, args);
    if (methodId < 0)
        return methodId;

    if (call == QMetaObject::InvokeMetaMethod) {
        const QObject *object = sender();
        Q_ASSERT(object);
        Q_ASSERT(QThread::currentThread() == object->thread());
        Q_ASSERT(senderSignalIndex() == methodId);
        Q_ASSERT(m_connectionsCounter.contains(object));
        Q_ASSERT(m_connectionsCounter.value(object).contains(methodId));

        dispatch(object, methodId, args);

        return -1;
    }
    return methodId;
}

template<class Receiver>
void SignalHandler<Receiver>::remove(const QObject *object)
{
    for (ConnectionPair &connection : m_connectionsCounter.take(object))
        QObject::disconnect(connection.first);
}

QT_END_NAMESPACE

#endif // QTWEBCHANNEL_SIGNALHANDLER_P_H
