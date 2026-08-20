// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TARGETEMULATOR_P_H
#define TARGETEMULATOR_P_H

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

#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtNfc/qtnfcglobal.h>
#include <QMetaType>

QT_FORWARD_DECLARE_CLASS(QSettings)

QT_BEGIN_NAMESPACE

class TagBase
{
public:
    TagBase();
    virtual ~TagBase();

    virtual void load(QSettings *settings) = 0;

    virtual QByteArray processCommand(const QByteArray &command) = 0;

    virtual QByteArray uid() const = 0;

    qint64 lastAccessTime() const { return lastAccess; }

protected:
    mutable qint64 lastAccess;
};

class NfcTagType1 : public TagBase
{
public:
    NfcTagType1();
    ~NfcTagType1();

    void load(QSettings *settings) override;

    QByteArray processCommand(const QByteArray &command) override;

    QByteArray uid() const override;

private:
    quint8 readData(quint8 block, quint8 byte);

    quint8 hr0;
    quint8 hr1;

    QByteArray memory;
};

class NfcTagType2 : public TagBase
{
public:
    NfcTagType2();
    ~NfcTagType2();

    void load(QSettings *settings) override;

    QByteArray processCommand(const QByteArray &command) override;

    QByteArray uid() const override;

private:
    QByteArray memory;
    quint8 currentSector;
    bool expectPacket2;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(TagBase *)

#endif // TARGETEMULATOR_P_H
