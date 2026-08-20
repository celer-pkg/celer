// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testactivitycommunicator.h"

using namespace QtJniTypes;

static TestActivityCommunicator *s_instance = nullptr;

TestActivityCommunicator::TestActivityCommunicator(QObject *parent)
    : QObject{ parent },
      m_activity(TestActivity::callStaticMethod<TestActivity>("instance")),
      m_view(m_activity.callMethod<TestView>("testView"))
{
    s_instance = this;
    TestActivity::registerNativeMethods({ Q_JNI_NATIVE_SCOPED_METHOD(onInvalidFunctionsTestFinished,
                                                                     TestActivityCommunicator) });
}

void TestActivityCommunicator::callFunction(const QString &name)
{
    m_activity.callMethod("callFunction", name);
}

void TestActivityCommunicator::callFunctionWithParams(const QString &name, int i, double d,
                                                      double r, const QString &s, bool b)
{
    m_activity.callMethod("callFunctionWithParams", name, i, d, r, s, b);
}

void TestActivityCommunicator::callFunctionQuickViewContent()
{
    m_activity.callMethod("callFunctionQuickViewContent");
}

void TestActivityCommunicator::callFunctionWithParamsQuickViewContent(int i, double d, double r,
                                                                      const QString &s, bool b)
{
    m_activity.callMethod("callFunctionWithParamsQuickViewContent", i, d, r, s, b);
}

void TestActivityCommunicator::testInvalidFunctions(const QStringList &functions)
{
    m_activity.callMethod("verifyFunctionsDontExist", QJniArray<jstring>::fromContainer(functions));
}

void TestActivityCommunicator::onInvalidFunctionsTestFinished(JNIEnv *, jobject, bool success)
{
    Q_ASSERT(s_instance);
    emit s_instance->onInvalidFunctionsTestFinished(success);
}
