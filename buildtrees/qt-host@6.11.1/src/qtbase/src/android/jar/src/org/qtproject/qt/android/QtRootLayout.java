// Copyright (C) 2023 The Qt Company Ltd.
// Copyright (C) 2012 BogDan Vatra <bogdan@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt.android;

import android.app.Activity;
import android.content.Context;
import android.content.res.Configuration;

/**
    A layout which corresponds to one Activity, i.e. is the root layout where the top level window
    and handles orientation changes.
*/
class QtRootLayout extends QtLayout
{
    QtRootLayout(Context context)
    {
        super(context);
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh)
    {
        Activity activity = (Activity) getContext();
        if (activity == null || (w == oldw && h == oldh))
            return;

        QtDisplayManager.handleLayoutSizeChanged(w, h);
    }

    @Override
    public void onConfigurationChanged(Configuration configuration)
    {
        Activity activity = (Activity) getContext();
        if (activity == null)
            return;

        // Post the orientation handling just in case the onSizeChanged() is
        // called a bit late (QTBUG-94459).
        QtNative.runAction(() -> {
            QtDisplayManager.handleOrientationChange(activity);
        });
    }
}
