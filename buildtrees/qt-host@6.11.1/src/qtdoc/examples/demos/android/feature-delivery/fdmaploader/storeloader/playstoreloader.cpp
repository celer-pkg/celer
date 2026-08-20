// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "playstoreloader.h"

#include <QHash>
#include <QMutex>
#include <QCoreApplication>
#include <QPointer>

#include <QtCore/qjniarray.h>

#include <jni.h>

Q_DECLARE_JNI_CLASS(JPlayStoreLoader, "io/qt/fdmaploader/PlayStoreLoader");
Q_DECLARE_JNI_CLASS(JList, "java/util/List");
Q_DECLARE_JNI_CLASS(JIterator, "java/util/Iterator");

class PlayStoreLoaderImpl
{
public:
    PlayStoreLoaderImpl();
    bool registerNatives() const;
    QtJniTypes::JPlayStoreLoader loader();
    void addHandler(PlayStoreLoaderHandler *handler);
    PlayStoreLoaderHandler *findHandler(const jstring &callId);
    void removeHandler(const jstring &callId);

private:
    QtJniTypes::JPlayStoreLoader m_loader = nullptr;
    QHash<QString, QPointer<PlayStoreLoaderHandler>> m_handlers;
    QMutex m_lock;
};

Q_GLOBAL_STATIC(PlayStoreLoaderImpl, loaderInstance)

/*!
    \fn PlayStoreLoaderHandler::stateChanged(PlayStoreLoader::State state)

     Signals changes between various states.
     \sa PlayStoreLoader::State
*/

void stateChangedNative(JNIEnv *, jobject, jstring callId, int state)
{
    auto *handler = loaderInstance->findHandler(callId);
    if (!handler)
        return;
    emit handler->stateChanged(static_cast<PlayStoreLoader::State>(state));
}

/*!
    \fn PlayStoreLoaderHandler::errorOccured(int errorCode, const QString &errorString)

    Signals an error. Error code from Android SplitInstallErrorCode
    \l https://developer.android.com/reference/com/google/android/play/core/splitinstall/model/SplitInstallErrorCode
    In the case of exception, error code is -1 and exception described in the
    errorString.
    \a errorCode Code for resulted error.
    \a errorString String describing the error.
*/

void errorOccurredNative(JNIEnv *, jobject, jstring callId, int errorCode, jstring errorMessage)
{
    auto *handler = loaderInstance->findHandler(callId);
    if (!handler)
        return;
    emit handler->errorOccured(errorCode, QJniObject(errorMessage).toString());
}

/*!
    \fn PlayStoreLoaderHandler::confirmationRequest(int errorCode, const QString &errorString)

    Requests for user confirmation
*/

void userConfirmationRequestedNative(JNIEnv *, jobject, jstring callId, int errorCode,
                                     jstring errorMessage)
{
    auto *handler = loaderInstance->findHandler(callId);
    if (!handler)
        return;
    emit handler->confirmationRequest(errorCode, QJniObject(errorMessage).toString());
}

/*!
    \fn PlayStoreLoaderHandler::downloadProgress(qsizetype bytes, qsizetype total)

    Signals changed download progress. Called repeatedly during download.
    \a bytes Currently downloaded bytes.
    \a total Total amount of bytes to be downloaded.
*/

void downloadProgressChangedNative(JNIEnv *, jobject, jstring callId, jlong bytes, jlong total)
{
    auto *handler = loaderInstance->findHandler(callId);
    if (!handler)
        return;
    emit handler->downloadProgress(bytes, total);
}

/*!
    \fn PlayStoreLoaderHandler::finished()

    Signals finished module retrival process. Called when request for feature
    module has completed downloading and installing, or the request has resulted
    an error.
*/

void finishedNative(JNIEnv *, jobject, jstring callId)
{
    auto *handler = loaderInstance->findHandler(callId);
    if (!handler)
        return;
    emit handler->finished();
    loaderInstance->removeHandler(callId);
}

/*!
    \fn PlayStoreLoaderHandler::moduleUninstalled(PlayStoreLoader::ModuleRemovalStatus status);

    Signals finished uninstallation process. Called when a request ro uninstall
    module has finished.
    \a Uninstallation result, see PlayStoreLoader::ModuleRemovalStatus.
*/

void moduleUninstalledNative(JNIEnv *, jobject, jstring callId, int status)
{
    auto *handler = loaderInstance->findHandler(callId);
    if (!handler)
        return;
    emit handler->moduleUninstalled(static_cast<PlayStoreLoader::ModuleRemovalStatus>(status));
}

Q_DECLARE_JNI_NATIVE_METHOD(stateChangedNative)
Q_DECLARE_JNI_NATIVE_METHOD(errorOccurredNative)
Q_DECLARE_JNI_NATIVE_METHOD(userConfirmationRequestedNative)
Q_DECLARE_JNI_NATIVE_METHOD(downloadProgressChangedNative)
Q_DECLARE_JNI_NATIVE_METHOD(finishedNative)
Q_DECLARE_JNI_NATIVE_METHOD(moduleUninstalledNative)

PlayStoreLoaderImpl::PlayStoreLoaderImpl()
{
    m_loader = QJniObject::construct<QtJniTypes::JPlayStoreLoader, QtJniTypes::Context>(
            QNativeInterface::QAndroidApplication::context());
}

bool PlayStoreLoaderImpl::registerNatives() const
{
    static bool result = [] {
        QJniEnvironment env;
        return env.registerNativeMethods<QtJniTypes::JPlayStoreLoader>({
                Q_JNI_NATIVE_METHOD(stateChangedNative),
                Q_JNI_NATIVE_METHOD(errorOccurredNative),
                Q_JNI_NATIVE_METHOD(userConfirmationRequestedNative),
                Q_JNI_NATIVE_METHOD(downloadProgressChangedNative),
                Q_JNI_NATIVE_METHOD(finishedNative),
                Q_JNI_NATIVE_METHOD(moduleUninstalledNative)
        });
    }();
    if (!result)
        qCritical("Unable to register native methods.");
    return result;
}

QtJniTypes::JPlayStoreLoader PlayStoreLoaderImpl::loader()
{
    return m_loader;
}

void PlayStoreLoaderImpl::addHandler(PlayStoreLoaderHandler *handler)
{
    Q_ASSERT(handler);

    QMutexLocker lock(&m_lock);
    const auto &callId = handler->callId();
    if (Q_UNLIKELY(m_handlers.constFind(callId) != m_handlers.constEnd())) {
        qCritical() << "Handler with callId " << callId << " already exists";
        Q_ASSERT(false);
        return;
    }

    m_handlers[callId] = QPointer(handler);
}

PlayStoreLoaderHandler *PlayStoreLoaderImpl::findHandler(const jstring &callId)
{
    QMutexLocker lock(&m_lock);
    const auto it = m_handlers.constFind(QJniObject(callId).toString());
    if (it == m_handlers.constEnd()) {
        qCritical() << "The handler for the call " << callId << " was not found.";
        // TODO: Is it exceptional or should we ignore this
        return nullptr;
    }

    if (it.value().isNull()) {
        qCritical() << "The handler for the call " << callId << " expired.";
        m_handlers.erase(it);
    }

    return it.value().get();
}

void PlayStoreLoaderImpl::removeHandler(const jstring &callId)
{
    QMutexLocker lock(&m_lock);
    m_handlers.remove(QJniObject(callId).toString());
}

/*!
    Gets handler for callbacks.
*/
std::unique_ptr<PlayStoreLoaderHandler> PlayStoreLoader::getHandler()
{
    auto handlerPtr = std::make_unique<PlayStoreLoaderHandler>(nullptr);
    loaderInstance->addHandler(handlerPtr.get());

    return handlerPtr;
}

/*!
    \fn void PlayStoreLoader::loadModule(const QString & callId, const QString &moduleName)

    Loads feature module. \a callId Identifier for this transaction. Get handler
    to obtain the id. \a moduleName Name of the feature module to be downloaded.
*/
//! [Load Module Implementation]
void PlayStoreLoader::loadModule(const QString & callId,
    const QString &moduleName)
{
    if (callId.isEmpty() || moduleName.isEmpty())
        return;
    if (!loaderInstance->registerNatives())
        return;
    if (!loaderInstance->loader().isValid()) {
        qCritical("StoreLoader not constructed");
        return;
    }
    loaderInstance->loader().callMethod<void>("installModuleFromStore", moduleName, callId);
}
//! [Load Module Implementation]

/*!
    Gets Set of installed modules.
*/
QSet<QString> PlayStoreLoader::getInstalledModules()
{
    Q_ASSERT(loaderInstance->loader().isValid());
    QSet<QString> resModules;
    QtJniTypes::Set javaSet =
        loaderInstance->loader().callMethod<QtJniTypes::Set>("getInstalledModules");
    Q_ASSERT(javaSet.isValid());
    QtJniTypes::JIterator iterator = javaSet.callMethod<QtJniTypes::JIterator>("iterator");
    while (iterator.callMethod<jboolean>("hasNext")) {
        QJniObject module = iterator.callMethod<jobject>("next");
        resModules << module.toString();
    }
    return resModules;
}

/*!
    Uninstalls one or more feature modules. The uninstall process is deferred
    and Android processes it asynchronously at some point in the future.
    \a callId Id for the request. Get handler to obtain the id.
    \a modules List of module names to be uninstalled.
*/
void PlayStoreLoader::uninstallModules(const QString & callId,
    const QStringList &modules)
{
    if (modules.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "No modules given";
        return;
    }
    Q_ASSERT(loaderInstance->loader().isValid());

    QtJniTypes::ArrayList javaList;
    Q_ASSERT(javaList.isValid());

    for (const auto& qtModule : modules) {
        if (!javaList.callMethod<jboolean>("add", QJniObject::fromString(qtModule))) {
            qWarning() <<Q_FUNC_INFO <<"Failed to add module: " <<qtModule <<"J: "
                <<qtModule;
        }
    }
    loaderInstance->loader().callMethod<void>("uninstallModules", callId,
        javaList.object<QtJniTypes::JList>());
}

/*!
    Cancels pending or on-process loading of feature module.
    \a callId Id for the loading process.
*/
void PlayStoreLoader::cancelLoad(const QString &callId)
{
    Q_ASSERT(loaderInstance->loader().isValid());
    loaderInstance->loader().callMethod<void>("cancelLoad", callId);
}

PlayStoreLoaderHandler::PlayStoreLoaderHandler(QObject *parent)
    : QObject(parent)
{
}

PlayStoreLoaderHandler::~PlayStoreLoaderHandler() = default;

const QString &PlayStoreLoaderHandler::callId() const & noexcept
{
    return m_callId;
}

void PlayStoreLoaderHandler::setState(PlayStoreLoader::State state)
{
    if (m_state == state)
        return;
    m_state = state;
    emit stateChanged(m_state);
}
