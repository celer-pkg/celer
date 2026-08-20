// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

package org.qtproject.example.android_dynamic_feature;

import com.google.android.play.core.splitinstall.SplitInstallStateUpdatedListener;
import com.google.android.play.core.splitinstall.SplitInstallSessionState;
import com.google.android.play.core.splitinstall.model.SplitInstallSessionStatus;

import android.util.Log;

public class StoreLoaderListener implements SplitInstallStateUpdatedListener {

    private final StoreLoaderListenerCallback m_callback;
    private final String m_callId;
    private int m_installSessionId = -1;
    private static final String TAG = "StoreLoaderListener";
    private int m_currentStatus = SplitInstallSessionStatus.UNKNOWN;
    private boolean m_postponeCancel = false;

    public StoreLoaderListener(StoreLoaderListenerCallback callback, String callId) {
        m_callback = callback;
        m_callId = callId;
    }

    public void setSessionId(int sessionId) { m_installSessionId = sessionId; }
    public int getSessionId() { return m_installSessionId; }

    public void postponeCancel() { m_postponeCancel = true; }
    public boolean isCancelled() { return m_postponeCancel; }

    @Override
    public void onStateUpdate(SplitInstallSessionState state) {
        Log.d(TAG,
              "onStateUpdate, status: " + state.status() + " session id: " + state.sessionId());
        if (state.sessionId() != m_installSessionId) {
            // Not mine
            return;
        }

        switch (state.status()) {
        case SplitInstallSessionStatus.DOWNLOADING:
            Log.d(TAG,
                  "SplitInstallSessionState: DOWNLOADING " + state.bytesDownloaded() + "/"
                          + state.totalBytesToDownload());
            if (m_currentStatus != SplitInstallSessionStatus.DOWNLOADING)
                m_callback.onStateChanged(m_callId, StoreLoaderListenerCallback.DOWNLOADING);

            m_callback.onDownloadProgressChanged(m_callId, state.bytesDownloaded(),
                                                 state.totalBytesToDownload());
            break;
        case SplitInstallSessionStatus.REQUIRES_USER_CONFIRMATION:
            Log.d(TAG, "SplitInstallSessionState: REQUIRES_USER_CONFIRMATION");
            m_callback.onStateChanged(m_callId,
                                    StoreLoaderListenerCallback.REQUIRES_USER_CONFIRMATION);

            m_callback.onUserConfirmationRequested(m_callId, state.errorCode(), "" /*noop for now*/);
            break;
        case SplitInstallSessionStatus.INSTALLED:
            Log.d(TAG, "SplitInstallSessionState: INSTALLED");
            m_callback.onStateChanged(m_callId, StoreLoaderListenerCallback.INSTALLED);
            m_callback.onLoadLibrary(m_callId);
            break;
        case SplitInstallSessionStatus.INSTALLING:
            Log.d(TAG, "SplitInstallSessionState: INSTALLING");
            m_callback.onStateChanged(m_callId, StoreLoaderListenerCallback.INSTALLING);
            break;
        case SplitInstallSessionStatus.FAILED:
            Log.d(TAG,
                  "SplitInstallSessionState: FAILED with error code: " + state.errorCode());
            m_callback.onErrorOccurred(m_callId, state.errorCode(), "" /*noop for now*/);
            break;
        case SplitInstallSessionStatus.PENDING:
            Log.d(TAG, "SplitInstallSessionState: PENDING");
            m_callback.onStateChanged(m_callId, StoreLoaderListenerCallback.PENDING);
            break;
        case SplitInstallSessionStatus.DOWNLOADED:
            Log.d(TAG, "SplitInstallSessionState: DOWNLOADED");
            m_callback.onStateChanged(m_callId, StoreLoaderListenerCallback.DOWNLOADED);
            break;
        case SplitInstallSessionStatus.CANCELED:
            Log.d(TAG, "SplitInstallSessionState: CANCELED");
            m_callback.onStateChanged(m_callId, StoreLoaderListenerCallback.CANCELED);
            break;
        case SplitInstallSessionStatus.CANCELING:
            Log.d(TAG, "SplitInstallSessionState: CANCELING");
            m_callback.onStateChanged(m_callId, StoreLoaderListenerCallback.CANCELING);
            break;
        }
        m_currentStatus = state.status();
    }
}
