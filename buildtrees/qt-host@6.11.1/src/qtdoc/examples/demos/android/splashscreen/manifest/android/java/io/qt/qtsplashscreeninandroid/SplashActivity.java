// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
package io.qt.qtsplashscreeninandroid;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.util.Log;
import android.content.Intent;
import android.view.ViewTreeObserver;
import org.qtproject.qt.android.bindings.QtActivity;

public class SplashActivity extends Activity {

    private static final String TAG = "SplashActivity";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Set up an OnPreDrawListener to the root view.
        final View content = findViewById(android.R.id.content);
                content.getViewTreeObserver().addOnPreDrawListener(
                        new ViewTreeObserver.OnPreDrawListener() {
            @Override
            public boolean onPreDraw() {
                Intent intent = new Intent(SplashActivity.this, QtActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NO_ANIMATION);
                startActivity(intent);

                content.getViewTreeObserver().removeOnPreDrawListener(this);
                finish();
                return true;
            }
        });
    }
}
