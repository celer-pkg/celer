// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt.android.positioning;

import android.content.Context;
import android.location.altitude.AltitudeConverter;
import android.location.GpsSatellite;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.GnssStatus;
import android.location.GnssStatus.Callback;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Build;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

import android.util.Log;

class QtPositioning implements LocationListener
{

    private static final String TAG = "qt.positioning.android";
    static LocationManager locationManager = null;
    static Object m_syncObject = new Object();
    static HashMap<Integer, QtPositioning> runningListeners = new HashMap<Integer, QtPositioning>();

    /*
        The positionInfo instance to which this
        QtPositioning instance is attached to.
    */
    private int nativeClassReference = 0;

    /*
        The provider type requested by Qt
    */
    private int expectedProviders = 0;

    static final int QT_GPS_PROVIDER = 1;
    static final int QT_NETWORK_PROVIDER = 2;

    /* The following values must match the corresponding error enums in the Qt API*/
    static final int QT_ACCESS_ERROR = 0;
    static final int QT_CLOSED_ERROR = 1;
    static final int QT_POSITION_UNKNOWN_SOURCE_ERROR = 2;
    static final int QT_POSITION_NO_ERROR = 3;
    static final int QT_SATELLITE_NO_ERROR = 2;
    static final int QT_SATELLITE_UNKNOWN_SOURCE_ERROR = -1;

    /* True, if updates were caused by requestUpdate() */
    private boolean isSingleUpdate = false;
    /* The length requested for regular intervals in msec. */
    private int updateIntervalTime = 0;

    /* The last received GPS update */
    private Location lastGps = null;
    /* The last received network update */
    private Location lastNetwork = null;
    /* If true this class acts as satellite signal monitor rather than location monitor */
    private boolean isSatelliteUpdate = false;
    /* Try to convert the altitude to MSL or not */
    private boolean useAltitudeConverter = false;

    private PositioningLooperBase looperThread;

    private boolean isLocationProvidersDisabledInvoked = false;

    private static Context appContext = null;
    private static AltitudeConverter altitudeConverter = null;

    static void setContext(Context context)
    {
        try {
            appContext = context;
            locationManager = (LocationManager)context.getSystemService(Context.LOCATION_SERVICE);
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    static private int[] providerList()
    {
        if (locationManager == null) {
            Log.w(TAG, "No locationManager available in QtPositioning");
            return new int[0];
        }
        List<String> providers = locationManager.getProviders(true);
        int retList[] = new int[providers.size()];
        for (int i = 0; i < providers.size();  i++) {
            if (providers.get(i).equals(LocationManager.GPS_PROVIDER)) {
                //must be in sync with AndroidPositioning::PositionProvider::PROVIDER_GPS
                retList[i] = 0;
            } else if (providers.get(i).equals(LocationManager.NETWORK_PROVIDER)) {
                //must be in sync with AndroidPositioning::PositionProvider::PROVIDER_NETWORK
                retList[i] = 1;
            } else if (providers.get(i).equals(LocationManager.PASSIVE_PROVIDER)) {
                //must be in sync with AndroidPositioning::PositionProvider::PROVIDER_PASSIVE
                retList[i] = 2;
            } else {
                retList[i] = -1;
            }
        }
        return retList;
    }

    static private void addMslAltitude(Location location)
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE) {
            if (location.hasMslAltitude()) // Nothing to be done
                return;
            if (altitudeConverter == null)
                altitudeConverter = new AltitudeConverter();
            try {
                altitudeConverter.addMslAltitudeToLocation(appContext, location);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    static Location lastKnownPosition(boolean fromSatelliteOnly,
                                             boolean useAltitudeConverter)
    {
        Location gps = null;
        Location network = null;
        try {
            gps = locationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
            if (useAltitudeConverter)
                addMslAltitude(gps);
        } catch (Exception e) {
            // This can throw if we only use ACCESS_COARSE_LOCATION. However,
            // if we didn't set fromSatelliteOnly to true, that is not an error.
            if (fromSatelliteOnly)
                e.printStackTrace();
            gps = null;
        }
        if (!fromSatelliteOnly) {
            try {
                network = locationManager.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
                if (useAltitudeConverter)
                    addMslAltitude(network);
            } catch(Exception e) {
                e.printStackTrace();
                network = null;
            }
        }

        if (gps != null && network != null) {
            //we return the most recent location but slightly prefer GPS
            //prefer GPS if it is max 4 hrs older than network
            long delta = network.getTime() - gps.getTime();
            if (delta < 4*60*60*1000) {
                return gps;
            } else {
                return network;
            }
        } else if (gps != null ) {
            return gps;
        } else if (network != null) {
            return network;
        }

        return null;
    }

    /* Returns true if at least on of the given providers is enabled. */
    static private boolean expectedProvidersAvailable(int desiredProviders)
    {
        List<String> enabledProviders = locationManager.getProviders(true);
        if ((desiredProviders & QT_GPS_PROVIDER) > 0) { //gps desired
            if (enabledProviders.contains(LocationManager.GPS_PROVIDER)) {
                return true;
            }
        }
        if ((desiredProviders & QT_NETWORK_PROVIDER) > 0) { //network desired
            if (enabledProviders.contains(LocationManager.NETWORK_PROVIDER)) {
                return true;
            }
        }

        return false;
    }


    static private void addActiveListener(QtPositioning listener, String provider, long minTime, float minDistance)
    {
        int androidClassKey = listener.nativeClassReference;
        //start update thread
        listener.setActiveLooper(true);

        if (runningListeners.containsKey(androidClassKey) && runningListeners.get(androidClassKey) != listener) {
            removeActiveListener(androidClassKey);
        }

        locationManager.requestLocationUpdates(provider,
                                               minTime, minDistance,
                                               listener,
                                               listener.looper());

        runningListeners.put(androidClassKey, listener);
    }


    static private void removeActiveListener(QtPositioning listener)
    {
        removeActiveListener(listener.nativeClassReference);
    }


    static private void removeActiveListener(int androidClassKey)
    {
        QtPositioning listener = runningListeners.remove(androidClassKey);

        if (listener != null) {
            locationManager.removeUpdates(listener);
            listener.setActiveLooper(false);
        }
    }


    static int startUpdates(int androidClassKey, int locationProvider, int updateInterval,
                                   boolean useAltitudeConverter)
    {
        synchronized (m_syncObject) {
            try {
                boolean providerStarted = false;
                SecurityException lastException = null;
                QtPositioning positioningListener = new QtPositioning();
                positioningListener.nativeClassReference = androidClassKey;
                positioningListener.expectedProviders = locationProvider;
                positioningListener.isSatelliteUpdate = false;
                positioningListener.useAltitudeConverter = useAltitudeConverter;

                if (updateInterval == 0)
                    updateInterval = 50; //don't update more often than once per 50ms

                positioningListener.updateIntervalTime = updateInterval;
                if ((locationProvider & QT_GPS_PROVIDER) > 0) {
                    Log.d(TAG, "Regular updates using GPS " + updateInterval);
                    try {
                        addActiveListener(positioningListener,
                                          LocationManager.GPS_PROVIDER,
                                          updateInterval, 0);
                        providerStarted = true;
                    } catch (SecurityException se) {
                        lastException = se;
                    }
                }

                if ((locationProvider & QT_NETWORK_PROVIDER) > 0) {
                    Log.d(TAG, "Regular updates using network " + updateInterval);
                    try {
                        addActiveListener(positioningListener,
                                          LocationManager.NETWORK_PROVIDER,
                                          updateInterval, 0);
                        providerStarted = true;
                    } catch (SecurityException se) {
                        lastException = se;
                    }
                }
                // The preferred positioning methods and the granted permissions
                // might not match, so raise an error only if none of the
                // requested providers could be started.
                if (!providerStarted && lastException != null) {
                    lastException.printStackTrace();
                    removeActiveListener(positioningListener);
                    return QT_ACCESS_ERROR;
                }

                if (!expectedProvidersAvailable(locationProvider)) {
                    //all location providers unavailbe -> when they come back we resume automatically
                    return QT_CLOSED_ERROR;
                }

            } catch(Exception e) {
                e.printStackTrace();
                return QT_POSITION_UNKNOWN_SOURCE_ERROR;
            }

            return QT_POSITION_NO_ERROR;
        }
    }

    static void stopUpdates(int androidClassKey)
    {
        synchronized (m_syncObject) {
            try {
                Log.d(TAG, "Stopping updates");
                removeActiveListener(androidClassKey);
            } catch(Exception e) {
                e.printStackTrace();
                return;
            }
        }
    }

    static int requestUpdate(int androidClassKey, int locationProvider, int timeout,
                                    boolean useAltitudeConverter)
    {
        synchronized (m_syncObject) {
            try {
                boolean providerStarted = false;
                SecurityException lastException = null;
                QtPositioning positioningListener = new QtPositioning();
                positioningListener.nativeClassReference = androidClassKey;
                positioningListener.isSingleUpdate = true;
                positioningListener.expectedProviders = locationProvider;
                positioningListener.isSatelliteUpdate = false;
                positioningListener.useAltitudeConverter = useAltitudeConverter;

                if ((locationProvider & QT_GPS_PROVIDER) > 0) {
                    Log.d(TAG, "Single update using GPS");
                    try {
                        addActiveListener(positioningListener, LocationManager.GPS_PROVIDER,
                                          timeout, 0);
                        providerStarted = true;
                    } catch (SecurityException se) {
                        lastException = se;
                    }
                }

                if ((locationProvider & QT_NETWORK_PROVIDER) > 0) {
                    Log.d(TAG, "Single update using network");
                    try {
                        addActiveListener(positioningListener, LocationManager.NETWORK_PROVIDER,
                                          timeout, 0);
                        providerStarted = true;
                    } catch (SecurityException se) {
                        lastException = se;
                    }
                }
                // The preferred positioning methods and the granted permissions
                // might not match, so raise an error only if none of the
                // requested providers could be started.
                if (!providerStarted && lastException != null) {
                    lastException.printStackTrace();
                    removeActiveListener(positioningListener);
                    return QT_ACCESS_ERROR;
                }

                if (!expectedProvidersAvailable(locationProvider)) {
                    //all location providers unavailable -> when they come back we resume automatically
                    //in the mean time return ClosedError
                    return QT_CLOSED_ERROR;
                }

            } catch(Exception e) {
                e.printStackTrace();
                return QT_POSITION_UNKNOWN_SOURCE_ERROR;
            }

            return QT_POSITION_NO_ERROR;
        }
    }

    static int startSatelliteUpdates(int androidClassKey, int updateInterval, boolean isSingleRequest)
    {
        synchronized (m_syncObject) {
            try {
                boolean exceptionOccurred = false;
                QtPositioning positioningListener = new QtPositioning();
                positioningListener.isSatelliteUpdate = true;
                positioningListener.nativeClassReference = androidClassKey;
                positioningListener.expectedProviders = 1; //always satellite provider
                positioningListener.isSingleUpdate = isSingleRequest;

                if (updateInterval == 0)
                    updateInterval = 50; //don't update more often than once per 50ms

                if (isSingleRequest)
                    Log.d(TAG, "Single update for Satellites " + updateInterval);
                else
                    Log.d(TAG, "Regular updates for Satellites " + updateInterval);
                try {
                    addActiveListener(positioningListener, LocationManager.GPS_PROVIDER,
                                      updateInterval, 0);
                } catch (SecurityException se) {
                    se.printStackTrace();
                    exceptionOccurred = true;
                }

                if (exceptionOccurred) {
                    removeActiveListener(positioningListener);
                    return QT_ACCESS_ERROR;
                }

                if (!expectedProvidersAvailable(positioningListener.expectedProviders)) {
                    //all location providers unavailable -> when they come back we resume automatically
                    //in the mean time return ClosedError
                    return QT_CLOSED_ERROR;
                }

            } catch(Exception e) {
                e.printStackTrace();
                return QT_SATELLITE_UNKNOWN_SOURCE_ERROR;
            }

            return QT_SATELLITE_NO_ERROR;
        }
    }

    QtPositioning()
    {
        // Use GpsStatus for API Level <= 23 (version M and below) and
        // GnssStatus for other API levels.
        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.M)
            looperThread = new PositioningLooperGps();
        else
            looperThread = new PositioningLooperGnss();
    }

    Looper looper()
    {
        return looperThread.looper();
    }

    private void setActiveLooper(boolean setActive)
    {
        try{
            if (setActive) {
                if (looperThread.isAlive())
                    return;

                if (isSatelliteUpdate)
                    looperThread.isSatelliteListener(true);

                long start = System.currentTimeMillis();
                looperThread.start();

                //busy wait but lasts ~20-30 ms only
                while (!looperThread.isReady());

                long stop = System.currentTimeMillis();
                Log.d(TAG, "Looper Thread startup time in ms: " + (stop-start));
            } else {
                looperThread.quitLooper();
            }
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    private abstract class PositioningLooperBase extends Thread
    {
        private boolean looperRunning;
        private Looper posLooper;
        private boolean isSatelliteLooper = false;

        abstract protected void addSatelliteInfoListener();
        abstract protected void removeSatelliteInfoListener();

        private PositioningLooperBase()
        {
            looperRunning = false;
        }

        @Override
        public void run()
        {
            Looper.prepare();

            if (isSatelliteLooper)
                addSatelliteInfoListener();

            posLooper = Looper.myLooper();
            synchronized (this) {
                looperRunning = true;
            }
            Looper.loop();
            synchronized (this) {
                looperRunning = false;
            }
        }

        void quitLooper()
        {
            if (isSatelliteLooper)
                removeSatelliteInfoListener();
            looper().quit();
        }

        synchronized boolean isReady()
        {
            return looperRunning;
        }

        void isSatelliteListener(boolean isListener)
        {
            isSatelliteLooper = isListener;
        }

        Looper looper()
        {
            return posLooper;
        }

    }

    private class PositioningLooperGps extends PositioningLooperBase implements GpsStatus.Listener
    {
        @Override
        protected void addSatelliteInfoListener()
        {
            try {
                locationManager.addGpsStatusListener(this);
            } catch(Exception e) {
                e.printStackTrace();
            }
        }

        @Override
        protected void removeSatelliteInfoListener()
        {
            locationManager.removeGpsStatusListener(this);
        }

        @Override
        public void onGpsStatusChanged(int event) {
            switch (event) {
                case GpsStatus.GPS_EVENT_FIRST_FIX:
                    break;
                case GpsStatus.GPS_EVENT_SATELLITE_STATUS:
                    GpsStatus status = locationManager.getGpsStatus(null);
                    Iterable<GpsSatellite> iterable = status.getSatellites();
                    Iterator<GpsSatellite> it = iterable.iterator();

                    ArrayList<GpsSatellite> list = new ArrayList<GpsSatellite>();
                    while (it.hasNext()) {
                        GpsSatellite sat = it.next();
                        list.add(sat);
                    }
                    GpsSatellite[] sats = list.toArray(new GpsSatellite[list.size()]);
                    satelliteGpsUpdated(sats, nativeClassReference, isSingleUpdate);

                    break;
                case GpsStatus.GPS_EVENT_STARTED:
                    break;
                case GpsStatus.GPS_EVENT_STOPPED:
                    break;
            }
        }
    }

    private class PositioningGnssListener extends GnssStatus.Callback
    {
        @Override
        public void onSatelliteStatusChanged(GnssStatus status)
        {
            satelliteGnssUpdated(status, nativeClassReference, isSingleUpdate);
        }
    }

    private class PositioningLooperGnss extends PositioningLooperBase
    {
        private PositioningGnssListener gnssListener;

        private PositioningLooperGnss()
        {
            gnssListener = new PositioningGnssListener();
        }

        @Override
        protected void addSatelliteInfoListener()
        {
            try {
                locationManager.registerGnssStatusCallback(gnssListener, null);
            } catch(Exception e) {
                e.printStackTrace();
            }
        }

        @Override
        protected void removeSatelliteInfoListener()
        {
            locationManager.unregisterGnssStatusCallback(gnssListener);
        }
    }

    static native void positionUpdated(Location update, int androidClassKey, boolean isSingleUpdate);
    static native void locationProvidersDisabled(int androidClassKey);
    static native void locationProvidersChanged(int androidClassKey);
    static native void satelliteGpsUpdated(Object[] update, int androidClassKey, boolean isSingleUpdate);
    static native void satelliteGnssUpdated(GnssStatus update, int androidClassKey, boolean isSingleUpdate);

    @Override
    public void onLocationChanged(Location location) {
        //Log.d(TAG, "**** Position Update ****: " +  location.toString() + " " + isSingleUpdate);
        if (location == null)
            return;

        if (useAltitudeConverter)
            addMslAltitude(location);

        if (isSatelliteUpdate) //we are a QGeoSatelliteInfoSource -> ignore
            return;

        if (isSingleUpdate || expectedProviders < 3) {
            positionUpdated(location, nativeClassReference, isSingleUpdate);
            return;
        }

        /*
            We can use GPS and Network, pick the better location provider.
            Generally we prefer GPS data due to their higher accurancy but we
            let Network data pass until GPS fix is available
        */

        if (location.getProvider().equals(LocationManager.GPS_PROVIDER)) {
            lastGps = location;

            // assumption: GPS always better -> pass it on
            positionUpdated(location, nativeClassReference, isSingleUpdate);
        } else if (location.getProvider().equals(LocationManager.NETWORK_PROVIDER)) {
            lastNetwork = location;

            if (lastGps == null) { //no GPS fix yet use network location
                positionUpdated(location, nativeClassReference, isSingleUpdate);
                return;
            }

            long delta = location.getTime() - lastGps.getTime();

            // Ignore if network update is older than last GPS (delta < 0)
            // Ignore if gps update still has time to provide next location (delta < updateInterval)
            if (delta < updateIntervalTime)
                return;

            // Use network data -> GPS has timed out on updateInterval
            positionUpdated(location, nativeClassReference, isSingleUpdate);
        }
    }

    // This method is deprecated starting from API Level 29, but for older
    // versions it should still be implemented!
    @SuppressWarnings("deprecation")
    @Override
    public void onStatusChanged(String provider, int status, Bundle extras) {}

    @Override
    public void onProviderEnabled(String provider) {
        Log.d(TAG, "Enabled provider: " + provider);
        locationProvidersChanged(nativeClassReference);
        if (isLocationProvidersDisabledInvoked && expectedProvidersAvailable(expectedProviders))
            isLocationProvidersDisabledInvoked = false;
    }

    @Override
    public void onProviderDisabled(String provider) {
        Log.d(TAG, "Disabled provider: " + provider);
        locationProvidersChanged(nativeClassReference);
        if (!isLocationProvidersDisabledInvoked && !expectedProvidersAvailable(expectedProviders)) {
            isLocationProvidersDisabledInvoked = true;
            locationProvidersDisabled(nativeClassReference);
        }
    }
}
