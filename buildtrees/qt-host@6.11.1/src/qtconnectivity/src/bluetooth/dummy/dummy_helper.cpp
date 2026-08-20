// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtCore/QGlobalStatic>
#include <QtCore/QLoggingCategory>

#include "dummy_helper_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT)

Q_GLOBAL_STATIC_WITH_ARGS(bool, dummyWarningPrinted, (false))

void printDummyWarning()
{
    if (!*dummyWarningPrinted()) {
        qCWarning(QT_BT) << "Dummy backend running. Qt Bluetooth module is non-functional.";
        *dummyWarningPrinted() = true;
    }
}

QT_END_NAMESPACE
