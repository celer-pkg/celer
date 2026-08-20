// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QUICK3D_PHYSICS_TEST_UTIL_H
#define QUICK3D_PHYSICS_TEST_UTIL_H

#include <QtGlobal>
#include <QtCore/qstring.h>

using namespace Qt::StringLiterals;

QString needSkip() {
    if (!qEnvironmentVariableIsEmpty("QEMU_LD_PREFIX"))
        return "This test is unstable on QEMU, so it will be skipped."_L1;

    auto platform = qEnvironmentVariable("QT_QPA_PLATFORM");
    if (platform == "offscreen"_L1 || platform == "minimal"_L1)
        return "This test doesn't work on offscreen or minimal, so it will be skipped."_L1;

    return ""_L1;
}

#endif
