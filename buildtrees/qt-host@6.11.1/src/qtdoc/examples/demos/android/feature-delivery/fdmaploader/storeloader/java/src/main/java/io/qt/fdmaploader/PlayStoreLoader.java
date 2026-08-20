// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

package io.qt.fdmaploader;

import com.google.android.play.core.splitcompat.SplitCompat;
import com.google.android.play.core.splitinstall.SplitInstallManager;
import com.google.android.play.core.splitinstall.SplitInstallManagerFactory;
import com.google.android.play.core.splitinstall.SplitInstallRequest;
import com.google.android.play.core.splitinstall.SplitInstallHelper;
import com.google.android.play.core.splitinstall.SplitInstallException;
import com.google.android.play.core.splitinstall.testing.FakeSplitInstallManagerFactory;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Set;
import java.io.File;
import android.util.Log;
import android.content.Context;
import android.os.Build;

/**
 * Android implementation of store loader.
 */
public class PlayStoreLoader implements PlayStoreLoaderListenerCallback
{
    private static final String TAG = "PlayStoreLoader";
    private final SplitInstallManager m_splitInstallManager;
    private HashMap<String, PlayStoreLoaderListener> m_listeners =
            new HashMap<String, PlayStoreLoaderListener>();
    private boolean m_testMode;
    private Context m_context;
    private String m_moduleName;

    public static native void stateChangedNative(String callId, int state);
    public static native void errorOccurredNative(String callId, int errorCode,
                                                  String errorMessage);
    public static native void userConfirmationRequestedNative(String callId, int errorCode,
                                                              String errorMessage);
    public static native void downloadProgressChangedNative(String callId, long bytes, long total);
    public static native void finishedNative(String callId);
    public static native void moduleUninstalledNative(String callId, int status);

    //Must be in sync with ModuleRemovalStatus enum in playstoreloader.h
    public static final int UNINSTALL_CANCELED = 0;
    public static final int UNINSTALL_FAILURE = 1;
    public static final int UNINSTALL_SUCCESS = 2;

    /**
     * Constructor
     * @param context
     */
    public PlayStoreLoader(Context context) {
        m_testMode = false;
        m_context = context;

        SplitCompat.install(context);

        m_splitInstallManager = SplitInstallManagerFactory.create(context);
    }

    /**
     * Loads Feature Delivery module from a play store.
     */
    public void installModuleFromStore(String moduleName, String callId) {
        m_moduleName = moduleName;

        registerListener(callId);

        SplitInstallRequest request =
                SplitInstallRequest.newBuilder().addModule(m_moduleName).build();
//! [Call To Split Install]
        m_splitInstallManager.startInstall(request)
                .addOnSuccessListener(sessionId -> {
                    PlayStoreLoaderListener listener = m_listeners.get(callId);
                    if (listener != null)
                        listener.setSessionId(sessionId);
                })
//! [Call To Split Install]
                .addOnFailureListener(exception -> {
                    int errorCode = -128;
                    String message = "splitInstall() failed.";
                    if (exception != null) {
                        message = exception.getMessage();
                        if (exception instanceof SplitInstallException)
                            errorCode = ((SplitInstallException) exception).getErrorCode();
                    }
                    errorOccurred(callId, errorCode, message);
                });
    }

    /**
     * Returns installed feature delivery modules.
     */
    public Set<String> getInstalledModules() {
        return m_splitInstallManager.getInstalledModules();
    }

    /**
     * Uninstall modules in the background.
     * Failure listener seems to be called on local-testing. Success on device.
     * @param modules
     */
    public void uninstallModules(String callId, List<String> modules) {
        if (m_splitInstallManager == null)
            return;
        m_splitInstallManager.deferredUninstall(modules)
                .addOnCanceledListener(() -> {
                   moduleUninstalledNative(callId, UNINSTALL_CANCELED);
                })
                .addOnFailureListener(exception -> {
                    moduleUninstalledNative(callId, UNINSTALL_FAILURE);
                })
                .addOnSuccessListener(result -> {
                    moduleUninstalledNative(callId, UNINSTALL_SUCCESS);
                });
    }

    /**
     * Attempts to cancel current module loading.
     * @param callId Identifier to the module being loaded.
     */
    public void cancelLoad(String callId) {
        if (m_splitInstallManager == null)
            return;
        PlayStoreLoaderListener listener = m_listeners.get(callId);
        if (listener != null) {
            int sessionId = listener.sessionId();
            m_splitInstallManager.cancelInstall(sessionId);
        }
    }

    /**
     * Feature delivery state has changed
     * @param callId
     * @param state New state.
     */
    public void stateChanged(String callId, int state) {
        stateChangedNative(callId, state);
        if (state == PlayStoreLoaderListenerCallback.LOADED ||
            state == PlayStoreLoaderListenerCallback.CANCELED)
            finished(callId);
    }

    /**
     * Error has occurred.
     * @param callId
     * @param errorCode One of com.google.android.play.core.splitinstall.model.SplitInstallErrorCode
     *     or -1 in the case of exception
     * @param errorMessage String describing the error.
     */
    public void errorOccurred(String callId, int errorCode, String errorMessage) {
        stateChangedNative(callId, ERROR);
        errorOccurredNative(callId, errorCode, errorMessage);
        finished(callId);
    }

    /**
     * User confirmation has been requested.
     */
    public void userConfirmationRequested(String callId, int errorCode, String errorMessage) {
        userConfirmationRequestedNative(callId, errorCode, errorMessage);
    }

    /**
     * Notification of download process.
     */
    public void downloadProgressChanged(String callId, long bytes, long total) {
        downloadProgressChangedNative(callId, bytes, total);
    }

    /**
     * Loads the module being installed.
     * @param callId
     */
    public void loadLibrary(String callId) {
        stateChangedNative(callId, PlayStoreLoaderListenerCallback.LOADING);
        // update context.
        try {
            m_context = m_context.createPackageContext(m_context.getPackageName(), 0);
        } catch (android.content.pm.PackageManager.NameNotFoundException exc) {
            errorOccurred(callId, -1, "Could not get package name");
            return;
        }
        // install splitcompat for new context.
        SplitCompat.install(m_context);
        // try to load new library
        for (String abi : Build.SUPPORTED_ABIS) {
            String libraryName = m_moduleName + "_" + abi;
            try {
                System.loadLibrary(libraryName);
                break;
            } catch (Exception e) {
                String errorString = e.toString() +"For library: " +libraryName;
                Log.e(TAG, errorString);
                errorOccurred(callId, -1, errorString);
            }
        }
        stateChanged(callId, PlayStoreLoaderListenerCallback.LOADED);
    }

    private void finished(String callId) {
        unregisterListener(callId);
        finishedNative(callId);
    }

    private void registerListener(String callId) {
        PlayStoreLoaderListener listener = new PlayStoreLoaderListener(this, callId);
        if (listener != null) {
            m_listeners.put(callId, listener);
            m_splitInstallManager.registerListener(listener);
        }
    }

    private void unregisterListener(String callId) {
        PlayStoreLoaderListener listener = m_listeners.remove(callId);
        if (listener != null)
            m_splitInstallManager.unregisterListener(listener);
    }
}
