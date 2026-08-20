// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt.android.bluetooth;

import android.Manifest;
import android.os.Build;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.util.Log;

class QtBluetoothUtility {

    private static final String TAG = "QtBluetoothUtility";

    // The check if bluetooth scanning requires location is somewhat computationally
    // expensive and doesn't change at runtime, so we cache the result
    private static boolean isScanRequiresLocationChecked = false;
    private static boolean scanRequiresLocation = true;

    // Returns whether or not Location + Location permission is required for Bluetooth scan
    //
    // API-level < 31: returns always true
    //
    // API-level >= 31: returns true if BLUETOOTH_SCAN doesn't have 'neverForLocation' set,
    // or if the BLUETOOTH_SCAN permission is not present.
    // Returns false if BLUETOOTH_SCAN has 'neverForLocation' set, or in case of any
    // unexpected failure.
    public static synchronized boolean bluetoothScanRequiresLocation(final Context qtContext)
    {
        Log.d(TAG, "Checking if Location is required for bluetooth scan");
        if (isScanRequiresLocationChecked) {
            Log.d(TAG, "Using cached result for scan needing location: " + scanRequiresLocation);
            return scanRequiresLocation;
        }

        // API-levels below 31 always need location
        if (Build.VERSION.SDK_INT < 31) {
            Log.d(TAG, "SDK version is below 31, assuming Location needed");
            scanRequiresLocation = true;
            isScanRequiresLocationChecked = true;
            return scanRequiresLocation;
        }
        if (qtContext == null) {
            Log.w(TAG, "No context object provided");
            return false;
        }

        try {
            // API-levels 31+ require Location if there is no 'neverForLocation' assertion.
            // Therefore check for BLUETOOTH_SCAN and its permission flags. Note that the
            // permissions and their flags are indeed "parallel" arrays, i.e. permissions[i]
            // corresponds with permissionsFlags[i]
            PackageManager pm = qtContext.getPackageManager();
            PackageInfo pi = pm.getPackageInfo(qtContext.getPackageName(),
                                               PackageManager.GET_PERMISSIONS);
            String[] permissions = pi.requestedPermissions;
            int[] permissionsFlags = pi.requestedPermissionsFlags;

            if (permissions != null && permissionsFlags != null) {
                for (int i = 0; i < permissions.length; ++i) {
                    if (Manifest.permission.BLUETOOTH_SCAN.equals(permissions[i])) {
                        if ((permissionsFlags[i] & PackageInfo.REQUESTED_PERMISSION_NEVER_FOR_LOCATION) != 0) {
                            Log.d(TAG, "BLUETOOTH_SCAN with 'neverForLocation' found");
                            scanRequiresLocation = false;
                        } else {
                            Log.d(TAG, "BLUETOOTH_SCAN without 'neverForLocation' found");
                            scanRequiresLocation = true;
                        }
                        isScanRequiresLocationChecked = true;
                        return scanRequiresLocation;
                    }
                }
            }
        } catch (Exception ex) {
            Log.w(TAG, "An error occurred while checking Bluetooth's location need: " + ex);
            scanRequiresLocation = false;
        }
        Log.d(TAG, "BLUETOOTH_SCAN permission not found");
        isScanRequiresLocationChecked = true;
        return scanRequiresLocation;
    }
}
