// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt.android;

import android.app.Activity;
import android.content.Context;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Rect;
import android.hardware.display.DisplayManager;
import android.os.Build;
import android.util.DisplayMetrics;
import android.util.Size;
import android.view.Display;
import android.view.Surface;
import android.view.WindowManager;
import android.view.WindowMetrics;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import android.util.Log;

class QtDisplayManager
{
    private static String QtTAG = "QtDisplayManager";
    // screen methods
    static native void handleLayoutSizeChanged(int availableWidth, int availableHeight);
    static native void handleOrientationChanged(int newRotation, int nativeOrientation);
    static native void handleRefreshRateChanged(float refreshRate);
    static native void handleUiDarkModeChanged(int newUiMode);
    static native void handleScreenAdded(int displayId);
    static native void handleScreenChanged(int displayId);
    static native void handleScreenRemoved(int displayId);
    static native void handleScreenDensityChanged(double density);
    // screen methods

    private static int m_previousRotation = -1;

    private final DisplayManager.DisplayListener m_displayListener;
    private final Activity m_activity;

    QtDisplayManager(Activity activity)
    {
        m_activity = activity;
        m_displayListener = new DisplayManager.DisplayListener() {
            @Override
            public void onDisplayAdded(int displayId) {
                QtDisplayManager.handleScreenAdded(displayId);
            }

            @Override
            public void onDisplayChanged(int displayId) {
                QtDisplayManager.updateRefreshRate(m_activity);
                QtDisplayManager.updateScreenDensity(m_activity);
                QtDisplayManager.handleScreenChanged(displayId);
            }

            @Override
            public void onDisplayRemoved(int displayId) {
                QtDisplayManager.handleScreenRemoved(displayId);
            }
        };
    }

    static void updateRefreshRate(Context context)
    {
        Display display = getDisplay(context);
        float refreshRate = display != null ? display.getRefreshRate() : 60.0f;
        QtDisplayManager.handleRefreshRateChanged(refreshRate);
    }

    static void handleOrientationChange(Activity activity)
    {
        Display display = QtDisplayManager.getDisplay(activity);
        int rotation = display != null ? display.getRotation() : 0;
        if (m_previousRotation == rotation)
            return;
        int nativeOrientation = getNativeOrientation(activity, rotation);
        QtDisplayManager.handleOrientationChanged(rotation, nativeOrientation);
        m_previousRotation = rotation;
    }

    static void updateScreenDensity(Activity activity)
    {
        Resources resources = activity == null ? Resources.getSystem() : activity.getResources();
        double density = resources.getDisplayMetrics().density;
        QtDisplayManager.handleScreenDensityChanged(density);
    }

    private static int getNativeOrientation(Activity activity, int rotation)
    {
        int orientation = activity.getResources().getConfiguration().orientation;
        boolean rot90 = (rotation == Surface.ROTATION_90 || rotation == Surface.ROTATION_270);
        boolean isLandscape = (orientation == Configuration.ORIENTATION_LANDSCAPE);
        if ((isLandscape && !rot90) || (!isLandscape && rot90))
            return Configuration.ORIENTATION_LANDSCAPE;

        return Configuration.ORIENTATION_PORTRAIT;
    }

    void initDisplayProperties()
    {
        QtDisplayManager.handleOrientationChange(m_activity);
        QtDisplayManager.updateRefreshRate(m_activity);
        QtDisplayManager.updateScreenDensity(m_activity);
    }

    void registerDisplayListener()
    {
        DisplayManager displayManager =
                (DisplayManager) m_activity.getSystemService(Context.DISPLAY_SERVICE);
        displayManager.registerDisplayListener(m_displayListener, null);
    }

    void unregisterDisplayListener()
    {
        DisplayManager displayManager =
                (DisplayManager) m_activity.getSystemService(Context.DISPLAY_SERVICE);
        displayManager.unregisterDisplayListener(m_displayListener);
    }

    @SuppressWarnings("deprecation")
    static Display getDisplay(Context context)
    {
        Activity activity = (Activity) context;
        if (activity != null) {
            return (Build.VERSION.SDK_INT < Build.VERSION_CODES.R)
                        ? activity.getWindowManager().getDefaultDisplay()
                        : activity.getDisplay();
        }

        final DisplayManager dm = context.getSystemService(DisplayManager.class);
        return dm.getDisplay(Display.DEFAULT_DISPLAY);
    }

    @UsedFromNativeCode
    static Display getDisplay(Context context, int displayId)
    {
        DisplayManager displayManager =
                (DisplayManager)context.getSystemService(Context.DISPLAY_SERVICE);
        if (displayManager != null) {
            return displayManager.getDisplay(displayId);
        }
        return null;
    }

    @UsedFromNativeCode
    static List<Display> getAvailableDisplays(Context context)
    {
        DisplayManager displayManager =
                (DisplayManager)context.getSystemService(Context.DISPLAY_SERVICE);
        if (displayManager != null) {
            Display[] displays = displayManager.getDisplays();
            return Arrays.asList(displays);
        }
        return new ArrayList<>();
    }

    @UsedFromNativeCode
    @SuppressWarnings("deprecation")
    static Size getDisplaySize(Context context, Display display)
    {
        if (display == null || context == null)
            return new Size(0, 0);

        if (android.os.Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
            final DisplayMetrics metrics = new DisplayMetrics();
            display.getRealMetrics(metrics);
            return new Size(metrics.widthPixels, metrics.heightPixels);
        } else {
            try {
                Context displayContext = context.createDisplayContext(display);
                WindowManager windowManager = displayContext.getSystemService(WindowManager.class);
                if (windowManager != null) {
                    WindowMetrics metrics = windowManager.getCurrentWindowMetrics();
                    Rect areaBounds = metrics.getBounds();
                    return new Size(areaBounds.width(), areaBounds.height());
                } else {
                    Log.e(QtTAG, "getDisplaySize(): WindowManager null, display ID" + display.getDisplayId());
                }
            } catch (Exception e) {
                Log.e(QtTAG, "Failed to retrieve display metrics with " + e);
            }
            return new Size(0, 0);
        }
    }

    @UsedFromNativeCode
    static float getXDpi(final DisplayMetrics metrics) {
        if (metrics.xdpi < android.util.DisplayMetrics.DENSITY_LOW)
            return android.util.DisplayMetrics.DENSITY_LOW;
        return metrics.xdpi;
    }

    @UsedFromNativeCode
    static float getYDpi(final DisplayMetrics metrics) {
        if (metrics.ydpi < android.util.DisplayMetrics.DENSITY_LOW)
            return android.util.DisplayMetrics.DENSITY_LOW;
        return metrics.ydpi;
    }
}
