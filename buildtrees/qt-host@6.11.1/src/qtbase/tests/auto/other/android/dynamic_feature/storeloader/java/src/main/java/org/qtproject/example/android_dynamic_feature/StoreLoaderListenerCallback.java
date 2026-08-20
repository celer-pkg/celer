// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

package org.qtproject.example.android_dynamic_feature;

public interface StoreLoaderListenerCallback {
    public static final int UNKNOWN = 0; // Unused but native part has and uses one.
    public static final int INITIALIZED = 1;
    public static final int PENDING = 2;
    public static final int DOWNLOADING = 3;
    public static final int DOWNLOADED = 4;
    public static final int REQUIRES_USER_CONFIRMATION = 5;
    public static final int CANCELING = 6;
    public static final int CANCELED = 7;
    public static final int INSTALLING = 8;
    public static final int INSTALLED = 9;
    public static final int LOADING = 10;
    public static final int LOADED = 11;
    public static final int ERROR = 12;

    public void onStateChanged(String callId, int state);
    public void onErrorOccurred(String callId, int errorCode, String errorMessage);
    public void onUserConfirmationRequested(String callId, int errorCode, String errorMessage);
    public void onDownloadProgressChanged(String callId, long bytes, long total);
    public void onLoadLibrary(String callId);
}
