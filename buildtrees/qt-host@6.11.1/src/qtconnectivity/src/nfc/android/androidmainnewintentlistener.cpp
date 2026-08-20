// Copyright (C) 2016 BasysKom GmbH
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#include "androidmainnewintentlistener_p.h"
#include "android/androidjninfc_p.h"
#include <QtGui/QGuiApplication>
#include <QtCore/QJniObject>

QT_BEGIN_NAMESPACE

QMainNfcNewIntentListener::QMainNfcNewIntentListener() : paused(true), receiving(false)
{
    QtAndroidPrivate::registerNewIntentListener(this);
    QtAndroidPrivate::registerResumePauseListener(this);
}

QMainNfcNewIntentListener::~QMainNfcNewIntentListener()
{
    QtAndroidPrivate::unregisterNewIntentListener(this);
    QtAndroidPrivate::unregisterResumePauseListener(this);
}

bool QMainNfcNewIntentListener::handleNewIntent(JNIEnv * /*env*/, jobject intentObject)
{
    QtJniTypes::Intent intent = intentObject;
    // Only intents with a tag are relevant
    if (!QtNfc::getTag(intent).isValid())
        return false;

    listenersLock.lockForRead();
    for (auto listener : std::as_const(listeners))
        listener->newIntent(intent);
    listenersLock.unlock();
    return true;
}

bool QMainNfcNewIntentListener::registerListener(QAndroidNfcListenerInterface *listener)
{
    static bool firstListener = true;
    if (firstListener) {
        QtJniTypes::Intent intent = QtNfc::getStartIntent();
        if (intent.isValid())
            listener->newIntent(intent);
        paused = static_cast<QGuiApplication*>(QGuiApplication::instance())->applicationState() != Qt::ApplicationActive;
    }
    firstListener = false;
    listenersLock.lockForWrite();
    if (!listeners.contains(listener))
        listeners.push_back(listener);
    listenersLock.unlock();
    updateReceiveState();
    return true;
}

bool QMainNfcNewIntentListener::unregisterListener(QAndroidNfcListenerInterface *listener)
{
    listenersLock.lockForWrite();
    listeners.removeOne(listener);
    listenersLock.unlock();
    updateReceiveState();
    return true;
}

void QMainNfcNewIntentListener::handleResume()
{
    paused = false;
    updateReceiveState();
}

void QMainNfcNewIntentListener::handlePause()
{
    paused = true;
    updateReceiveState();
}

void QMainNfcNewIntentListener::updateReceiveState()
{
    if (paused) {
        // We were paused while receiving, so we stop receiving.
        if (receiving) {
            QtNfc::stopDiscovery();
            receiving = false;
        }
        return;
    }

    // We reach here, so we are not paused.
    listenersLock.lockForRead();
    // We have nfc listeners and do not receive. Switch on.
    if (!listeners.isEmpty() && !receiving)
        receiving = QtNfc::startDiscovery();

    // we have no nfc listeners and do receive. Switch off.
    if (listeners.isEmpty() && receiving) {
        QtNfc::stopDiscovery();
        receiving = false;
    }
    listenersLock.unlock();
}

QT_END_NAMESPACE
