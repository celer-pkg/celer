// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
package org.qtproject.qtjennydemo;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.util.Objects;

public class NativeInvocationHandler implements InvocationHandler {
    private final long nativeHandle;

    public NativeInvocationHandler(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    public int hashCode() {
        return Objects.hash(nativeHandle);
    }

    private native Object invokeNative(long nativeHandle, String method, Object[] args);

    public Object invoke(String method, Object[] args) {
        return invokeNative(nativeHandle, method, args);
    }

    @Override
    public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
        return invokeNative(nativeHandle, method.getName(), args);
    }
}
