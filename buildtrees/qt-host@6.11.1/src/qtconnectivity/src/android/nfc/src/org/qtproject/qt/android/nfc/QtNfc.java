// Copyright (C) 2016 Centria research and development
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt.android.nfc;

import java.lang.Runnable;

import android.content.Context;
import android.app.Activity;
import android.app.PendingIntent;
import android.content.Intent;
import android.content.IntentFilter;
import android.nfc.NfcAdapter;
import android.nfc.Tag;
import android.content.IntentFilter.MalformedMimeTypeException;
import android.os.Build;
import android.os.Parcelable;
import android.util.Log;
import android.content.pm.PackageManager;

class QtNfc
{
    static private final String TAG = "QtNfc";
    static private NfcAdapter m_adapter = null;
    static private PendingIntent m_pendingIntent = null;
    static private Context m_context = null;
    static private Activity m_activity = null;

    static void setContext(Context context)
    {
        m_context = context;
        if (context instanceof Activity) m_activity = (Activity) context;
        m_adapter = NfcAdapter.getDefaultAdapter(context);

        if (m_activity == null) {
            Log.w(TAG, "New NFC tags will only be recognized with Android activities and not with Android services.");
            return;
        }

        if (m_adapter == null) {
            return;
        }

        // Since Android 12 (API level 31) it's mandatory to specify mutability
        // of PendingIntent. We need a mutable intent, which was a default
        // option earlier.
        int flags = (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) ? PendingIntent.FLAG_MUTABLE
                                                                     : 0;
        m_pendingIntent = PendingIntent.getActivity(
            m_activity,
            0,
            new Intent(m_activity, m_activity.getClass()).addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP),
            flags);
    }

    static boolean startDiscovery()
    {
        if (m_adapter == null || m_activity == null
               || !m_activity.getPackageManager().hasSystemFeature(PackageManager.FEATURE_NFC))
            return false;

        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                IntentFilter[] filters = new IntentFilter[3];
                filters[0] = new IntentFilter();
                filters[0].addAction(NfcAdapter.ACTION_TAG_DISCOVERED);
                filters[0].addCategory(Intent.CATEGORY_DEFAULT);
                filters[1] = new IntentFilter();
                filters[1].addAction(NfcAdapter.ACTION_NDEF_DISCOVERED);
                filters[1].addCategory(Intent.CATEGORY_DEFAULT);
                try {
                    filters[1].addDataType("*/*");
                } catch (MalformedMimeTypeException e) {
                    throw new RuntimeException("IntentFilter.addDataType() failed");
                }
                // some tags will report as tech, even if they are ndef formatted/formattable.
                filters[2] = new IntentFilter();
                filters[2].addAction(NfcAdapter.ACTION_TECH_DISCOVERED);
                String[][] techList = new String[][]{
                        {"android.nfc.tech.Ndef"},
                        {"android.nfc.tech.NdefFormatable"}
                    };
                try {
                    m_adapter.enableForegroundDispatch(m_activity, m_pendingIntent, filters, techList);
                } catch(IllegalStateException e) {
                    // On Android we must call enableForegroundDispatch when the activity is in foreground, only.
                    Log.d(TAG, "enableForegroundDispatch failed: " + e.toString());
                }
            }
        });
        return true;
    }

    static boolean stopDiscovery()
    {
        if (m_adapter == null || m_activity == null
               || !m_activity.getPackageManager().hasSystemFeature(PackageManager.FEATURE_NFC))
            return false;

        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                try {
                    m_adapter.disableForegroundDispatch(m_activity);
                } catch(IllegalStateException e) {
                    // On Android we must call disableForegroundDispatch when the activity is in foreground, only.
                    Log.d(TAG, "disableForegroundDispatch failed: " + e.toString());
                }
            }
        });
        return true;
    }

    static boolean isEnabled()
    {
        if (m_adapter == null) {
            return false;
        }

        return m_adapter.isEnabled();
    }

    static boolean isSupported()
    {
        return (m_adapter != null);
    }

    static Intent getStartIntent()
    {
        Log.d(TAG, "getStartIntent");
        if (m_activity == null) return null;

        Intent intent = m_activity.getIntent();
        if (NfcAdapter.ACTION_NDEF_DISCOVERED.equals(intent.getAction()) ||
                NfcAdapter.ACTION_TECH_DISCOVERED.equals(intent.getAction()) ||
                NfcAdapter.ACTION_TAG_DISCOVERED.equals(intent.getAction())) {
            return intent;
        } else {
            return null;
        }
    }

    @SuppressWarnings("deprecation")
    static Parcelable getTag(Intent intent)
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU)
            return intent.getParcelableExtra(NfcAdapter.EXTRA_TAG, Tag.class);

        return intent.getParcelableExtra(NfcAdapter.EXTRA_TAG);
    }
}
