// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DLIGHTMAPBAKER_P_H
#define QQUICK3DLIGHTMAPBAKER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick3D/qtquick3dglobal.h>
#include <QtQuick/private/qquickview_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DViewport;

class Q_QUICK3D_EXPORT QQuick3DLightmapBaker : public QObject
{
    Q_OBJECT
public:
    enum class BakingStatus {
        None,
        Info,
        Warning,
        Error,
        Cancelled,
        Failed,
        Complete
    };

    struct BakingControl {
        void reset();
        void requestCancel();
        bool isCancelled() const;

    private:
        bool cancelFlag = false;
    };

    typedef std::function<void(const QVariantMap &, BakingControl*)> Callback;

    explicit QQuick3DLightmapBaker(QQuick3DViewport *view);
    ~QQuick3DLightmapBaker();

    void bake(Callback callback);
    void bake();

    void denoise(Callback callback);
    void denoise();

private slots:
    void onLmCancelButtonClicked();
    void onLmWindowClosing(QQuickCloseEvent *event);

private:
    void updateView();

    bool m_bakingRequested = false;
    bool m_denoisingRequested = false;
    bool m_currentlyBaking = false;
    BakingControl *m_bakingControl = nullptr;
    QQuick3DViewport *m_view = nullptr;
    Callback m_callback;

    // For the internal status/control provided by the default impl through DebugView
    QQuickView *m_lmWindow = nullptr;
    bool m_windowCancelRequested = false;

    friend class QQuick3DSceneRenderer;
};

QT_END_NAMESPACE


#endif // QQUICK3DLIGHTMAPBAKER_P_H
        ;
