// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt.android.bluetooth;

import android.bluetooth.BluetoothGattDescriptor;
import android.os.Build;

import java.util.UUID;

class QtBluetoothGattDescriptor extends BluetoothGattDescriptor {
    QtBluetoothGattDescriptor(UUID uuid, int permissions) {
        super(uuid, permissions);
    }
    // Starting from API 33 Android Bluetooth deprecates descriptor local value caching by
    // deprecating the getValue() and setValue() accessors. For peripheral role we store the value
    // locally in the descriptor as a convenience - looking up the value on the C++ side would
    // be somewhat complicated. This should be safe as all accesses to this class are synchronized.
    // For clarity: For API levels below 33 we still need to use the setValue() of the base class
    // because Android internally uses getValue() with APIs below 33.
    @SuppressWarnings("deprecation")
    boolean setLocalValue(byte[] value) {
        if (Build.VERSION.SDK_INT >= 33) {
            m_localValue = value;
            return true;
        } else {
            return setValue(value);
        }
    }

    @SuppressWarnings("deprecation")
    byte[] getLocalValue()
    {
        if (Build.VERSION.SDK_INT >= 33)
            return m_localValue;
        else
            return getValue();
    }

    private byte[] m_localValue = null;
}
