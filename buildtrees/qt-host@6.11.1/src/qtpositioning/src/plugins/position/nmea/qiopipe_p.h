// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QIOPIPE_P_H
#define QIOPIPE_P_H

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

#include <QtCore/qiodevice.h>
#include <QtCore/qbytearray.h>
#include <QtCore/private/qiodevice_p.h>
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QObject;
class QIOPipePrivate;

class QIOPipe : public QIODevice
{
    Q_OBJECT

public:
    enum Mode {
        EndPipe = 0x0000,
        ProxyPipe = 0x0001
    };

     explicit QIOPipe(QIODevice *parent, Mode mode = EndPipe);
    ~QIOPipe() override;

    bool open(OpenMode openMode) override;
    bool isSequential() const override;
    void setReadChannelCount(int count);
    void addChildPipe(QIOPipe *childPipe);

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    Q_DECLARE_PRIVATE(QIOPipe)
    Q_DISABLE_COPY(QIOPipe)
};

class QIOPipePrivate : public QIODevicePrivate
{
public:
    Q_DECLARE_PUBLIC(QIOPipe)

    explicit QIOPipePrivate(QIODevice *iodevice, bool proxying);
    ~QIOPipePrivate() override;

    void initialize();
    bool readAvailableData();
    void pumpData(const QByteArray &ba);
    void pushData(const QByteArray &ba);
    void _q_onReadyRead();
    void addChildPipe(QIOPipe *childPipe);
    void removeChildPipe(QIOPipe *childPipe);

    bool m_proxying = false;
    QPointer<QIODevice> source;
    QList<QPointer<QIOPipe>> childPipes;
};

QT_END_NAMESPACE

#endif // QIOPIPE_P_H

