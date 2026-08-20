// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt.android.bluetooth;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.util.Log;
import java.io.IOException;
import java.util.UUID;

@SuppressWarnings("WeakerAccess")
class QtBluetoothSocketServer extends Thread
{

    /* Pointer to the Qt object that "owns" the Java object */
    @SuppressWarnings({"WeakerAccess", "CanBeFinal"})
    long qtObject = 0;
    @SuppressWarnings({"WeakerAccess", "CanBeFinal"})
    boolean logEnabled = false;
    @SuppressWarnings("WeakerAccess")
    static Context qtContext = null;

    private static final String TAG = "QtBluetooth";
    private boolean m_isSecure = false;
    private UUID m_uuid;
    private String m_serviceName;
    private BluetoothServerSocket m_serverSocket = null;

    //error codes
    private static final int QT_NO_BLUETOOTH_SUPPORTED = 0;
    private static final int QT_LISTEN_FAILED = 1;
    private static final int QT_ACCEPT_FAILED = 2;

    QtBluetoothSocketServer(Context context)
    {
        qtContext = context;
        setName("QtSocketServerThread");
    }

    void setServiceDetails(String uuid, String serviceName, boolean isSecure)
    {
        m_uuid = UUID.fromString(uuid);
        m_serviceName = serviceName;
        m_isSecure = isSecure;

    }

    @Override
    public void run()
    {
        BluetoothManager manager =
            (BluetoothManager)qtContext.getSystemService(Context.BLUETOOTH_SERVICE);

        if (manager == null) {
            errorOccurred(qtObject, QT_NO_BLUETOOTH_SUPPORTED);
            return;
        }

        BluetoothAdapter adapter = manager.getAdapter();
        if (adapter == null) {
            errorOccurred(qtObject, QT_NO_BLUETOOTH_SUPPORTED);
            return;
        }

        try {
            if (m_isSecure) {
                m_serverSocket = adapter.listenUsingRfcommWithServiceRecord(m_serviceName, m_uuid);
                if (logEnabled)
                    Log.d(TAG, "Using secure socket listener");
            } else {
                m_serverSocket = adapter.listenUsingInsecureRfcommWithServiceRecord(m_serviceName, m_uuid);
                if (logEnabled)
                    Log.d(TAG, "Using insecure socket listener");
            }
        } catch (IOException ex) {
            if (logEnabled)
                Log.d(TAG, "Server socket listen() failed:" + ex.toString());
            ex.printStackTrace();
            errorOccurred(qtObject, QT_LISTEN_FAILED);
            return;
        }

        if (isInterrupted()) // close() may have been called
            return;

        BluetoothSocket s;
        if (m_serverSocket != null) {
            try {
                while (!isInterrupted()) {
                    //this blocks until we see incoming connection
                    //or close() is called
                    if (logEnabled)
                        Log.d(TAG, "Waiting for new incoming socket");
                    s = m_serverSocket.accept();

                    if (logEnabled)
                        Log.d(TAG, "New socket accepted");
                    newSocket(qtObject, s);
                }
            } catch (IOException ex) {
                if (logEnabled)
                    Log.d(TAG, "Server socket accept() failed:" + ex.toString());
                ex.printStackTrace();
                errorOccurred(qtObject, QT_ACCEPT_FAILED);
            }
        }

        Log.d(TAG, "Leaving server socket thread.");
    }

    // This function closes the socket server
    //
    // A note on threading behavior
    // 1. This function is called from Qt thread which is different from the Java thread (run())
    // 2. The caller of this function expects the Java thread to be finished upon return
    //
    // First we mark the Java thread as interrupted, then call close() on the
    // listening socket if it had been created, and lastly wait for the thread to finish.
    // The close() method of the socket is intended to be used to abort the accept() from
    // another thread, as per the accept() documentation.
    //
    // If the Java thread was in the middle of creating a socket with the non-blocking
    // listen* call, the run() will notice after the returning from the listen* that it has
    // been interrupted and returns early from the run().
    //
    // If the Java thread was in the middle of the blocking accept() call, it will get
    // interrupated by the close() call on the socket. After returning the run() will
    // notice it has been interrupted and return from the run()
    void close()
    {
        if (!isAlive())
            return;

        try {
            //ensure closing of thread if we are not currently blocking on accept()
            interrupt();

            //interrupts accept() call above
            if (m_serverSocket != null)
                m_serverSocket.close();
            // Wait for the thread to finish
            join(20); // Maximum wait in ms, typically takes < 1ms
        } catch (Exception ex) {
            Log.d(TAG, "Closing server socket close() failed:" + ex.toString());
            ex.printStackTrace();
        }
    }

    static native void errorOccurred(long qtObject, int errorCode);
    static native void newSocket(long qtObject, BluetoothSocket socket);
}
