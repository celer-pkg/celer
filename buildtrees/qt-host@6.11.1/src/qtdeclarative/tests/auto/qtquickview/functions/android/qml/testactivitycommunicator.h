// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TESTACTIVITYCOMMUNICATOR_H
#define TESTACTIVITYCOMMUNICATOR_H

#include <QtCore/qobject.h>
#include <QtQml/qqmlengine.h>
#include <QtCore/qjnitypes.h>

Q_DECLARE_JNI_CLASS(TestActivity, "org/qtproject/qt/android/tst_qtquickview_functions/TestActivity")
Q_DECLARE_JNI_CLASS(
        TestView, "org/qtproject/qt/android/tst_qtquickview_functions_qml/TestViewModule/TestView")
class TestActivityCommunicator : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit TestActivityCommunicator(QObject *parent = nullptr);

public slots:
    void callFunction(const QString &name);
    void callFunctionWithParams(const QString &name, int i, double d, double r, const QString &s,
                                bool b);

    void callFunctionQuickViewContent();
    void callFunctionWithParamsQuickViewContent(int i, double d, double r, const QString &s,
                                                bool b);

    void testInvalidFunctions(const QStringList &functions);

signals:
    void onInvalidFunctionsTestFinished(bool success);

private:
    QtJniTypes::TestActivity m_activity;
    QtJniTypes::TestView m_view;

    static void onInvalidFunctionsTestFinished(JNIEnv *, jobject, bool success);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(onInvalidFunctionsTestFinished)
};

#endif // TESTACTIVITYCOMMUNICATOR_H
