// Copyright (C) 2024 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:declarations-only

#ifndef QREMOTEOBJECTREPPARSER_P_H
#define QREMOTEOBJECTREPPARSER_P_H

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

#include "repparser.h"
#include "qtremoteobjectglobal.h"

#include <QMetaObject>

QT_BEGIN_NAMESPACE

/*
 * These methods use a POD or Class definition from RepParser to dynamically create QMetaObject
 * definitions and (for PODs and enums) register them with Qt's type system.  This requires memory
 * allocation, so cleanup is handled by passing a QObject pointer as a reference object.  When the
 * QObject is destroyed, memory will be freed.  This supports multiple QObjects referring to the
 * same type, so the addTracker method is provided to set additional QObjects as references.  Memory
 * will be freed only when the last reference is destroyed.
 *
 * If creation fails, a nullptr is returned.
*/

Q_REMOTEOBJECTS_EXPORT QMetaObject *createAndRegisterMetaTypeFromPOD(const POD &pod, QObject *reference);
Q_REMOTEOBJECTS_EXPORT QMetaObject *createAndRegisterReplicaFromASTClass(const ASTClass &astClass, QObject *reference);
Q_REMOTEOBJECTS_EXPORT QMetaObject *createAndRegisterSourceFromASTClass(const ASTClass &astClass, QObject *reference);

Q_REMOTEOBJECTS_EXPORT bool addTracker(const QByteArray &typeName, QObject *reference);

QT_END_NAMESPACE

#endif // QREMOTEOBJECTREPPARSER_P_H
