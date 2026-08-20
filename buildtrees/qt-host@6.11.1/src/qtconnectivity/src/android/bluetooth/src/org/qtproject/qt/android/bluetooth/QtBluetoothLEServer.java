// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt.android.bluetooth;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.content.Context;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattServerCallback;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseData;
import android.bluetooth.le.AdvertiseSettings;
import android.bluetooth.le.BluetoothLeAdvertiser;
import android.os.Build;
import android.util.Log;
import android.util.Pair;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;
import java.util.HashMap;
import java.util.UUID;

class QtBluetoothLEServer {
    private static final String TAG = "QtBluetoothGattServer";

    /* Pointer to the Qt object that "owns" the Java object */
    @SuppressWarnings({"CanBeFinal", "WeakerAccess"})
    long qtObject = 0;
    @SuppressWarnings("WeakerAccess")

    private Context qtContext = null;

    // Bluetooth members
    private BluetoothAdapter mBluetoothAdapter = null;
    private BluetoothManager mBluetoothManager = null;
    private BluetoothGattServer mGattServer = null;
    private BluetoothLeAdvertiser mLeAdvertiser = null;

    private ArrayList<BluetoothGattService> mPendingServiceAdditions =
                      new ArrayList<BluetoothGattService>();

    private String mRemoteName = "";
    // This function is called from Qt thread
    synchronized String remoteName() {
        return mRemoteName;
    }

    private String mRemoteAddress = "";
    // This function is called from Qt thread
    synchronized String remoteAddress() {
        return mRemoteAddress;
    }

    // BT Core v5.3, 5.2.1, Vol 3, Part G
    private static final int DEFAULT_LE_ATT_MTU = 23;
    // Holds the currently supported/used MTU
    private int mSupportedMtu = DEFAULT_LE_ATT_MTU;
    // Implementation defined limit
    private static final int MAX_PENDING_WRITE_COUNT = 1024;
    // BT Core v5.3, 3.4.6.1, Vol 3, Part F
    private static final int GATT_ERROR_PREPARE_QUEUE_FULL = 0x9;
    // BT Core v5.3, 3.2.9, Vol 3, Part F
    private static final int BTLE_MAX_ATTRIBUTE_VALUE_SIZE = 512;

    // The class stores queued writes from the remote device. The writes are
    // executed later when instructed to do so by onExecuteWrite() callback.
    private class WriteEntry {
        WriteEntry(BluetoothDevice remoteDevice, Object target) {
                this.remoteDevice = remoteDevice;
                this.target = target;
                this.writes = new ArrayList<Pair<byte[], Integer>>();
        }
        // Returns true if this is a proper entry for given device + target
        boolean match(BluetoothDevice device, Object target) {
            return remoteDevice.equals(device) && target.equals(target);
        }
        final BluetoothDevice remoteDevice; // Device that issued the writes
        final Object target; // Characteristic or Descriptor
        final List<Pair<byte[], Integer>> writes; // Value, offset
    }
    private final List<WriteEntry> mPendingPreparedWrites = new ArrayList<>();

    // Helper function to clear the pending writes of a remote device. If the provided device
    // is null, all writes are cleared
    private void clearPendingPreparedWrites(Object device) {
        if (device == null)
            mPendingPreparedWrites.clear();
        ListIterator<WriteEntry> iterator = mPendingPreparedWrites.listIterator();
        while (iterator.hasNext()) {
            if (iterator.next().remoteDevice.equals(device))
                iterator.remove();
        }
    }

    // The function adds a 'prepared write' entry to target's queue. If the "target + device"
    // didn't have a queue before (this being the first write), the queue is created.
    // Targets must be either descriptors or characteristics.
    private int addPendingPreparedWrite(BluetoothDevice device, Object target,
                                        int offset, byte[] value) {
        WriteEntry entry = null;
        int currentWriteCount = 0;

        // Try to find an existing matching entry. Also while looping, count
        // the total number of writes so far in order to know if we exceed the
        // write queue size we have set for ourselves
        for (WriteEntry e : mPendingPreparedWrites) {
            if (e.match(device, target))
                entry = e;
            currentWriteCount += e.writes.size();
        }

        // BT Core v5.3, 3.4.6.1, Vol 3, Part F
        if (currentWriteCount > MAX_PENDING_WRITE_COUNT) {
            Log.w(TAG, "Prepared write queue is full, returning an error.");
            return GATT_ERROR_PREPARE_QUEUE_FULL;
        }

        // If no matching entry, create a new one. This means this is the first prepared
        // write request to this "device + target" combination
        if (entry == null)
            mPendingPreparedWrites.add(entry = new WriteEntry(device, target));

        // Append the newly received chunk of data along with its offset
        entry.writes.add(new Pair<byte[], Integer>(value, offset));
        return BluetoothGatt.GATT_SUCCESS;
    }

    /*
        As per Bluetooth specification each connected device can have individual and persistent
        Client characteristic configurations (see Bluetooth Spec 5.0 Vol 3 Part G 3.3.3.3)
        This class manages the existing configurrations.
     */
    private class ClientCharacteristicManager {
        private final HashMap<BluetoothGattCharacteristic, List<Entry>> notificationStore = new HashMap<BluetoothGattCharacteristic, List<Entry>>();

        private class Entry {
            BluetoothDevice device = null;
            byte[] value = null;
            boolean isConnected = false;
        }

        void insertOrUpdate(BluetoothGattCharacteristic characteristic,
                              BluetoothDevice device, byte[] newValue)
        {
            if (notificationStore.containsKey(characteristic)) {

                List<Entry> entries = notificationStore.get(characteristic);
                for (int i = 0; i < entries.size(); i++) {
                    if (entries.get(i).device.equals(device)) {
                        Entry e = entries.get(i);
                        e.value = newValue;
                        entries.set(i, e);
                        return;
                    }
                }

                // not match so far -> add device to list
                Entry e = new Entry();
                e.device = device;
                e.value = newValue;
                e.isConnected = true;
                entries.add(e);
                return;
            }

            // new characteristic
            Entry e = new Entry();
            e.device = device;
            e.value = newValue;
            e.isConnected = true;
            List<Entry> list = new LinkedList<Entry>();
            list.add(e);
            notificationStore.put(characteristic, list);
        }

        /*
            Marks client characteristic configuration entries as (in)active based the associated
            devices general connectivity state.
            This function avoids that existing configurations are not acted
            upon when the associated device is not connected.
         */
        void markDeviceConnectivity(BluetoothDevice device, boolean isConnected)
        {
            final Iterator<BluetoothGattCharacteristic> keys = notificationStore.keySet().iterator();
            while (keys.hasNext()) {
                final BluetoothGattCharacteristic characteristic = keys.next();
                final List<Entry> entries = notificationStore.get(characteristic);
                if (entries == null)
                    continue;

                ListIterator<Entry> charConfig = entries.listIterator();
                while (charConfig.hasNext()) {
                    Entry e = charConfig.next();
                    if (e.device.equals(device))
                        e.isConnected = isConnected;
                }
            }
        }

        // Returns list of all BluetoothDevices which require notification or indication.
        // No match returns an empty list.
        List<BluetoothDevice> getToBeUpdatedDevices(BluetoothGattCharacteristic characteristic)
        {
            ArrayList<BluetoothDevice> result = new ArrayList<BluetoothDevice>();
            if (!notificationStore.containsKey(characteristic))
                return result;

            final ListIterator<Entry> iter = notificationStore.get(characteristic).listIterator();
            while (iter.hasNext())
                result.add(iter.next().device);

            return result;
        }

        // Returns null if no match; otherwise the configured actual client characteristic
        // configuration value
        byte[] valueFor(BluetoothGattCharacteristic characteristic, BluetoothDevice device)
        {
            if (!notificationStore.containsKey(characteristic))
                return null;

            List<Entry> entries = notificationStore.get(characteristic);
            for (int i = 0; i < entries.size(); i++) {
                final Entry entry = entries.get(i);
                if (entry.device.equals(device) && entry.isConnected == true)
                    return entries.get(i).value;
            }

            return null;
        }
    }

    private static final UUID CLIENT_CHARACTERISTIC_CONFIGURATION_UUID = UUID
            .fromString("00002902-0000-1000-8000-00805f9b34fb");
    ClientCharacteristicManager clientCharacteristicManager = new ClientCharacteristicManager();

    QtBluetoothLEServer(Context context)
    {
        qtContext = context;
        if (qtContext == null) {
            Log.w(TAG, "Missing context object. Peripheral role disabled.");
            return;
        }

        mBluetoothManager =
            (BluetoothManager) qtContext.getSystemService(Context.BLUETOOTH_SERVICE);
        if (mBluetoothManager == null) {
            Log.w(TAG, "Bluetooth service not available. Peripheral role disabled.");
            return;
        }

        mBluetoothAdapter = mBluetoothManager.getAdapter();
        if (mBluetoothAdapter == null) {
            Log.w(TAG, "Missing Bluetooth adapter. Peripheral role disabled.");
            return;
        }

        Log.w(TAG, "Let's do BTLE Peripheral.");
    }

    // The following functions are synchronized callback handlers. The callbacks
    // from Android are forwarded to these methods to synchronize member variable
    // access with other threads (the Qt thread's JNI calls in particular).
    //
    // We use a single lock object (this server) for simplicity because:
    // - Some variables may change and would thus not be suitable as locking objects but
    //   would require their own additional objects => overhead
    // - Many accesses to shared variables are infrequent and the code paths are fast and
    //   deterministic meaning that long "wait times" on a lock should not happen
    // - Typically several shared variables are accessed in a single code block.
    //   If each variable would be protected individually, the amount of (nested) locking
    //   would become quite unreasonable

    synchronized void handleOnConnectionStateChange(BluetoothDevice device,
                                                           int status, int newState)
    {
        if (mGattServer == null) {
            Log.w(TAG, "Ignoring connection state event, server is disconnected");
            return;
        }
        // Multiple GATT devices may be connected. Check if we still have connected
        // devices or not, and set the server state accordingly. Note: it seems we get
        // notifications from all GATT clients, not just from the ones interested in
        // the services provided by this BT LE Server. Furthermore the list of
        // currently connected devices does not appear to be in any particular order.
        List<BluetoothDevice> connectedDevices =
            mBluetoothManager.getConnectedDevices(BluetoothProfile.GATT_SERVER);
        Log.w(TAG, "Device " + device + " connection state: " + newState + ", status: "
              + status + ", connected devices: " + connectedDevices);
        // 0 == QLowEnergyController::UnconnectedState
        // 2 == QLowEnergyController::ConnectedState
        int qtControllerState = connectedDevices.size() > 0 ? 2 : 0;

        switch (newState) {
            case BluetoothProfile.STATE_CONNECTED:
                clientCharacteristicManager.markDeviceConnectivity(device, true);
                mRemoteName = device.getName();
                mRemoteAddress = device.getAddress();
                break;
            case BluetoothProfile.STATE_DISCONNECTED:
                clientCharacteristicManager.markDeviceConnectivity(device, false);
                clearPendingPreparedWrites(device);
                // Update the remoteAddress and remoteName if needed
                if (device.getAddress().equals(mRemoteAddress)
                        && !connectedDevices.isEmpty()) {
                    mRemoteName = connectedDevices.get(0).getName();
                    mRemoteAddress = connectedDevices.get(0).getAddress();
                }
                break;
            default:
                // According to the API doc of this callback this should not happen
                Log.w(TAG, "Unhandled connection state change: " + newState);
                return;
        }

        // If last client disconnected, close down the server
        if (qtControllerState == 0) { // QLowEnergyController::UnconnectedState
            mPendingServiceAdditions.clear();
            mGattServer.close();
            mGattServer = null;
            mRemoteName = "";
            mRemoteAddress = "";
            mSupportedMtu = DEFAULT_LE_ATT_MTU;
        }

        int qtErrorCode;
        switch (status) {
            case BluetoothGatt.GATT_SUCCESS:
                qtErrorCode = 0;
                break;
            default:
                Log.w(TAG, "Unhandled error code on peripheral connectionStateChanged: "
                            + status + " " + newState);
                qtErrorCode = status;
                break;
        }

        leConnectionStateChange(qtObject, qtErrorCode, qtControllerState);
    }

    synchronized void handleOnServiceAdded(int status, BluetoothGattService service)
    {
        if (mGattServer == null) {
            Log.w(TAG, "Ignoring service addition event, server is disconnected");
            return;
        }

        Log.d(TAG, "Service " + service.getUuid().toString() + " addition result: " + status);

        // Remove the indicated service from the pending queue
        ListIterator<BluetoothGattService> iterator = mPendingServiceAdditions.listIterator();
        while (iterator.hasNext()) {
            if (iterator.next().getUuid().equals(service.getUuid())) {
                iterator.remove();
                break;
            }
        }

        // If there are more services in the queue, add the next whose add initiation succeeds
        iterator = mPendingServiceAdditions.listIterator();
        while (iterator.hasNext()) {
            BluetoothGattService nextService = iterator.next();
            if (mGattServer.addService(nextService)) {
                break;
            } else {
                Log.w(TAG, "Adding service " + nextService.getUuid().toString() + " failed");
                iterator.remove();
            }
        }
    }

    synchronized void handleOnCharacteristicReadRequest(BluetoothDevice device,
                                                   int requestId, int offset,
                                                   BluetoothGattCharacteristic characteristic)
    {
        if (mGattServer == null) {
            Log.w(TAG, "Ignoring characteristic read, server is disconnected");
            return;
        }

        byte[] characteristicData =
                                ((QtBluetoothGattCharacteristic)characteristic).getLocalValue();

        try {
            byte[] dataArray = Arrays.copyOfRange(characteristicData,
                                                  offset, characteristicData.length);
            mGattServer.sendResponse(device, requestId, BluetoothGatt.GATT_SUCCESS,
                                     offset, dataArray);
        } catch (Exception ex) {
            Log.w(TAG, "onCharacteristicReadRequest: " + requestId + " "
                        + offset + " " + characteristicData.length);
            ex.printStackTrace();
            mGattServer.sendResponse(device, requestId, BluetoothGatt.GATT_FAILURE,
                                     offset, null);
        }
    }

    synchronized void handleOnCharacteristicWriteRequest(BluetoothDevice device,
                                                   int requestId,
                                                   BluetoothGattCharacteristic characteristic,
                                                   boolean preparedWrite, boolean responseNeeded,
                                                   int offset, byte[] value)
    {
        if (mGattServer == null) {
            Log.w(TAG, "Ignoring characteristic write, server is disconnected");
            return;
        }
        Log.w(TAG, "onCharacteristicWriteRequest " + preparedWrite + " " + offset + " "
                    + value.length);
        final int minValueLen = ((QtBluetoothGattCharacteristic)characteristic).minValueLength;
        final int maxValueLen = ((QtBluetoothGattCharacteristic)characteristic).maxValueLength;

        int resultStatus = BluetoothGatt.GATT_SUCCESS;
        boolean sendNotificationOrIndication = false;

        if (!preparedWrite) { // regular write
            // User may have defined minimum and maximum size for the value, which
            // we enforce here. If the user has not defined these limits, the default
            // values 0..INT_MAX do not limit anything.
            if (value.length < minValueLen || value.length > maxValueLen) {
                // BT Core v 5.3, 4.9.3, Vol 3, Part G
                Log.w(TAG, "onCharacteristicWriteRequest invalid char value length: "
                      + value.length + ", min: " + minValueLen + ", max: " + maxValueLen);
                resultStatus = BluetoothGatt.GATT_INVALID_ATTRIBUTE_LENGTH;
            } else if (offset == 0) {
                ((QtBluetoothGattCharacteristic)characteristic).setLocalValue(value);
                leServerCharacteristicChanged(qtObject, characteristic, value);
                sendNotificationOrIndication = true;
            } else {
                // This should not really happen as per Bluetooth spec
                Log.w(TAG, "onCharacteristicWriteRequest: !preparedWrite, offset "
                            + offset + ", Not supported");
                resultStatus = BluetoothGatt.GATT_INVALID_OFFSET;
            }
        } else {
            // BT Core v5.3, 3.4.6, Vol 3, Part F
            // This is a prepared write which is used to write characteristics larger than
            // MTU. We need to record all requests and execute them in one go once
            // onExecuteWrite() is received. We use a queue to remember the pending
            // requests.
            resultStatus = addPendingPreparedWrite(device, characteristic, offset, value);
        }

        if (responseNeeded)
            mGattServer.sendResponse(device, requestId, resultStatus, offset, value);
        if (sendNotificationOrIndication)
            sendNotificationsOrIndications(characteristic);
    }

    synchronized void handleOnDescriptorReadRequest(BluetoothDevice device, int requestId,
                                              int offset, BluetoothGattDescriptor descriptor)
    {
        if (mGattServer == null) {
            Log.w(TAG, "Ignoring descriptor read, server is disconnected");
            return;
        }

        byte[] dataArray = ((QtBluetoothGattDescriptor)descriptor).getLocalValue();

        try {
            if (descriptor.getUuid().equals(CLIENT_CHARACTERISTIC_CONFIGURATION_UUID)) {
                dataArray = clientCharacteristicManager.valueFor(
                            descriptor.getCharacteristic(), device);
                if (dataArray == null)
                    dataArray = ((QtBluetoothGattDescriptor)descriptor).getLocalValue();
            }

            dataArray = Arrays.copyOfRange(dataArray, offset, dataArray.length);
            mGattServer.sendResponse(device, requestId, BluetoothGatt.GATT_SUCCESS,
                                     offset, dataArray);
        } catch (Exception ex) {
            Log.w(TAG, "onDescriptorReadRequest: " + requestId + " "
                                                   + offset + " " + dataArray.length);
            ex.printStackTrace();
            mGattServer.sendResponse(device, requestId, BluetoothGatt.GATT_FAILURE,
                                     offset, null);
        }
    }

    synchronized void handleOnDescriptorWriteRequest(BluetoothDevice device, int requestId,
                                     BluetoothGattDescriptor descriptor, boolean preparedWrite,
                                     boolean responseNeeded, int offset, byte[] value)
    {
        if (mGattServer == null) {
            Log.w(TAG, "Ignoring descriptor write, server is disconnected");
            return;
        }

        Log.w(TAG, "onDescriptorWriteRequest " + preparedWrite + " " + offset + " " + value.length);
        int resultStatus = BluetoothGatt.GATT_SUCCESS;

        if (!preparedWrite) { // regular write
            if (offset == 0) {
                if (descriptor.getUuid().equals(CLIENT_CHARACTERISTIC_CONFIGURATION_UUID)) {
                    // If both IND and NTF are requested, resort to NTF only. BT
                    // specification does not prohibit nor mention using both, but it is
                    // unlikely what the client intended. Stack behaviours vary;
                    // Apple client-side stack does not allow this, while Bluez client-side
                    // stack erroneously sends this even if the developer only asked for
                    // the other. The 0x03 value is a bitwise combination of 0x01 and 0x02
                    // as per specification: BT Core v5.3, 3.3.3.3, Vol 3, Part G
                    if (value[0] == 0x03) {
                        Log.w(TAG, "Warning: In CCC of characteristic: "
                              + descriptor.getCharacteristic().getUuid()
                              + " enabling both NTF & IND requested, enabling NTF only.");
                        value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE;
                    }
                    clientCharacteristicManager.insertOrUpdate(
                                                descriptor.getCharacteristic(),
                                                device, value);
                }
                ((QtBluetoothGattDescriptor)descriptor).setLocalValue(value);
                leServerDescriptorWritten(qtObject, descriptor, value);
            } else {
                // This should not really happen as per Bluetooth spec
                Log.w(TAG, "onDescriptorWriteRequest: !preparedWrite, offset "
                           + offset + ", Not supported");
                resultStatus = BluetoothGatt.GATT_REQUEST_NOT_SUPPORTED;
            }
        } else {
            // BT Core v5.3, 3.4.6, Vol 3, Part F
            // This is a prepared write which is used to write descriptors larger than MTU.
            // We need to record all requests and execute them in one go once
            // onExecuteWrite() is received. We use a queue to remember the pending
            // requests.
            resultStatus = addPendingPreparedWrite(device, descriptor, offset, value);
        }

        if (responseNeeded)
            mGattServer.sendResponse(device, requestId, resultStatus, offset, value);
    }

    synchronized void handleOnExecuteWrite(BluetoothDevice device,
                                            int requestId, boolean execute)
    {
        if (mGattServer == null) {
            Log.w(TAG, "Ignoring execute write, server is disconnected");
            return;
        }

        Log.w(TAG, "onExecuteWrite " + device + " " + requestId + " " + execute);

        if (execute) {
            // BT Core v5.3, 3.4.6.3, Vol 3, Part F
            // Execute all pending prepared writes for the provided 'device'
            for (WriteEntry entry : mPendingPreparedWrites) {
                if (!entry.remoteDevice.equals(device))
                    continue;

                byte[] newValue = null;
                // The target can be a descriptor or a characteristic
                byte[] currentValue = (entry.target instanceof BluetoothGattCharacteristic)
                                    ? ((QtBluetoothGattCharacteristic)entry.target).getLocalValue()
                                    : ((QtBluetoothGattDescriptor)entry.target).getLocalValue();

                // Iterate writes and apply them to the currentValue in received order
                for (Pair<byte[], Integer> write : entry.writes) {
                    // write.first is data, write.second.intValue() is offset. Check
                    // that the offset is not beyond the length of the current value
                    if (write.second.intValue() > currentValue.length) {
                        clearPendingPreparedWrites(device);
                        // BT Core v5.3, 3.4.6.3, Vol 3, Part F
                        mGattServer.sendResponse(device, requestId,
                                                 BluetoothGatt.GATT_INVALID_OFFSET,
                                                 0, null);
                        return;
                    }

                    // User may have defined value minimum and maximum sizes for
                    // characteristics, which we enforce here. If the user has not defined
                    // these limits, the default values 0..INT_MAX do not limit anything.
                    // The value size cannot decrease in prepared write (small write is a
                    // partial update) => no check for the minimum size limit here.
                    if (entry.target instanceof QtBluetoothGattCharacteristic &&
                            (write.second.intValue() + write.first.length >
                            ((QtBluetoothGattCharacteristic)entry.target).maxValueLength)) {
                        clearPendingPreparedWrites(device);
                        // BT Core v5.3, 3.4.6.3, Vol 3, Part F
                        mGattServer.sendResponse(device, requestId,
                                    BluetoothGatt.GATT_INVALID_ATTRIBUTE_LENGTH,
                                    0, null);
                        return;
                    }

                    // Determine the size of the new value as we may be extending the
                    // current value size
                    newValue = new byte[Math.max(write.second.intValue() +
                                                 write.first.length, currentValue.length)];
                    // Copy the current value to the newValue. We can't use the currentValue
                    // directly because the length of value might increase by this write
                    System.arraycopy(currentValue, 0, newValue, 0, currentValue.length);
                    // Apply this iteration's write to the newValue
                    System.arraycopy(write.first, 0, newValue, write.second.intValue(),
                                     write.first.length);
                    // Update the currentValue as there may be more writes to apply
                    currentValue = newValue;
                }

                // Update value and inform the Qt/C++ side on the update
                if (entry.target instanceof BluetoothGattCharacteristic) {
                    ((QtBluetoothGattCharacteristic)entry.target).setLocalValue(newValue);
                    leServerCharacteristicChanged(
                        qtObject, (BluetoothGattCharacteristic)entry.target, newValue);
                } else {
                    ((QtBluetoothGattDescriptor)entry.target).setLocalValue(newValue);
                    leServerDescriptorWritten(
                        qtObject, (BluetoothGattDescriptor)entry.target, newValue);
                }
            }
        }
        // Either we executed all writes or were asked to cancel.
        // In any case clear writes and respond.
        clearPendingPreparedWrites(device);
        mGattServer.sendResponse(device, requestId, BluetoothGatt.GATT_SUCCESS, 0, null);
    }

    synchronized void handleOnMtuChanged(BluetoothDevice device, int mtu)
    {
        if (mSupportedMtu == mtu)
            return;
        mSupportedMtu = mtu;
        leMtuChanged(qtObject, mSupportedMtu);
    }

    /*
     * Call back handler for the Gatt Server.
     */
    private BluetoothGattServerCallback mGattServerListener = new BluetoothGattServerCallback()
    {
        @Override
        public void onConnectionStateChange(BluetoothDevice device, int status, int newState) {
            super.onConnectionStateChange(device, status, newState);
            handleOnConnectionStateChange(device, status, newState);
        }

        @Override
        public void onServiceAdded(int status, BluetoothGattService service) {
            super.onServiceAdded(status, service);
            handleOnServiceAdded(status, service);
        }

        @Override
        public void onCharacteristicReadRequest(BluetoothDevice device, int requestId, int offset, BluetoothGattCharacteristic characteristic)
        {
            super.onCharacteristicReadRequest(device, requestId, offset, characteristic);
            handleOnCharacteristicReadRequest(device, requestId, offset, characteristic);
        }

        @Override
        public void onCharacteristicWriteRequest(BluetoothDevice device, int requestId, BluetoothGattCharacteristic characteristic,
                                                 boolean preparedWrite, boolean responseNeeded, int offset, byte[] value)
        {
            super.onCharacteristicWriteRequest(device, requestId, characteristic,
                                               preparedWrite, responseNeeded, offset, value);
            handleOnCharacteristicWriteRequest(device, requestId, characteristic,
                                               preparedWrite, responseNeeded, offset, value);
        }

        @Override
        public void onDescriptorReadRequest(BluetoothDevice device, int requestId, int offset, BluetoothGattDescriptor descriptor)
        {
            super.onDescriptorReadRequest(device, requestId, offset, descriptor);
            handleOnDescriptorReadRequest(device, requestId, offset, descriptor);
        }

        @Override
        public void onDescriptorWriteRequest(BluetoothDevice device, int requestId, BluetoothGattDescriptor descriptor,
                                             boolean preparedWrite, boolean responseNeeded, int offset, byte[] value)
        {
            super.onDescriptorWriteRequest(device, requestId, descriptor, preparedWrite,
                                           responseNeeded, offset, value);
            handleOnDescriptorWriteRequest(device, requestId, descriptor, preparedWrite,
                                           responseNeeded, offset, value);
        }

        @Override
        public void onExecuteWrite(BluetoothDevice device, int requestId, boolean execute)
        {
            super.onExecuteWrite(device, requestId, execute);
            handleOnExecuteWrite(device, requestId, execute);
        }

        @Override
        public void onNotificationSent(BluetoothDevice device, int status) {
            super.onNotificationSent(device, status);
            Log.w(TAG, "onNotificationSent" + device + " " + status);
        }

        @Override
        public void onMtuChanged(BluetoothDevice device, int mtu) {
            handleOnMtuChanged(device, mtu);
        }
    };

    // This function is called from Qt thread
    synchronized int mtu() {
        return mSupportedMtu;
    }

    // This function is called from Qt thread
    synchronized boolean connectServer()
    {
        if (mGattServer != null)
            return true;

        BluetoothManager manager = (BluetoothManager) qtContext.getSystemService(Context.BLUETOOTH_SERVICE);
        if (manager == null) {
            Log.w(TAG, "Bluetooth service not available.");
            return false;
        }

        mGattServer = manager.openGattServer(qtContext, mGattServerListener);

        return (mGattServer != null);
    }

    // This function is called from Qt thread
    synchronized void disconnectServer()
    {
        if (mGattServer == null)
            return;

        clearPendingPreparedWrites(null);
        mPendingServiceAdditions.clear();
        mGattServer.close();
        mGattServer = null;

        mRemoteName = mRemoteAddress = "";
        leConnectionStateChange(qtObject, 0 /*NoError*/,
                                0 /*QLowEnergyController::UnconnectedState*/);
    }

    // This function is called from Qt thread
    boolean startAdvertising(AdvertiseData advertiseData,
                                    AdvertiseData scanResponse,
                                    AdvertiseSettings settings)
    {
        // Check that the bluetooth is on
        if (mBluetoothAdapter == null || !mBluetoothAdapter.isEnabled()) {
            Log.w(TAG, "StartAdvertising: Bluetooth not available or offline");
            return false;
        }

        // According to Android doc this check should always precede the advertiser creation
        if (mLeAdvertiser == null && mBluetoothAdapter.isMultipleAdvertisementSupported())
            mLeAdvertiser = mBluetoothAdapter.getBluetoothLeAdvertiser();

        if (mLeAdvertiser == null) {
            Log.w(TAG, "StartAdvertising: LE advertisement not supported");
            return false;
        }

        if (!connectServer()) {
            Log.w(TAG, "Server::startAdvertising: Cannot open GATT server");
            return false;
        }

        Log.w(TAG, "Starting to advertise.");
        mLeAdvertiser.startAdvertising(settings, advertiseData, scanResponse, mAdvertiseListener);

        return true;
    }

    // This function is called from Qt thread
    void stopAdvertising()
    {
        if (mLeAdvertiser == null)
            return;

        mLeAdvertiser.stopAdvertising(mAdvertiseListener);
        Log.w(TAG, "Advertisement stopped.");
    }

    // This function is called from Qt thread
    synchronized void addService(BluetoothGattService service)
    {
        if (!connectServer()) {
            Log.w(TAG, "Server::addService: Cannot open GATT server");
            return;
        }

        // When we add a service, we must wait for onServiceAdded callback before adding the
        // next one. If the pending service queue is empty it means that there are no ongoing
        // service additions => add the service to the server. If there are services in the
        // queue it means there is an initiated addition ongoing, and we only add to the queue.
        if (mPendingServiceAdditions.isEmpty()) {
            if (mGattServer.addService(service))
                mPendingServiceAdditions.add(service);
            else
                Log.w(TAG, "Adding service " + service.getUuid().toString() + " failed.");
        } else {
            mPendingServiceAdditions.add(service);
        }
    }

    // API-level < 33
    @SuppressWarnings("deprecation")
    private void notifyCharacteristicChange(BluetoothDevice device,
                                            BluetoothGattCharacteristic characteristic,
                                            boolean confirm)
    {
        mGattServer.notifyCharacteristicChanged(device, characteristic, confirm);
    }

    /*
        Check the client characteristics configuration for the given characteristic
        and sends notifications or indications as per required.

        This function is called from Qt and Java threads and calls must be protected
     */
    private void sendNotificationsOrIndications(BluetoothGattCharacteristic characteristic)
    {
        final ListIterator<BluetoothDevice> iter =
                clientCharacteristicManager.getToBeUpdatedDevices(characteristic).listIterator();

        // TODO This quick loop over multiple devices should be synced with onNotificationSent().
        //      The next notifyCharacteristicChanged() call must wait until onNotificationSent()
        //      was received. At this becomes an issue when the server accepts multiple remote
        //      devices at the same time.
        while (iter.hasNext()) {
            final BluetoothDevice device = iter.next();
            final byte[] clientCharacteristicConfig =
                                      clientCharacteristicManager.valueFor(characteristic, device);
            if (clientCharacteristicConfig != null) {
                if (Arrays.equals(clientCharacteristicConfig,
                                  BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE)) {
                    if (Build.VERSION.SDK_INT >= 33) {
                        mGattServer.notifyCharacteristicChanged(device, characteristic, false,
                                  ((QtBluetoothGattCharacteristic)characteristic).getLocalValue());
                    } else {
                        notifyCharacteristicChange(device, characteristic, false);
                    }
                } else if (Arrays.equals(clientCharacteristicConfig,
                                         BluetoothGattDescriptor.ENABLE_INDICATION_VALUE)) {
                    if (Build.VERSION.SDK_INT >= 33) {
                        mGattServer.notifyCharacteristicChanged(device, characteristic, true,
                                  ((QtBluetoothGattCharacteristic)characteristic).getLocalValue());
                    } else {
                        notifyCharacteristicChange(device, characteristic, true);
                    }
                }
            }
        }
    }

    /*
        Updates the local database value for the given characteristic with \a charUuid and
        \a newValue. If notifications for this task are enabled an appropriate notification will
        be send to the remote client.

        This function is called from the Qt thread.
     */
    boolean writeCharacteristic(BluetoothGattService service, UUID charUuid, byte[] newValue)
    {
        BluetoothGattCharacteristic foundChar = null;
        List<BluetoothGattCharacteristic> charList = service.getCharacteristics();
        for (BluetoothGattCharacteristic iter: charList) {
            if (iter.getUuid().equals(charUuid) && foundChar == null) {
                foundChar = iter;
                // don't break here since we want to check next condition below on next iteration
            } else if (iter.getUuid().equals(charUuid)) {
                Log.w(TAG, "Found second char with same UUID. Wrong char may have been selected.");
                break;
            }
        }

        if (foundChar == null) {
            Log.w(TAG, "writeCharacteristic: update for unknown characteristic failed");
            return false;
        }

        // User may have set minimum and/or maximum characteristic value size. Enforce
        // them here. If the user has not defined these limits, the default values 0..INT_MAX
        // do not limit anything.
        final int minValueLength = ((QtBluetoothGattCharacteristic)foundChar).minValueLength;
        final int maxValueLength = ((QtBluetoothGattCharacteristic)foundChar).maxValueLength;
        if (newValue.length < minValueLength || newValue.length > maxValueLength) {
            Log.w(TAG, "writeCharacteristic: invalid value length: "
                  + newValue.length + ", min: " + minValueLength + ", max: " + maxValueLength);
            return false;
        }

        synchronized (this) // a value update might be in progress
        {
            ((QtBluetoothGattCharacteristic)foundChar).setLocalValue(newValue);
            // Value is updated even if server is not connected, but notifying is not possible
            if (mGattServer != null)
                sendNotificationsOrIndications(foundChar);
        }

        return true;
    }

    /*
        Updates the local database value for the given \a descUuid to \a newValue.

        This function is called from the Qt thread.
     */
    boolean writeDescriptor(BluetoothGattService service, UUID charUuid, UUID descUuid,
                                   byte[] newValue)
    {
        BluetoothGattDescriptor foundDesc = null;
        BluetoothGattCharacteristic foundChar = null;
        final List<BluetoothGattCharacteristic> charList = service.getCharacteristics();
        for (BluetoothGattCharacteristic iter: charList) {
            if (!iter.getUuid().equals(charUuid))
                continue;

            if (foundChar == null) {
                foundChar = iter;
            } else {
                Log.w(TAG, "Found second char with same UUID. Wrong char may have been selected.");
                break;
            }
        }

        if (foundChar != null)
            foundDesc = foundChar.getDescriptor(descUuid);

        if (foundChar == null || foundDesc == null) {
            Log.w(TAG, "writeDescriptor: update for unknown char or desc failed (" + foundChar + ")");
            return false;
        }

        // we even write CLIENT_CHARACTERISTIC_CONFIGURATION_UUID this way as we choose
        // to interpret the server's call as a change of the default value.
        synchronized (this) // a value update might be in progress
        {
            ((QtBluetoothGattDescriptor)foundDesc).setLocalValue(newValue);
        }

        return true;
    }

    /*
     * Call back handler for Advertisement requests.
     */
    private AdvertiseCallback mAdvertiseListener = new AdvertiseCallback()
    {
        @Override
        public void onStartSuccess(AdvertiseSettings settingsInEffect) {
            super.onStartSuccess(settingsInEffect);
        }

        @Override
        public void onStartFailure(int errorCode) {
            Log.e(TAG, "Advertising failure: " + errorCode);
            super.onStartFailure(errorCode);

            // changing errorCode here implies changes to errorCode handling on Qt side
            int qtErrorCode = 0;
            switch (errorCode) {
                case AdvertiseCallback.ADVERTISE_FAILED_ALREADY_STARTED:
                    return; // ignore -> noop
                case AdvertiseCallback.ADVERTISE_FAILED_DATA_TOO_LARGE:
                    Log.e(TAG, "Please reduce size of advertising data.");
                    qtErrorCode = 1;
                    break;
                case AdvertiseCallback.ADVERTISE_FAILED_FEATURE_UNSUPPORTED:
                    qtErrorCode = 2;
                    break;
                default: // default maps to internal error
                case AdvertiseCallback.ADVERTISE_FAILED_INTERNAL_ERROR:
                    qtErrorCode = 3;
                    break;
                case AdvertiseCallback.ADVERTISE_FAILED_TOO_MANY_ADVERTISERS:
                    qtErrorCode = 4;
                    break;
            }

            if (qtErrorCode > 0)
                leServerAdvertisementError(qtObject, qtErrorCode);
        }
    };

    native void leConnectionStateChange(long qtObject, int errorCode, int newState);
    native void leMtuChanged(long qtObject, int mtu);
    native void leServerAdvertisementError(long qtObject, int status);
    native void leServerCharacteristicChanged(long qtObject,
                                                     BluetoothGattCharacteristic characteristic,
                                                     byte[] newValue);
    native void leServerDescriptorWritten(long qtObject,
                                                 BluetoothGattDescriptor descriptor,
                                                 byte[] newValue);
}
