// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

package org.qtproject.qt.android.tst_qtquickview_functions;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import java.lang.Integer;
import java.lang.Boolean;
import java.lang.String;
import java.lang.Double;
import java.lang.Float;

import java.lang.reflect.Method;

import org.qtproject.qt.android.QtQmlStatus;
import org.qtproject.qt.android.QtQmlStatusChangeListener;
import org.qtproject.qt.android.QtQuickView;
import org.qtproject.qt.android.tst_qtquickview_functions_qml.TestViewModule.TestView;

public class TestActivity extends Activity implements QtQmlStatusChangeListener
{
    native void onInvalidFunctionsTestFinished(boolean success);

    private final TestView m_testView = new TestView();
    private QtQuickView m_quickView;
    private static TestActivity m_instance;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        m_instance = this;
        m_quickView = new QtQuickView(this);

        final FrameLayout qmlFrame = findViewById(R.id.qmlFrame);
        qmlFrame.addView(m_quickView);
        m_quickView.loadContent(m_testView);
    }

    @Override
    public void onStatusChanged(QtQmlStatus qtQmlStatus) {
        if (qtQmlStatus == QtQmlStatus.ERROR)
            finish(); // Error loading QML tests, just exit the app
    }

    public TestView testView() { return m_testView; }

    public static TestActivity instance() { return m_instance; }

    void callFunction(String name) {
        runOnUiThread(() -> m_quickView.invokeMethod(name));
    }

    void callFunctionWithParams(String name, int i, double d, double r, String s, boolean b) {
        runOnUiThread(() -> {
            m_quickView.invokeMethod(name, new Object[] {
                Integer.valueOf(i),
                Double.valueOf(d),
                Double.valueOf(r),
                s,
                Boolean.valueOf(b)
            });
        });
    }

    void callFunctionQuickViewContent() {
        runOnUiThread(() -> m_testView.emptyFunction());
    }

    void callFunctionWithParamsQuickViewContent(int i, double d, double r, String s, boolean b) {
        runOnUiThread(() -> m_testView.parameterisedFunction(Integer.valueOf(i), Double.valueOf(d), Double.valueOf(r), s, Boolean.valueOf(b)));
    }

    void verifyFunctionsDontExist(String[] names) {
        final Method[] contentMethods = m_testView.getClass().getMethods();
        for (String name : names) {
            for (Method m : contentMethods) {
                if (m.getName() == name) {
                    onInvalidFunctionsTestFinished(false);
                    return;
                }
            }
        }
        onInvalidFunctionsTestFinished(true);
    }
}
