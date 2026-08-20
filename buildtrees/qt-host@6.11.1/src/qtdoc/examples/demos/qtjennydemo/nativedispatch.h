// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifndef NATIVEDISPATCH_H
#define NATIVEDISPATCH_H

#include <QJniObject>

struct NativeInterface {
    virtual jobject qt_invoke(jstring method, jobjectArray args) = 0;
};

void registerNativeInvocationHandler();

#endif // NATIVEDISPATCH_H
