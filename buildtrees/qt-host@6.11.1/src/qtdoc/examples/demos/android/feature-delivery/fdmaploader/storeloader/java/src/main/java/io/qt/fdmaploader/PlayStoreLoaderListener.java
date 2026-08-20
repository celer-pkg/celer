// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

package io.qt.fdmaploader;

import com.google.android.play.core.splitinstall.SplitInstallStateUpdatedListener;
import com.google.android.play.core.splitinstall.SplitInstallSessionState;
import com.google.android.play.core.splitinstall.model.SplitInstallSessionStatus;

import android.util.Log;

public class PlayStoreLoaderListener implements SplitInstallStateUpdatedListener {

    private final PlayStoreLoaderListenerCallback m_callback;
    private final String m_callId;
    private int m_installSessionId = -1;
    private static final String TAG = "PlayStoreLoaderListener";
    private int m_currentStatus = SplitInstallSessionStatus.UNKNOWN;

    public PlayStoreLoaderListener(PlayStoreLoaderListenerCallback callback, String callId) {
        m_callback = callback;
        m_callId = callId;
    }

    public void setSessionId(int sessionId) { m_installSessionId = sessionId; }

    public int sessionId() { return m_installSessionId; }

    @Override public void onStateUpdate(SplitInstallSessionState state) {
        if (state.sessionId() != m_installSessionId) {
            // Not mine
            return;
        }

        switch (state.status()) {
        case SplitInstallSessionStatus.DOWNLOADING:
            if (m_currentStatus != SplitInstallSessionStatus.DOWNLOADING)
                m_callback.stateChanged(m_callId, PlayStoreLoaderListenerCallback.DOWNLOADING);
            m_callback.downloadProgressChanged(m_callId, state.bytesDownloaded(),
                    state.totalBytesToDownload());
            break;
        case SplitInstallSessionStatus.REQUIRES_USER_CONFIRMATION:
            m_callback.stateChanged(m_callId,
                    PlayStoreLoaderListenerCallback.REQUIRES_USER_CONFIRMATION);
            m_callback.userConfirmationRequested(m_callId, state.errorCode(),
                    "" /*noop by now*/);
            break;
        case SplitInstallSessionStatus.INSTALLED:
            m_callback.stateChanged(m_callId, PlayStoreLoaderListenerCallback.INSTALLED);
            m_callback.loadLibrary(m_callId);
            break;
        case SplitInstallSessionStatus.INSTALLING:
            m_callback.stateChanged(m_callId, PlayStoreLoaderListenerCallback.INSTALLING);
            break;
        case SplitInstallSessionStatus.FAILED:
            m_callback.errorOccurred(m_callId, state.errorCode(), "" /*noop by now*/);
            break;
        case SplitInstallSessionStatus.PENDING:
            m_callback.stateChanged(m_callId, PlayStoreLoaderListenerCallback.PENDING);
            break;
        case SplitInstallSessionStatus.DOWNLOADED:
            m_callback.stateChanged(m_callId, PlayStoreLoaderListenerCallback.DOWNLOADED);
            break;
        case SplitInstallSessionStatus.CANCELED:
            m_callback.stateChanged(m_callId, PlayStoreLoaderListenerCallback.CANCELED);
            break;
        case SplitInstallSessionStatus.CANCELING:
            m_callback.stateChanged(m_callId, PlayStoreLoaderListenerCallback.CANCELING);
            break;
        }
        m_currentStatus = state.status();
    }
}
