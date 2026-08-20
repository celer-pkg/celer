// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef UTIL_H
#define UTIL_H

#include <QEventLoop>
#include <QSignalSpy>
#include <QTimer>
#include <QtTest/QtTest>
#include <QtWebViewQuick/private/qquickwebview_p.h>
#include <QtWebViewQuick/private/qquickwebviewloadrequest_p.h>

#if !defined(TESTS_SOURCE_DIR)
#define TESTS_SOURCE_DIR ""
#endif

class LoadSpy : public QEventLoop {
    Q_OBJECT

public:
    LoadSpy(QQuickWebView *webView)
    {
        connect(webView, SIGNAL(loadingChanged(QQuickWebViewLoadRequest*)), SLOT(onLoadingChanged(QQuickWebViewLoadRequest*)));
    }

    ~LoadSpy() { }

Q_SIGNALS:
    void loadSucceeded();
    void loadFailed();

private Q_SLOTS:
    void onLoadingChanged(QQuickWebViewLoadRequest *loadRequest)
    {
        if (loadRequest->status() == QQuickWebView::LoadSucceededStatus)
            emit loadSucceeded();
        else if (loadRequest->status() == QQuickWebView::LoadFailedStatus)
            emit loadFailed();
    }
};

class LoadStartedCatcher : public QObject {
    Q_OBJECT

public:
    LoadStartedCatcher(QQuickWebView *webView)
        : m_webView(webView)
    {
        connect(m_webView, SIGNAL(loadingChanged(QQuickWebViewLoadRequest*)), this, SLOT(onLoadingChanged(QQuickWebViewLoadRequest*)));
    }

    virtual ~LoadStartedCatcher() { }

public Q_SLOTS:
    void onLoadingChanged(QQuickWebViewLoadRequest *loadRequest)
    {
        if (loadRequest->status() == QQuickWebView::LoadStartedStatus)
            QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
    }

Q_SIGNALS:
    void finished();

private:
    QQuickWebView *m_webView;
};

/**
 * Starts an event loop that runs until the given signal is received.
 * Optionally the event loop
 * can return earlier on a timeout.
 *
 * \return \p true if the requested signal was received
 *         \p false on timeout
 */
inline bool waitForSignal(QObject *obj, const char *signal, int timeout = 10000)
{
    QEventLoop loop;
    QObject::connect(obj, signal, &loop, SLOT(quit()));
    QTimer timer;
    QSignalSpy timeoutSpy(&timer, SIGNAL(timeout()));
    if (timeout > 0) {
        QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.setSingleShot(true);
        timer.start(timeout);
    }
    loop.exec();
    return timeoutSpy.isEmpty();
}

inline bool waitForLoadSucceeded(QQuickWebView *webView, int timeout = 10000)
{
    LoadSpy loadSpy(webView);
    return waitForSignal(&loadSpy, SIGNAL(loadSucceeded()), timeout);
}

inline bool waitForLoadFailed(QQuickWebView *webView, int timeout = 10000)
{
    LoadSpy loadSpy(webView);
    return waitForSignal(&loadSpy, SIGNAL(loadFailed()), timeout);
}

struct Cookie
{
    struct SigArg
    {
        QString domain;
        QString name;
    };

    using List = QList<Cookie>;
    using SignalReturnValues = QList<QList<QVariant>>;

    QString domain;
    QString name;
    QString value;
    friend bool operator==(const Cookie &a, const Cookie::SigArg &b)
    {
        return (a.domain == b.domain) && (a.name == b.name);
    }

    static bool testSignalValues(const Cookie::List &cookies, const SignalReturnValues &sigValues)
    {
        if (cookies.size() != sigValues.size())
            return false;

        int found = 0;
        for (const auto &cookie : cookies) {
            auto it = std::find_if(sigValues.constBegin(), sigValues.constEnd(), [cookie](const QVariantList &sigArgs) {
                return (cookie == Cookie::SigArg{sigArgs.at(0).toString(), sigArgs.at(1).toString() });
            });
            if (it != sigValues.constEnd())
                ++found;
        }

        return (found == cookies.size());
    }
};

#endif /* UTIL_H */
