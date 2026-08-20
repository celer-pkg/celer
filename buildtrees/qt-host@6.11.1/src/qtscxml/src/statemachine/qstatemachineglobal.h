// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:insignificant

#ifndef QSTATEMACHINEGLOBAL_H
#define QSTATEMACHINEGLOBAL_H

#include <QtCore/qglobal.h>

#if defined(BUILD_QSTATEMACHINE)
#  define Q_STATEMACHINE_EXPORT
#else
#  include <QtStateMachine/qtstatemachineexports.h>
#endif

#endif // QSTATEMACHINEGLOBAL_H
