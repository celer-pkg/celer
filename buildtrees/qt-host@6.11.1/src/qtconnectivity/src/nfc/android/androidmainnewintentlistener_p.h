// Copyright (C) 2016 BasysKom GmbH
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef ANDROIDMAINNEWINTENTLISTENER_P_H_
#define ANDROIDMAINNEWINTENTLISTENER_P_H_

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

#include <QtCore/private/qjnihelpers_p.h>

#include "qlist.h"
#include "qreadwritelock.h"

QT_BEGIN_NAMESPACE

class QJniObject;

class QAndroidNfcListenerInterface
{
public:
    virtual ~QAndroidNfcListenerInterface() = default;
    virtual void newIntent(QJniObject intent) = 0;
};

class QMainNfcNewIntentListener : public QtAndroidPrivate::NewIntentListener,
                                  QtAndroidPrivate::ResumePauseListener
{
public:
    QMainNfcNewIntentListener();
    ~QMainNfcNewIntentListener();

    //QtAndroidPrivate::NewIntentListener
    bool handleNewIntent(JNIEnv *env, jobject intent);

    bool registerListener(QAndroidNfcListenerInterface *listener);
    bool unregisterListener(QAndroidNfcListenerInterface *listener);

    //QtAndroidPrivate::ResumePauseListener
    void handleResume();
    void handlePause();

private:
    void updateReceiveState();

    QList<QAndroidNfcListenerInterface *> listeners;
    QReadWriteLock listenersLock;
    bool paused;
    bool receiving;
};

QT_END_NAMESPACE

#endif /* ANDROIDMAINNEWINTENTLISTENER_P_H_ */
