// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QNEARFIELDTARGET_EMULATOR_P_H
#define QNEARFIELDTARGET_EMULATOR_P_H

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

#include "qnearfieldtagtype1_p.h"
#include "qnearfieldtagtype2_p.h"
#include "targetemulator_p.h"

#include <QtCore/QMap>

QT_BEGIN_NAMESPACE

class TagType1 : public QNearFieldTagType1
{
    Q_OBJECT

public:
    TagType1(TagBase *tag, QObject *parent = nullptr);
    ~TagType1();

    QByteArray uid() const override;

    QNearFieldTarget::AccessMethods accessMethods() const override;

    QNearFieldTarget::RequestId sendCommand(const QByteArray &command) override;
    bool waitForRequestCompleted(const QNearFieldTarget::RequestId &id, int msecs);

private:
    TagBase *tag;
};

class TagType2 : public QNearFieldTagType2
{
    Q_OBJECT

public:
    TagType2(TagBase *tag, QObject *parent = nullptr);
    ~TagType2();

    QByteArray uid() const override;

    QNearFieldTarget::AccessMethods accessMethods() const override;

    QNearFieldTarget::RequestId sendCommand(const QByteArray &command) override;
    bool waitForRequestCompleted(const QNearFieldTarget::RequestId &id, int msecs);

private:
    TagBase *tag;
};

class TagActivator : public QObject
{
    Q_OBJECT

public:
    TagActivator();
    ~TagActivator();

    void initialize();
    void reset();

    void start();
    void stop();

    static TagActivator *instance();

protected:
    void timerEvent(QTimerEvent *e) override;

signals:
    void tagActivated(TagBase *tag);
    void tagDeactivated(TagBase *tag);

private:
    void stopInternal();

    QMap<TagBase *, bool>::Iterator current;
    int timerId = -1;
};

QT_END_NAMESPACE

#endif // QNEARFIELDTARGET_EMULATOR_P_H
