// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt.android.view;

import android.content.pm.PackageManager;
import android.view.View;
import android.webkit.GeolocationPermissions;
import android.webkit.URLUtil;
import android.webkit.ValueCallback;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.webkit.WebChromeClient;
import android.webkit.CookieManager;
import java.lang.Runnable;
import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import java.lang.String;
import android.webkit.WebSettings;
import android.util.Log;
import android.webkit.WebSettings.PluginState;
import android.graphics.Bitmap;
import java.util.concurrent.Semaphore;
import java.lang.reflect.Method;
import android.os.Build;
import java.util.concurrent.TimeUnit;
import java.time.format.DateTimeFormatter;

class QtAndroidWebViewController
{
    private final Activity m_activity;
    private final long m_id;
    private boolean m_hasLocationPermission;
    private WebView m_webView = null;
    private static final String TAG = "QtAndroidWebViewController";
    private final int INIT_STATE = 0;
    private final int STARTED_STATE = 1;
    private final int LOADING_STATE = 2;
    private final int FINISHED_STATE = 3;

    private volatile int m_loadingState = INIT_STATE;
    private volatile int m_progress = 0;
    private volatile int m_frameCount = 0;

    // API 11 methods
    private Method m_webViewOnResume = null;
    private Method m_webViewOnPause = null;
    private Method m_webSettingsSetDisplayZoomControls = null;

    // API 19 methods
    private Method m_webViewEvaluateJavascript = null;

    // Native callbacks
    private native void c_onPageFinished(long id, String url);
    private native void c_onPageStarted(long id, String url, Bitmap icon);
    private native void c_onProgressChanged(long id, int newProgress);
    private native void c_onReceivedIcon(long id, Bitmap icon);
    private native void c_onReceivedTitle(long id, String title);
    private native void c_onRunJavaScriptResult(long id, long callbackId, String result);
    private native void c_onReceivedError(long id, int errorCode, String description, String url);
    private static native void c_onCookieAdded(long id, boolean result, String domain, String name);
    private static native void c_onCookieRemoved(long id, boolean result, String domain, String name);

    // We need to block the UI thread in some cases, if it takes to long we should timeout before
    // ANR kicks in... Usually the hard limit is set to 10s and if exceed that then we're in trouble.
    // In general we should not let input events be delayed for more then 500ms (If we're spending more
    // then 200ms somethings off...).
    private final long BLOCKING_TIMEOUT = 250;

    private void resetLoadingState(final int state)
    {
        m_progress = 0;
        m_frameCount = 0;
        m_loadingState = state;
    }

    private class QtAndroidWebViewClient extends WebViewClient
    {
        QtAndroidWebViewClient() { super(); }

        @Override
        public boolean shouldOverrideUrlLoading(WebView view, String url)
        {
            // handle http: and http:, etc., as usual
            if (URLUtil.isValidUrl(url))
                return false;

            // try to handle geo:, tel:, mailto: and other schemes
            try {
                Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
                view.getContext().startActivity(intent);
                return true;
            } catch (Exception e) {
                e.printStackTrace();
            }

            return false;
        }

        @Override
        public void onLoadResource(WebView view, String url)
        {
            super.onLoadResource(view, url);
        }

        @Override
        public void onPageFinished(WebView view, String url)
        {
            super.onPageFinished(view, url);
            m_frameCount = 0;
            if (m_loadingState == INIT_STATE) {
                // we got an error do not call pageFinished
                m_loadingState = FINISHED_STATE;
            } else {
                m_loadingState = FINISHED_STATE;
                c_onPageFinished(m_id, url);
            }
        }

        @Override
        public void onPageStarted(WebView view, String url, Bitmap favicon)
        {
            super.onPageStarted(view, url, favicon);
            if (++m_frameCount == 1) { // Only call onPageStarted for the first frame.
                m_loadingState = LOADING_STATE;
                c_onPageStarted(m_id, url, favicon);
            }
        }

        @Override
        public void onReceivedError(WebView view,
                                    int errorCode,
                                    String description,
                                    String url)
        {
            super.onReceivedError(view, errorCode, description, url);
            resetLoadingState(INIT_STATE);
            c_onReceivedError(m_id, errorCode, description, url);
        }
    }

    private class QtAndroidWebChromeClient extends WebChromeClient
    {
        QtAndroidWebChromeClient() { super(); }
        @Override
        public void onProgressChanged(WebView view, int newProgress)
        {
            super.onProgressChanged(view, newProgress);
            m_progress = newProgress;
            c_onProgressChanged(m_id, newProgress);
        }

        @Override
        public void onReceivedIcon(WebView view, Bitmap icon)
        {
            super.onReceivedIcon(view, icon);
            c_onReceivedIcon(m_id, icon);
        }

        @Override
        public void onReceivedTitle(WebView view, String title)
        {
            super.onReceivedTitle(view, title);
            c_onReceivedTitle(m_id, title);
        }

        @Override
        public void onGeolocationPermissionsShowPrompt(String origin, GeolocationPermissions.Callback callback)
        {
            callback.invoke(origin, m_hasLocationPermission, false);
        }
    }

    QtAndroidWebViewController(final Activity activity, final long id)
    {
        m_activity = activity;
        m_id = id;
        final Semaphore sem = new Semaphore(0);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                m_webView = new WebView(m_activity);
                m_hasLocationPermission = hasLocationPermission(m_webView);
                WebSettings webSettings = m_webView.getSettings();

                // The local storage options are not user changeable in QtWebView and disabled by default on Android.
                // In QtWebEngine and on iOS local storage is enabled by default, so we follow that.
                webSettings.setDatabaseEnabled(true);
                webSettings.setDomStorageEnabled(true);

                if (Build.VERSION.SDK_INT > 10) {
                    try {
                        m_webViewOnResume = m_webView.getClass().getMethod("onResume");
                        m_webViewOnPause = m_webView.getClass().getMethod("onPause");
                        m_webSettingsSetDisplayZoomControls = webSettings.getClass().getMethod("setDisplayZoomControls", boolean.class);
                        if (Build.VERSION.SDK_INT > 18) {
                            m_webViewEvaluateJavascript = m_webView.getClass().getMethod("evaluateJavascript",
                                                                                         String.class,
                                                                                         ValueCallback.class);
                        }
                    } catch (Exception e) { /* Do nothing */ e.printStackTrace(); }
                }

                //allowing access to location without actual ACCESS_FINE_LOCATION may throw security exception
                webSettings.setGeolocationEnabled(m_hasLocationPermission);

                webSettings.setJavaScriptEnabled(true);
                if (m_webSettingsSetDisplayZoomControls != null) {
                    try { m_webSettingsSetDisplayZoomControls.invoke(webSettings, false); } catch (Exception e) { e.printStackTrace(); }
                }
                webSettings.setBuiltInZoomControls(true);
                webSettings.setPluginState(PluginState.ON);
                m_webView.setWebViewClient((WebViewClient)new QtAndroidWebViewClient());
                m_webView.setWebChromeClient((WebChromeClient)new QtAndroidWebChromeClient());
                sem.release();
            }
        });

        try {
            sem.acquire();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // Settings
    void setLocalStorageEnabled(boolean enabled)
    {
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                WebSettings webSettings = m_webView.getSettings();
                webSettings.setDatabaseEnabled(enabled);
                webSettings.setDomStorageEnabled(enabled);
            }
        });
    }

    boolean isLocalStorageEnabled()
    {
        final boolean[] enabled = {true};
        final Semaphore sem = new Semaphore(0);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                WebSettings webSettings = m_webView.getSettings();
                enabled[0] = webSettings.getDatabaseEnabled() && webSettings.getDomStorageEnabled();
                sem.release();
            }
        });

        try {
            sem.tryAcquire(BLOCKING_TIMEOUT, TimeUnit.MILLISECONDS);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return enabled[0];
    }

    void setJavaScriptEnabled(boolean enabled)
    {
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                WebSettings webSettings = m_webView.getSettings();
                webSettings.setJavaScriptEnabled(enabled);
            }
        });
    }

    boolean isJavaScriptEnabled()
    {
        final boolean[] enabled = {true};
        final Semaphore sem = new Semaphore(0);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                WebSettings webSettings = m_webView.getSettings();
                enabled[0] = webSettings.getJavaScriptEnabled();
                sem.release();
            }
        });

        try {
            sem.tryAcquire(BLOCKING_TIMEOUT, TimeUnit.MILLISECONDS);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return enabled[0];
    }

    void setAllowFileAccessFromFileURLs(boolean enabled)
    {
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                WebSettings webSettings = m_webView.getSettings();
                webSettings.setAllowFileAccessFromFileURLs(enabled);
            }
        });
    }

    boolean isAllowFileAccessFromFileURLsEnabled()
    {
        final boolean[] enabled = {true};
        final Semaphore sem = new Semaphore(0);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                WebSettings webSettings = m_webView.getSettings();
                enabled[0] = webSettings.getAllowFileAccessFromFileURLs();
                sem.release();
            }
        });

        try {
            sem.tryAcquire(BLOCKING_TIMEOUT, TimeUnit.MILLISECONDS);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return enabled[0];
    }

    void setAllowFileAccess(boolean enabled)
    {
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                WebSettings webSettings = m_webView.getSettings();
                webSettings.setAllowFileAccess(enabled);
            }
        });
    }

    boolean isAllowFileAccessEnabled()
    {
        final boolean[] enabled = {true};
        final Semaphore sem = new Semaphore(0);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                WebSettings webSettings = m_webView.getSettings();
                enabled[0] = webSettings.getAllowFileAccess();
                sem.release();
            }
        });

        try {
            sem.tryAcquire(BLOCKING_TIMEOUT, TimeUnit.MILLISECONDS);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return enabled[0];
    }

    String getUserAgent()
    {
        final String[] ua = {""};
        final Semaphore sem = new Semaphore(0);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ua[0] = m_webView.getSettings().getUserAgentString();
                sem.release();
            }
        });

        try {
            sem.tryAcquire(BLOCKING_TIMEOUT, TimeUnit.MILLISECONDS);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return ua[0];
    }

    void setUserAgent(final String uaString)
    {
        final Semaphore sem = new Semaphore(0);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                m_webView.getSettings().setUserAgentString(uaString);
                sem.release();
            }
        });

        try {
            sem.tryAcquire(BLOCKING_TIMEOUT, TimeUnit.MILLISECONDS);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    String getUrl()
    {
        final String[] url = {""};
        final Semaphore sem = new Semaphore(0);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                WebSettings webSettings = m_webView.getSettings();
                url[0] = m_webView.getUrl();
                sem.release();
            }
        });

        try {
            sem.tryAcquire(BLOCKING_TIMEOUT, TimeUnit.MILLISECONDS);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return url[0];
    }

    void loadUrl(final String url)
    {
        if (url == null) {
            return;
        }

        resetLoadingState(STARTED_STATE);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() { m_webView.loadUrl(url); }
        });
    }

    void loadData(final String data, final String mimeType, final String encoding)
    {
        if (data == null)
            return;

        resetLoadingState(STARTED_STATE);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() { m_webView.loadData(data, mimeType, encoding); }
        });
    }

    void loadDataWithBaseURL(final String baseUrl,
                                    final String data,
                                    final String mimeType,
                                    final String encoding,
                                    final String historyUrl)
    {
        if (data == null)
            return;

        resetLoadingState(STARTED_STATE);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() { m_webView.loadDataWithBaseURL(baseUrl, data, mimeType, encoding, historyUrl); }
        });
    }

    void goBack()
    {
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() { m_webView.goBack(); }
        });
    }

    boolean canGoBack()
    {
        final boolean[] back = {false};
        final Semaphore sem = new Semaphore(0);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() { back[0] = m_webView.canGoBack(); sem.release(); }
        });

        try {
            sem.tryAcquire(BLOCKING_TIMEOUT, TimeUnit.MILLISECONDS);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return back[0];
    }

    void goForward()
    {
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() { m_webView.goForward(); }
        });
    }

    boolean canGoForward()
    {
        final boolean[] forward = {false};
        final Semaphore sem = new Semaphore(0);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() { forward[0] = m_webView.canGoForward(); sem.release(); }
        });

        try {
            sem.tryAcquire(BLOCKING_TIMEOUT, TimeUnit.MILLISECONDS);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return forward[0];
    }

    void stopLoading()
    {
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() { m_webView.stopLoading(); }
        });
    }

    void reload()
    {
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() { m_webView.reload(); }
        });
    }

    String getTitle()
    {
        final String[] title = {""};
        final Semaphore sem = new Semaphore(0);
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() { title[0] = m_webView.getTitle(); sem.release(); }
        });

        try {
            sem.tryAcquire(BLOCKING_TIMEOUT, TimeUnit.MILLISECONDS);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return title[0];
    }

    int getProgress()
    {
        return m_progress;
    }

    boolean isLoading()
    {
        return m_loadingState == LOADING_STATE || m_loadingState == STARTED_STATE || (m_progress > 0 && m_progress < 100);
    }

    void runJavaScript(final String script, final long callbackId)
    {
        if (script == null)
            return;

        if (Build.VERSION.SDK_INT < 19 || m_webViewEvaluateJavascript == null)
            return;

        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                try {
                    m_webViewEvaluateJavascript.invoke(m_webView, script, callbackId == -1 ? null :
                        new ValueCallback<String>() {
                            @Override
                            public void onReceiveValue(String result) {
                                c_onRunJavaScriptResult(m_id, callbackId, result);
                            }
                        });
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
    }

    WebView getWebView()
    {
       return m_webView;
    }

    void onPause()
    {
        if (m_webViewOnPause == null)
            return;

        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() { try { m_webViewOnPause.invoke(m_webView); } catch (Exception e) { e.printStackTrace(); } }
        });
    }

    void onResume()
    {
        if (m_webViewOnResume == null)
            return;

        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() { try { m_webViewOnResume.invoke(m_webView); } catch (Exception e) { e.printStackTrace(); } }
        });
    }

    private static boolean hasLocationPermission(View view)
    {
        final String name = view.getContext().getPackageName();
        final PackageManager pm = view.getContext().getPackageManager();
        return pm.checkPermission("android.permission.ACCESS_FINE_LOCATION", name) == PackageManager.PERMISSION_GRANTED;
    }

    void destroy()
    {
        m_activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                m_webView.destroy();
            }
        });
    }

    private static void setCookieImp(final String url, final String cookieString, ValueCallback<Boolean> callback)
    {
        CookieManager cookieManager = CookieManager.getInstance();
        cookieManager.setAcceptCookie(true);

        try {
            cookieManager.setCookie(url, cookieString, callback);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    static void setCookie(final long id, final String url, final String cookieString)
    {
        setCookieImp(url, cookieString, new ValueCallback<Boolean>() {
            @Override
            public void onReceiveValue(Boolean value) {
                try {
                    c_onCookieAdded(id, value, url, cookieString.split("=")[0]);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
    }

    private static boolean hasValidCookie(final String url, final String cookieString)
    {
        CookieManager cookieManager = CookieManager.getInstance();
        cookieManager.removeExpiredCookie();
        boolean cookieFound = false;

        final String domainCookie = cookieManager.getCookie(url);

        String found = null;
        if (domainCookie != null) {
            String cookies[] = domainCookie.split(";");
            for (final String cookie : cookies) {
                if (cookie.startsWith(cookieString)) {
                    found = cookie;
                    // Cookie is "cleared" so not considered valid.
                    cookieFound = !cookie.endsWith("=");
                    break;
                }
            }
        }

        return cookieFound;
    }

    private static String getExpireString()
    {
        return "expires=\"Thu, 1 Jan 1970 00:00:00 GMT\"";
    }

    static void removeCookie(final long id, final String url, final String cookieString)
    {
        // We need to work with what we have
        // 1. Check if there's cookies for the url
        final boolean hadCookie = hasValidCookie(url, cookieString);
        if (hadCookie) {
            // 2. Tag the string with an expire tag so it will be purged
            final String removeCookieString = cookieString + ";" + getExpireString();
            setCookieImp(url, removeCookieString, new ValueCallback<Boolean>() {
                @Override
                public void onReceiveValue(Boolean value) {
                    try {
                        // 3. Verify that the cookie was indeed removed
                        final boolean removed = (hadCookie && !hasValidCookie(url, cookieString));
                        c_onCookieRemoved(id, removed, url, cookieString.split("=")[0]);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            });
        }
    }

    static void removeCookies() {
        try {
            CookieManager.getInstance().removeAllCookies(null);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
