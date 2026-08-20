// Copyright (C) 2017 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef INPUTSTREAMTHREAD_H
#define INPUTSTREAMTHREAD_H

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

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QJniObject>
#include <QtCore/private/qglobal_p.h>
#include <jni.h>

QT_BEGIN_NAMESPACE

class QBluetoothSocketPrivateAndroid;

class InputStreamThread : public QObject
{
    Q_OBJECT
public:
    explicit InputStreamThread(QBluetoothSocketPrivateAndroid *socket_p);

    qint64 bytesAvailable() const;
    bool canReadLine() const;
    bool run();

    qint64 readData(char *data, qint64 maxSize);
    void javaThreadErrorOccurred(int errorCode);
    void javaReadyRead(jbyteArray buffer, int bufferLength);

    void prepareForClosure();

signals:
    void dataAvailable();
    void errorOccurred(int errorCode);

private:
    QBluetoothSocketPrivateAndroid *m_socket_p;
    QJniObject javaInputStreamThread;
    mutable QMutex m_mutex;
    bool expectClosure;
};

QT_END_NAMESPACE

#endif // INPUTSTREAMTHREAD_H
