// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QJniObject>
#include <QSet>
#include <QString>
#include <QTest>

constexpr int GET_PERMISSIONS(0x00001000);

using namespace QNativeInterface;
using namespace Qt::StringLiterals;

Q_DECLARE_JNI_CLASS(PackageManager, "android/content/pm/PackageManager")
Q_DECLARE_JNI_CLASS(PackageInfo, "android/content/pm/PackageInfo")

class tst_android_permissions : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void checkExpectedDefaults();
    void checkNonExisting();
    void checkNonDefaultPermissions();

private:
    QJniArray<QString> m_requestedPermissions;
};

void tst_android_permissions::initTestCase()
{
    QJniObject appCtx = QAndroidApplication::context();
    QVERIFY(appCtx.isValid());

    const auto packageName = appCtx.callMethod<QString>("getPackageName");
    const auto packageManager = appCtx.callMethod<QtJniTypes::PackageManager>("getPackageManager");
    QVERIFY(packageManager.isValid());

    const auto packageInfo = QJniObject(packageManager.callMethod<QtJniTypes::PackageInfo>(
            "getPackageInfo", packageName, jint(GET_PERMISSIONS)));
    QVERIFY(packageInfo.isValid());

    m_requestedPermissions = packageInfo.getField<QJniArray<QString>>("requestedPermissions");
    QVERIFY(m_requestedPermissions.isValid());
}

void tst_android_permissions::checkExpectedDefaults()
{
    QSet<QString> expectedDefaults{ { "android.permission.INTERNET"_L1 },
                                    { "android.permission.WRITE_EXTERNAL_STORAGE"_L1 },
                                    { "android.permission.READ_EXTERNAL_STORAGE"_L1 } };

    for (const auto &permission : m_requestedPermissions)
        expectedDefaults.remove(permission);

    QVERIFY(expectedDefaults.empty());
}

void tst_android_permissions::checkNonExisting()
{
    for (const auto &permission : m_requestedPermissions)
        QCOMPARE_NE(permission, "android.permission.BLUETOOTH_SCAN");
}

void tst_android_permissions::checkNonDefaultPermissions()
{
    bool hasNonDefaultPermissions = false;
    for (const auto &permission : m_requestedPermissions) {
        if (permission == "android.permission.ACCESS_COARSE_LOCATION")
            hasNonDefaultPermissions = true;
    }

    QVERIFY(hasNonDefaultPermissions);
}

QTEST_MAIN(tst_android_permissions);

#include "tst_android_permissions.moc"
