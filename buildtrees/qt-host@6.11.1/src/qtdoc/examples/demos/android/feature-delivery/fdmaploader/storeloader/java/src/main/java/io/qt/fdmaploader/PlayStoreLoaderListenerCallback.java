// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

package io.qt.fdmaploader;

public interface PlayStoreLoaderListenerCallback {
    // Must be in sync with State enum in playstoreloader.h
    public static final int UNINITIALIZED = 0; //used only in native side.
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

    public void stateChanged(String callId, int state);
    public void errorOccurred(String callId, int errorCode, String errorMessage);
    public void userConfirmationRequested(String callId, int errorCode, String errorMessage);
    public void downloadProgressChanged(String callId, long bytes, long total);
    public void loadLibrary(String callId);
}
