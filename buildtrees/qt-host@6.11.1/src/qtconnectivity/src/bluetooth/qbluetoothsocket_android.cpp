// Copyright (C) 2016 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothsocket.h"
#include "qbluetoothsocket_android_p.h"
#include "qbluetoothaddress.h"
#include "qbluetoothdeviceinfo.h"
#include "qbluetoothserviceinfo.h"
#include "android/androidutils_p.h"
#include "android/jni_android_p.h"
#include <QCoreApplication>
#include <QtCore/QLoggingCategory>
#include <QtCore/QThread>
#include <QtCore/QTime>
#include <QtCore/QJniEnvironment>
#include <QtCore/q26numeric.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

#define USE_FALLBACK true

Q_BLUETOOTH_EXPORT bool useReverseUuidWorkAroundConnect = true;

/* BluetoothSocket.connect() can block up to 10s. Therefore it must be
 * in a separate thread. Unfortunately if BluetoothSocket.close() is
 * called while connect() is still blocking the resulting behavior is not reliable.
 * This may well be an Android platform bug. In any case, close() must
 * be queued up until connect() has returned.
 *
 * The WorkerThread manages the connect() and close() calls. Interaction
 * with the main thread happens via signals and slots. There is an accepted but
 * undesirable side effect of this approach as the user may call connect()
 * and close() and the socket would continue to successfully connect to
 * the remote device just to immidiately close the physical connection again.
 *
 * WorkerThread and SocketConnectWorker are cleaned up via the threads
 * finished() signal.
 */

class SocketConnectWorker : public QObject
{
    Q_OBJECT
public:
    SocketConnectWorker(const QJniObject& socket,
                        const QJniObject& targetUuid,
                        const QBluetoothUuid& qtTargetUuid)
        : QObject(),
          mSocketObject(socket),
          mTargetUuid(targetUuid),
          mQtTargetUuid(qtTargetUuid)
    {
        static int t = qRegisterMetaType<QJniObject>();
        Q_UNUSED(t);
    }

signals:
    void socketConnectDone(const QJniObject &socket);
    void socketConnectFailed(const QJniObject &socket,
                             const QJniObject &targetUuid,
                             const QBluetoothUuid &qtUuid);
public slots:
    void connectSocket()
    {
        QJniEnvironment env;

        qCDebug(QT_BT_ANDROID) << "Connecting socket";
        auto methodId = env.findMethod(mSocketObject.objectClass(), "connect", "()V");
        if (methodId)
            env->CallVoidMethod(mSocketObject.object(), methodId);
        if (!methodId || env.checkAndClearExceptions()) {
            emit socketConnectFailed(mSocketObject, mTargetUuid, mQtTargetUuid);
            QThread::currentThread()->quit();
            return;
        }

        qCDebug(QT_BT_ANDROID) << "Socket connection established";
        emit socketConnectDone(mSocketObject);
    }

    void closeSocket()
    {
        qCDebug(QT_BT_ANDROID) << "Executing queued closeSocket()";

        mSocketObject.callMethod<void>("close");
        QThread::currentThread()->quit();
    }

private:
    QJniObject mSocketObject;
    QJniObject mTargetUuid;
    // same as mTargetUuid above - just the Qt C++ version rather than jni uuid
    QBluetoothUuid mQtTargetUuid;
};

class WorkerThread: public QThread
{
    Q_OBJECT
public:
    WorkerThread()
        : QThread(), workerPointer(0)
    {
    }

    // Runs in same thread as QBluetoothSocketPrivateAndroid
    void setupWorker(QBluetoothSocketPrivateAndroid* d_ptr, const QJniObject& socketObject,
                     const QJniObject& uuidObject, bool useFallback,
                     const QBluetoothUuid& qtUuid = QBluetoothUuid())
    {
        SocketConnectWorker* worker = new SocketConnectWorker(
                                            socketObject, uuidObject, qtUuid);
        worker->moveToThread(this);

        connect(this, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &QThread::finished, this, &QObject::deleteLater);
        connect(d_ptr, &QBluetoothSocketPrivateAndroid::connectJavaSocket,
                worker, &SocketConnectWorker::connectSocket);
        connect(d_ptr, &QBluetoothSocketPrivateAndroid::closeJavaSocket,
                worker, &SocketConnectWorker::closeSocket);
        connect(worker, &SocketConnectWorker::socketConnectDone,
                d_ptr, &QBluetoothSocketPrivateAndroid::socketConnectSuccess);
        if (useFallback) {
            connect(worker, &SocketConnectWorker::socketConnectFailed,
                    d_ptr, &QBluetoothSocketPrivateAndroid::fallbackSocketConnectFailed);
        } else {
            connect(worker, &SocketConnectWorker::socketConnectFailed,
                d_ptr, &QBluetoothSocketPrivateAndroid::defaultSocketConnectFailed);
        }

        workerPointer = worker;
    }

private:
    QPointer<SocketConnectWorker> workerPointer;
};

QBluetoothSocketPrivateAndroid::QBluetoothSocketPrivateAndroid()
  :
    inputThread(0)
{
    secFlags = QBluetooth::Security::Secure;
    adapter = getDefaultBluetoothAdapter();
    qRegisterMetaType<QBluetoothSocket::SocketError>();
    qRegisterMetaType<QBluetoothSocket::SocketState>();
}

QBluetoothSocketPrivateAndroid::~QBluetoothSocketPrivateAndroid()
{
    if (state != QBluetoothSocket::SocketState::UnconnectedState)
        emit closeJavaSocket();
}

bool QBluetoothSocketPrivateAndroid::ensureNativeSocket(QBluetoothServiceInfo::Protocol type)
{
    socketType = type;
    if (socketType == QBluetoothServiceInfo::RfcommProtocol)
        return true;

    return false;
}

/*
 * Workaround for QTBUG-61392. If the underlying Android bug gets fixed,
 * we need to consider restoring the non-reversed fallbackConnect from the repository.
 */
bool QBluetoothSocketPrivateAndroid::fallBackReversedConnect(const QBluetoothUuid &uuid)
{
    Q_Q(QBluetoothSocket);

    qCWarning(QT_BT_ANDROID) << "Falling back to reverse uuid workaround.";
    const QBluetoothUuid reverse = reverseUuid(uuid);
    if (reverse.isNull())
        return false;

    QString tempUuid = reverse.toString(QUuid::WithoutBraces);

    QJniEnvironment env;
    const QJniObject inputString = QJniObject::fromString(tempUuid);
    const QJniObject uuidObject = QJniObject::callStaticMethod<QtJniTypes::UUID>(
                QtJniTypes::Traits<QtJniTypes::UUID>::className(), "fromString",
                inputString.object<jstring>());

    if (secFlags == QBluetooth::SecurityFlags(QBluetooth::Security::NoSecurity)) {
        qCDebug(QT_BT_ANDROID) << "Connecting via insecure rfcomm";
        socketObject = remoteDevice.callMethod<QtJniTypes::BluetoothSocket>(
                                    "createInsecureRfcommSocketToServiceRecord",
                                    uuidObject.object<QtJniTypes::UUID>());
    } else {
        qCDebug(QT_BT_ANDROID) << "Connecting via secure rfcomm";
        socketObject = remoteDevice.callMethod<QtJniTypes::BluetoothSocket>(
                                    "createRfcommSocketToServiceRecord",
                                    uuidObject.object<QtJniTypes::UUID>());
    }

    if (!socketObject.isValid()) {
        remoteDevice = QJniObject();
        errorString = QBluetoothSocket::tr("Cannot connect to %1",
                                           "%1 = uuid").arg(reverse.toString());
        q->setSocketError(QBluetoothSocket::SocketError::ServiceNotFoundError);
        q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return false;
    }

    WorkerThread *workerThread = new WorkerThread();
    workerThread->setupWorker(this, socketObject, uuidObject, USE_FALLBACK);
    workerThread->start();
    emit connectJavaSocket();

    return true;
}

/*
 * The call order during a connectToServiceHelper() is as follows:
 *
 * 1. call connectToServiceHelper()
 * 2. wait for execution of SocketConnectThread::run()
 * 3. if threaded connect succeeds call socketConnectSuccess() via signals
 *      -> done
 * 4. if threaded connect fails call defaultSocketConnectFailed() via signals
 * 5. call fallBackReversedConnect()
 *     -> if failure entire connectToServiceHelper() fails
 *     Note: This fallback can be disabled with private API boolean
 * 6. if threaded connect on one of above fallbacks succeeds call socketConnectSuccess()
 *    via signals
 *      -> done
 * 7. if threaded connect on fallback channel fails call fallbackSocketConnectFailed()
 *      -> complete failure of entire connectToServiceHelper()
 * */
void QBluetoothSocketPrivateAndroid::connectToServiceHelper(const QBluetoothAddress &address,
                                               const QBluetoothUuid &uuid,
                                               QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);
    Q_UNUSED(openMode);

    qCDebug(QT_BT_ANDROID) << "connectToServiceHelper()" << address.toString() << uuid.toString();

    if (!ensureAndroidPermission(QBluetoothPermission::Access)) {
        qCWarning(QT_BT_ANDROID) << "Bluetooth socket connect failed due to missing permissions";
        errorString = QBluetoothSocket::tr(
                "Bluetooth socket connect failed due to missing permissions.");
        q->setSocketError(QBluetoothSocket::SocketError::MissingPermissionsError);
        q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return;
    }

    q->setSocketState(QBluetoothSocket::SocketState::ConnectingState);

    if (!adapter.isValid()) {
        qCWarning(QT_BT_ANDROID) << "Device does not support Bluetooth";
        errorString = QBluetoothSocket::tr("Device does not support Bluetooth");
        q->setSocketError(QBluetoothSocket::SocketError::NetworkError);
        q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return;
    }

    const int state = adapter.callMethod<jint>("getState");
    if (state != 12 ) { //BluetoothAdapter.STATE_ON
        qCWarning(QT_BT_ANDROID) << "Bluetooth device offline";
        errorString = QBluetoothSocket::tr("Device is powered off");
        q->setSocketError(QBluetoothSocket::SocketError::NetworkError);
        q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return;
    }

    QJniEnvironment env;
    QJniObject inputString = QJniObject::fromString(address.toString());
    remoteDevice = adapter.callMethod<QtJniTypes::BluetoothDevice>("getRemoteDevice",
                                            inputString.object<jstring>());
    if (!remoteDevice.isValid()) {
        errorString = QBluetoothSocket::tr("Cannot access address %1", "%1 = Bt address e.g. 11:22:33:44:55:66").arg(address.toString());
        q->setSocketError(QBluetoothSocket::SocketError::HostNotFoundError);
        q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return;
    }

    //cut leading { and trailing } {xxx-xxx}
    const QString tempUuid = uuid.toString(QUuid::WithoutBraces);

    inputString = QJniObject::fromString(tempUuid);
    const QJniObject uuidObject = QJniObject::callStaticMethod<QtJniTypes::UUID>(
                            QtJniTypes::Traits<QtJniTypes::UUID>::className(), "fromString",
                            inputString.object<jstring>());

    if (secFlags == QBluetooth::SecurityFlags(QBluetooth::Security::NoSecurity)) {
        qCDebug(QT_BT_ANDROID) << "Connecting via insecure rfcomm";
        socketObject = remoteDevice.callMethod<QtJniTypes::BluetoothSocket>(
                                    "createInsecureRfcommSocketToServiceRecord",
                                    uuidObject.object<QtJniTypes::UUID>());
    } else {
        qCDebug(QT_BT_ANDROID) << "Connecting via secure rfcomm";
        socketObject = remoteDevice.callMethod<QtJniTypes::BluetoothSocket>(
                                    "createRfcommSocketToServiceRecord",
                                    uuidObject.object<QtJniTypes::UUID>());
    }

    if (!socketObject.isValid()) {
        remoteDevice = QJniObject();
        errorString = QBluetoothSocket::tr("Cannot connect to %1 on %2",
                                           "%1 = uuid, %2 = Bt address").arg(uuid.toString()).arg(address.toString());
        q->setSocketError(QBluetoothSocket::SocketError::ServiceNotFoundError);
        q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return;
    }

    WorkerThread *workerThread = new WorkerThread();
    workerThread->setupWorker(this, socketObject, uuidObject, !USE_FALLBACK, uuid);
    workerThread->start();
    emit connectJavaSocket();
}

void QBluetoothSocketPrivateAndroid::connectToService(
        const QBluetoothServiceInfo &service, QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    if (q->state() != QBluetoothSocket::SocketState::UnconnectedState
            && q->state() != QBluetoothSocket::SocketState::ServiceLookupState) {
        qCWarning(QT_BT_ANDROID) << "QBluetoothSocketPrivateAndroid::connectToService called on busy socket";
        errorString = QBluetoothSocket::tr("Trying to connect while connection is in progress");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return;
    }

    // Workaround for QTBUG-75035
    /* Not all Android devices publish or discover the SPP uuid for serial services.
     * Also, Android does not permit the detection of the protocol used by a serial
     * Bluetooth connection.
     *
     * Therefore, QBluetoothServiceDiscoveryAgentPrivate::populateDiscoveredServices()
     * may have to guess what protocol a potential custom uuid uses. The guessing works
     * reasonably well as long as the SDP discovery finds the SPP uuid. Otherwise
     * the SPP and rfcomm protocol info is missing in \a service.
     *
     * Android only supports RFCOMM (no L2CP). We assume (in favor of user experience)
     * that a non-RFCOMM protocol implies a missing SPP uuid during discovery but the user
     * still wanting to connect with the given \a service instance.
     */

    auto protocol = service.socketProtocol();
    switch (protocol) {
    case QBluetoothServiceInfo::L2capProtocol:
    case QBluetoothServiceInfo::UnknownProtocol:
        qCWarning(QT_BT_ANDROID) << "Changing socket protocol to RFCOMM";
        protocol = QBluetoothServiceInfo::RfcommProtocol;
        break;
    case QBluetoothServiceInfo::RfcommProtocol:
        break;
    }

    if (!ensureNativeSocket(protocol)) {
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
        return;
    }
    connectToServiceHelper(service.device().address(), service.serviceUuid(), openMode);
}

void QBluetoothSocketPrivateAndroid::connectToService(
        const QBluetoothAddress &address, const QBluetoothUuid &uuid,
        QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    if (q->state() != QBluetoothSocket::SocketState::UnconnectedState) {
        qCWarning(QT_BT_ANDROID) << "QBluetoothSocketPrivateAndroid::connectToService called on busy socket";
        errorString = QBluetoothSocket::tr("Trying to connect while connection is in progress");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return;
    }

    if (q->socketType() == QBluetoothServiceInfo::UnknownProtocol) {
        qCWarning(QT_BT_ANDROID) << "QBluetoothSocketPrivateAndroid::connectToService cannot "
                                  "connect with 'UnknownProtocol' (type provided by given service)";
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
        return;
    }

    if (!ensureNativeSocket(q->socketType())) {
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
        return;
    }
    connectToServiceHelper(address, uuid, openMode);
}

void QBluetoothSocketPrivateAndroid::connectToService(
        const QBluetoothAddress &address, quint16 port, QIODevice::OpenMode openMode)
{
    Q_UNUSED(port);
    Q_UNUSED(openMode);
    Q_UNUSED(address);

    Q_Q(QBluetoothSocket);

    errorString = tr("Connecting to port is not supported");
    q->setSocketError(QBluetoothSocket::SocketError::ServiceNotFoundError);
    qCWarning(QT_BT_ANDROID) << "Connecting to port is not supported";
}

void QBluetoothSocketPrivateAndroid::socketConnectSuccess(const QJniObject &socket)
{
    Q_Q(QBluetoothSocket);
    QJniEnvironment env;

    // test we didn't get a success from a previous connect
    // which was cleaned up late
    if (socket != socketObject)
        return;

    if (inputThread) {
        inputThread->deleteLater();
        inputThread = 0;
    }

    inputStream = socketObject.callMethod<QtJniTypes::InputStream>("getInputStream");
    outputStream = socketObject.callMethod<QtJniTypes::OutputStream>("getOutputStream");

    if (!inputStream.isValid() || !outputStream.isValid()) {

        emit closeJavaSocket();
        socketObject = inputStream = outputStream = remoteDevice = QJniObject();


        errorString = QBluetoothSocket::tr("Obtaining streams for service failed");
        q->setSocketError(QBluetoothSocket::SocketError::NetworkError);
        q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return;
    }

    inputThread = new InputStreamThread(this);
    QObject::connect(inputThread, SIGNAL(dataAvailable()),
                     q, SIGNAL(readyRead()), Qt::QueuedConnection);
    QObject::connect(inputThread, SIGNAL(errorOccurred(int)), this, SLOT(inputThreadError(int)),
                     Qt::QueuedConnection);

    if (!inputThread->run()) {
        //close socket again
        emit closeJavaSocket();

        socketObject = inputStream = outputStream = remoteDevice = QJniObject();

        delete inputThread;
        inputThread = 0;

        errorString = QBluetoothSocket::tr("Input stream thread cannot be started");
        q->setSocketError(QBluetoothSocket::SocketError::NetworkError);
        q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return;
    }

    // only unbuffered behavior supported at this stage
    q->setOpenMode(QIODevice::ReadWrite|QIODevice::Unbuffered);

    q->setSocketState(QBluetoothSocket::SocketState::ConnectedState);
}

void QBluetoothSocketPrivateAndroid::defaultSocketConnectFailed(
        const QJniObject &socket, const QJniObject &targetUuid,
        const QBluetoothUuid &qtTargetUuid)
{
    Q_UNUSED(targetUuid);
    Q_Q(QBluetoothSocket);

    // test we didn't get a fail from a previous connect
    // which was cleaned up late - should be same socket
    if (socket != socketObject)
        return;

    if (!useReverseUuidWorkAroundConnect || !fallBackReversedConnect(qtTargetUuid)) {
        errorString = QBluetoothSocket::tr("Connection to service failed");
        socketObject = remoteDevice = QJniObject();
        q->setSocketError(QBluetoothSocket::SocketError::ServiceNotFoundError);
        q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        qCWarning(QT_BT_ANDROID) << "Socket connect workaround failed";
    }
}

void QBluetoothSocketPrivateAndroid::fallbackSocketConnectFailed(
        const QJniObject &socket, const QJniObject &targetUuid)
{
    Q_UNUSED(targetUuid);
    Q_Q(QBluetoothSocket);

    // test we didn't get a fail from a previous connect
    // which was cleaned up late - should be same socket
    if (socket != socketObject)
        return;

    qCWarning(QT_BT_ANDROID) << "Socket connect via workaround failed.";
    errorString = QBluetoothSocket::tr("Connection to service failed");
    socketObject = remoteDevice = QJniObject();

    q->setSocketError(QBluetoothSocket::SocketError::ServiceNotFoundError);
    q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
}

void QBluetoothSocketPrivateAndroid::abort()
{
    if (state == QBluetoothSocket::SocketState::UnconnectedState)
        return;

    if (socketObject.isValid()) {
        QJniEnvironment env;

        /*
         * BluetoothSocket.close() triggers an abort of the input stream
         * thread because inputStream.read() throws IOException
         * In turn the thread stops and throws an error which sets
         * new state, error and emits relevant signals.
         * See QBluetoothSocketPrivateAndroid::inputThreadError() for details
         */

        if (inputThread)
            inputThread->prepareForClosure();

        emit closeJavaSocket();

        inputStream = outputStream = socketObject = remoteDevice = QJniObject();

        if (inputThread) {
            // inputThread exists hence we had a successful connect
            // which means inputThread is responsible for setting Unconnected

            //don't delete here as signals caused by Java Thread are still
            //going to be emitted
            //delete occurs in inputThreadError()
            inputThread = 0;
        } else {
            // inputThread doesn't exist hence
            // we abort in the middle of connect(). WorkerThread will do
            // close() without further feedback. Therefore we have to set
            // Unconnected (now) in advance
            Q_Q(QBluetoothSocket);
            q->setOpenMode(QIODevice::NotOpen);
            q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
            emit q->readChannelFinished();
        }
    }
}

QString QBluetoothSocketPrivateAndroid::localName() const
{
    if (!ensureAndroidPermission(QBluetoothPermission::Access)) {
        qCWarning(QT_BT_ANDROID) << "Bluetooth socket localName() failed due to"
                                    "missing permissions";
    } else if (adapter.isValid()) {
        return adapter.callMethod<jstring>("getName").toString();
    }

    return QString();
}

QBluetoothAddress QBluetoothSocketPrivateAndroid::localAddress() const
{
    QString result;

    if (!ensureAndroidPermission(QBluetoothPermission::Access)) {
        qCWarning(QT_BT_ANDROID) << "Bluetooth socket localAddress() failed due to"
                                    "missing permissions";
    } else if (adapter.isValid()) {
        result = adapter.callMethod<jstring>("getAddress").toString();
    }

    return QBluetoothAddress(result);
}

quint16 QBluetoothSocketPrivateAndroid::localPort() const
{
    // Impossible to get channel number with current Android API (Levels 5 to 19)
    return 0;
}

QString QBluetoothSocketPrivateAndroid::peerName() const
{
    if (!remoteDevice.isValid())
        return QString();

    return remoteDevice.callMethod<jstring>("getName").toString();
}

QBluetoothAddress QBluetoothSocketPrivateAndroid::peerAddress() const
{
    if (!remoteDevice.isValid())
        return QBluetoothAddress();

    const QString address = remoteDevice.callMethod<jstring>("getAddress").toString();
    return QBluetoothAddress(address);
}

quint16 QBluetoothSocketPrivateAndroid::peerPort() const
{
    // Impossible to get channel number with current Android API (Levels 5 to 13)
    return 0;
}

qint64 QBluetoothSocketPrivateAndroid::writeData(const char *data, qint64 maxSize)
{
    //TODO implement buffered behavior (so far only unbuffered)
    Q_Q(QBluetoothSocket);
    if (state != QBluetoothSocket::SocketState::ConnectedState || !outputStream.isValid()) {
        qCWarning(QT_BT_ANDROID) << "Socket::writeData: " << state << outputStream.isValid();
        errorString = QBluetoothSocket::tr("Cannot write while not connected");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return -1;
    }

    const qint32 javaSize = q26::saturate_cast<qint32>(maxSize);
    QJniEnvironment env;
    jbyteArray nativeData = env->NewByteArray(javaSize);
    env->SetByteArrayRegion(nativeData, 0, javaSize, reinterpret_cast<const jbyte*>(data));
    auto methodId = env.findMethod(outputStream.objectClass(),
                                   "write",
                                   "([BII)V");
    if (methodId)
        env->CallVoidMethod(outputStream.object(), methodId, nativeData, 0, javaSize);
    env->DeleteLocalRef(nativeData);

    if (!methodId || env.checkAndClearExceptions()) {
        qCWarning(QT_BT_ANDROID) << "Error while writing";
        errorString = QBluetoothSocket::tr("Error during write on socket.");
        q->setSocketError(QBluetoothSocket::SocketError::NetworkError);
        return -1;
    }

    emit q->bytesWritten(javaSize);
    return javaSize;
}

qint64 QBluetoothSocketPrivateAndroid::readData(char *data, qint64 maxSize)
{
    Q_Q(QBluetoothSocket);
    if (state != QBluetoothSocket::SocketState::ConnectedState || !inputThread) {
        qCWarning(QT_BT_ANDROID) << "Socket::readData: " << state << inputThread ;
        errorString = QBluetoothSocket::tr("Cannot read while not connected");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return -1;
    }

    return inputThread->readData(data, maxSize);
}

void QBluetoothSocketPrivateAndroid::inputThreadError(int errorCode)
{
    Q_Q(QBluetoothSocket);

    if (errorCode != -1) { //magic error which is expected and can be ignored
        errorString = QBluetoothSocket::tr("Network error during read");
        q->setSocketError(QBluetoothSocket::SocketError::NetworkError);
    }

    //finally we can delete the InputStreamThread
    InputStreamThread *client = qobject_cast<InputStreamThread *>(sender());
    if (client)
        client->deleteLater();

    if (socketObject.isValid()) {
        //triggered when remote side closed the socket
        //cleanup internal objects
        //if it was call to local close()/abort() the objects are cleaned up already

        emit closeJavaSocket();

        inputStream = outputStream = remoteDevice = socketObject = QJniObject();
        if (inputThread) {
            // deleted already above (client->deleteLater())
            inputThread = 0;
        }
    }

    q->setOpenMode(QIODevice::NotOpen);
    q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
    emit q->readChannelFinished();
}

void QBluetoothSocketPrivateAndroid::close()
{
    /* This function is called by QBluetoothSocket::close and softer version
       QBluetoothSocket::disconnectFromService() which difference I do not quite fully understand.
       Anyways we end up in Android "close" function call.
     */
    abort();
}

bool QBluetoothSocketPrivateAndroid::setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType,
                         QBluetoothSocket::SocketState socketState, QBluetoothSocket::OpenMode openMode)
{
    Q_UNUSED(socketDescriptor);
    Q_UNUSED(socketType);
    Q_UNUSED(socketState);
    Q_UNUSED(openMode);
    qCWarning(QT_BT_ANDROID) << "No socket descriptor support on Android.";
    return false;
}

bool QBluetoothSocketPrivateAndroid::setSocketDescriptor(const QJniObject &socket, QBluetoothServiceInfo::Protocol socketType_,
                         QBluetoothSocket::SocketState socketState, QBluetoothSocket::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    if (q->state() != QBluetoothSocket::SocketState::UnconnectedState || !socket.isValid())
        return false;

    if (!ensureNativeSocket(socketType_))
        return false;

    socketObject = socket;

    inputStream = socketObject.callMethod<QtJniTypes::InputStream>("getInputStream");
    outputStream = socketObject.callMethod<QtJniTypes::OutputStream>("getOutputStream");

    if (!inputStream.isValid() || !outputStream.isValid()) {

        //close socket again
        socketObject.callMethod<void>("close");

        socketObject = inputStream = outputStream = remoteDevice = QJniObject();


        errorString = QBluetoothSocket::tr("Obtaining streams for service failed");
        q->setSocketError(QBluetoothSocket::SocketError::NetworkError);
        q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return false;
    }

    remoteDevice = socketObject.callMethod<QtJniTypes::BluetoothDevice>("getRemoteDevice");

    if (inputThread) {
        inputThread->deleteLater();
        inputThread = 0;
    }
    inputThread = new InputStreamThread(this);
    QObject::connect(inputThread, SIGNAL(dataAvailable()),
                     q, SIGNAL(readyRead()), Qt::QueuedConnection);
    QObject::connect(inputThread, SIGNAL(errorOccurred(int)), this, SLOT(inputThreadError(int)),
                     Qt::QueuedConnection);
    inputThread->run();

    // WorkerThread manages all sockets for us
    // When we come through here the socket was already connected by
    // server socket listener (see QBluetoothServer)
    // Therefore we only use WorkerThread to potentially close it later on
    WorkerThread *workerThread = new WorkerThread();
    workerThread->setupWorker(this, socketObject, QJniObject(), !USE_FALLBACK);
    workerThread->start();

    q->setOpenMode(openMode | QIODevice::Unbuffered);
    q->setSocketState(socketState);

    return true;
}

qint64 QBluetoothSocketPrivateAndroid::bytesAvailable() const
{
    //We cannot access buffer directly as it is part of different thread
    if (inputThread)
        return inputThread->bytesAvailable();

    return 0;
}

qint64 QBluetoothSocketPrivateAndroid::bytesToWrite() const
{
    return 0; // nothing because always unbuffered
}

/*
 * This function is part of a workaround for QTBUG-61392
 *
 * Returns null uuid if the given \a serviceUuid is not a uuid
 * derived from the Bluetooth base uuid.
 */
QBluetoothUuid QBluetoothSocketPrivateAndroid::reverseUuid(const QBluetoothUuid &serviceUuid)
{
    if (serviceUuid.isNull())
        return QBluetoothUuid();

    bool isBaseUuid = false;
    serviceUuid.toUInt32(&isBaseUuid);
    if (isBaseUuid)
        return serviceUuid;

    const QUuid::Id128Bytes original = serviceUuid.toBytes();
    QUuid::Id128Bytes reversed;
    for (int i = 0; i < 16; i++)
        reversed.data[15-i] = original.data[i];
    return QBluetoothUuid{reversed};
}

bool QBluetoothSocketPrivateAndroid::canReadLine() const
{
    // We cannot access buffer directly as it is part of different thread
    if (inputThread)
        return inputThread->canReadLine();

    return false;
}

QT_END_NAMESPACE

#include <qbluetoothsocket_android.moc>
