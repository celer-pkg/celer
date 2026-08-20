// Copyright (C) 2024 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QREMOTEOBJECTSTRUCTS_P_H
#define QREMOTEOBJECTSTRUCTS_P_H

#include <QByteArray>
#include <QList>

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

// Simple structs used to describe types for dynamically creating and registering
// new instances.

QT_BEGIN_NAMESPACE

namespace QRemoteObjectInternalTypes {

struct EnumPair {
    QByteArray name;
    int value;
};

struct EnumData {
    QByteArray name;
    bool isFlag, isScoped;
    quint32 keyCount, size;
    QList<EnumPair> values;
};

struct GadgetProperty {
    QByteArray name;
    QByteArray type;
};

struct GadgetData {
    QList<GadgetProperty> properties;
    QList<EnumData> enums;
};

struct ClassProperty {
    QByteArray name;
    QByteArray typeName;
    QByteArray signalName;
    bool isWritable = false;
};

struct ClassSignal {
    QByteArray signature;
    QByteArrayList parameterNames;
};

struct ClassSlot {
    QByteArray signature, returnType;
    QByteArrayList parameterNames;
};

struct ClassData
{
    ClassData(bool _isSource = false);
    QByteArray type;
    QList<ClassProperty> properties;
    QList<ClassSignal> _signals;
    QList<ClassSlot> _slots;
    QList<EnumData> enums;
    bool isSource;
    const QMetaObject *baseMeta;
};

}; // QRemoteObjectInternalTypes namespace

QT_END_NAMESPACE

#endif // QREMOTEOBJECTSTRUCTS_P_H
