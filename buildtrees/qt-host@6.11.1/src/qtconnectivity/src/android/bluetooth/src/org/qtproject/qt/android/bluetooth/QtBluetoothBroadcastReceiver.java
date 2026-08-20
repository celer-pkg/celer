// Copyright (C) 2016 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt.android.bluetooth;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import java.lang.reflect.Method;
import java.util.HashSet;
import java.util.List;

class QtBluetoothBroadcastReceiver extends BroadcastReceiver
{
    /* Pointer to the Qt object that "owns" the Java object */
    @SuppressWarnings("WeakerAccess")
    long qtObject = 0;
    @SuppressWarnings("WeakerAccess")
    static Context qtContext = null;

    // These are opaque tokens that could be used to match the completed action
    private static final int TURN_BT_ENABLED = 3330;
    private static final int TURN_BT_DISCOVERABLE = 3331;
    private static final int TURN_BT_DISABLED = 3332;

    // The 'Disable' action identifier is hidden in the public APIs so we define it here
    static final String ACTION_REQUEST_DISABLE =
        "android.bluetooth.adapter.action.REQUEST_DISABLE";

    private static final String TAG = "QtBluetoothBroadcastReceiver";

    @Override
    public void onReceive(Context context, Intent intent)
    {
        synchronized (qtContext) {
            if (qtObject == 0)
                return;

            jniOnReceive(qtObject, context, intent);
        }
    }

    void unregisterReceiver()
    {
        synchronized (qtContext) {
            qtObject = 0;
            try {
                qtContext.unregisterReceiver(this);
            } catch (Exception ex) {
                Log.d(TAG, "Trying to unregister a BroadcastReceiver which is not yet registered");
            }
        }
    }

    native void jniOnReceive(long qtObject, Context context, Intent intent);

    static void setContext(Context context)
    {
        qtContext = context;
    }

    static boolean setDisabled()
    {
        if (!(qtContext instanceof android.app.Activity)) {
            Log.w(TAG, "Bluetooth cannot be disabled from a service.");
            return false;
        }
        // The 'disable' is hidden in the public API and as such
        // there are no availability guarantees; may throw an "ActivityNotFoundException"
        Intent intent = new Intent(ACTION_REQUEST_DISABLE);

        try {
            ((Activity)qtContext).startActivityForResult(intent, TURN_BT_DISABLED);
        } catch (Exception ex) {
            Log.w(TAG, "setDisabled() failed to initiate Bluetooth disablement");
            ex.printStackTrace();
            return false;
        }
        return true;
    }

    static boolean setDiscoverable()
    {
        if (!(qtContext instanceof android.app.Activity)) {
            Log.w(TAG, "Discovery mode cannot be enabled from a service.");
            return false;
        }

        Intent intent = new Intent(BluetoothAdapter.ACTION_REQUEST_DISCOVERABLE);
        intent.putExtra(BluetoothAdapter.EXTRA_DISCOVERABLE_DURATION, 300);
        try {
            ((Activity)qtContext).startActivityForResult(intent, TURN_BT_DISCOVERABLE);
        } catch (Exception ex) {
            Log.w(TAG, "setDiscoverable() failed to initiate Bluetooth discoverability change");
            ex.printStackTrace();
            return false;
        }
        return true;
    }

    static boolean setEnabled()
    {
        if (!(qtContext instanceof android.app.Activity)) {
            Log.w(TAG, "Bluetooth cannot be enabled from a service.");
            return false;
        }

        Intent intent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
        try {
            ((Activity)qtContext).startActivityForResult(intent, TURN_BT_ENABLED);
        } catch (Exception ex) {
            Log.w(TAG, "setEnabled() failed to initiate Bluetooth enablement");
            ex.printStackTrace();
            return false;
        }
        return true;
    }

    static boolean setPairingMode(String address, boolean isPairing)
    {
        BluetoothManager manager =
            (BluetoothManager)qtContext.getSystemService(Context.BLUETOOTH_SERVICE);
        if (manager == null)
            return false;

        BluetoothAdapter adapter = manager.getAdapter();
        if (adapter == null)
            return false;

        // Uses reflection as the removeBond() is not part of public API
        try {
            BluetoothDevice device = adapter.getRemoteDevice(address);
            String methodName = "createBond";
            if (!isPairing)
                methodName = "removeBond";

            Method m = device.getClass()
                    .getMethod(methodName, (Class[]) null);
            m.invoke(device, (Object[]) null);
        } catch (Exception ex) {
            ex.printStackTrace();
            return false;
        }

        return true;
    }

    /*
     * Returns a list of remote devices confirmed to be connected.
     *
     * This list is not complete as it only detects GATT/BtLE related connections.
     * Unfortunately there is no API that provides the complete list.
     *
     */
    static String[] getConnectedDevices()
    {
        BluetoothManager bluetoothManager =
            (BluetoothManager) qtContext.getSystemService(Context.BLUETOOTH_SERVICE);

        if (bluetoothManager == null) {
            Log.w(TAG, "Failed to retrieve connected devices");
            return new String[0];
        }

        List<BluetoothDevice> gattConnections =
            bluetoothManager.getConnectedDevices(BluetoothProfile.GATT);
        List<BluetoothDevice> gattServerConnections =
             bluetoothManager.getConnectedDevices(BluetoothProfile.GATT_SERVER);

        // Process found remote connections but avoid duplications
        HashSet<String> set = new HashSet<String>();
        for (Object gattConnection : gattConnections)
            set.add(gattConnection.toString());

        for (Object gattServerConnection : gattServerConnections)
            set.add(gattServerConnection.toString());

        return set.toArray(new String[set.size()]);
    }
}
