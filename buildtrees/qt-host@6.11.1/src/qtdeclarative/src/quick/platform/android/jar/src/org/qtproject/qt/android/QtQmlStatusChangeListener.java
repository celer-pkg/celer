// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt.android;

/**
 * A callback that notifies clients about the status of QML component loading.
 **/
public interface QtQmlStatusChangeListener
{
    /**
     * Called on the Android UI thread when the QML component status has changed.
     * @param status The current status. The status can be QtQmlStatus.NULL,
     *               QtQmlStatus.READY, QtQmlStatus.LOADING, or QtQmlStatus.ERROR.
     **/
    default void onStatusChanged(QtQmlStatus status) {};

    /**
     * Called on the Android UI thread when the QML component status has changed.
     * This is called only when a QtQuickViewContent is loaded using QtQuickView.loadContent().
     *
     * @param status The current status. The status can be QtQmlStatus.NULL,
     *               QtQmlStatus.READY, QtQmlStatus.LOADING, or QtQmlStatus.ERROR.
     * @param content The content of the Qt Quick view, {@code content} cannot be null.
     **/
    default void onStatusChanged(QtQmlStatus status, QtQuickViewContent content)
    {
        onStatusChanged(status);
    };
}
