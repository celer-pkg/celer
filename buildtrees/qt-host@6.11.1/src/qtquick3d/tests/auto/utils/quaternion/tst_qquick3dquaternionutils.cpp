// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>
#include <QtNumeric>

#include <QtQuick3D/private/qquick3dquaternionutils_p.h>

class tst_qquick3dquaternionutils : public QObject
{
    Q_OBJECT

private slots:
    // fromAxisAndAngle
    void fromAxisAndAngle_identity();
    void fromAxisAndAngle_rotation();
    void fromAxisAndAngle_overloads();
    void fromAxisAndAngle_negativeAngle();
    void fromAxisAndAngle_zeroAxis();
    void fromAxisAndAngle_fullRotation();

    // fromEulerAngles
    void fromEulerAngles_identity();
    void fromEulerAngles_overloads();
    void fromEulerAngles_rotation();
    void fromEulerAngles_largeAngles();
    void fromEulerAngles_gimbalLock();

    // fromAxesAndAngles
    void fromAxesAndAngles_twoAxes();
    void fromAxesAndAngles_threeAxes();
    void fromAxesAndAngles_zeroAngles();
    void fromAxesAndAngles_nonOrthogonal();
    void fromAxesAndAngles_vectorCheck_twoAxes();
    void fromAxesAndAngles_vectorCheck_threeAxes();

    // lookAt
    void lookAt_identity();
    void lookAt_simple();
    void lookAt_sourceEqualsTarget();
    void lookAt_parallelUpForward();
    void lookAt_behindSource();
    void lookAt_vectorCheck_simple();
    void lookAt_vectorCheck_behind();
    void lookAt_vectorCheck_arbitrary();
    void lookAt_vectorCheck_parallelUpForward();
    void lookAt_QTBUG_143886();
};

// For quaternions a and b, they are equivalent if a == b OR a == -b
static bool fuzzyQuaternionCompare(const QQuaternion &a, const QQuaternion &b)
{
    return qFuzzyCompare(a, b) || qFuzzyCompare(a, -b);
}

void tst_qquick3dquaternionutils::fromAxisAndAngle_identity()
{
    const QQuaternion q = QQuick3DQuaternionUtils::fromAxisAndAngle(QVector3D(0, 1, 0), 0.0f);
    QVERIFY(q.isIdentity());
}

void tst_qquick3dquaternionutils::fromAxisAndAngle_rotation()
{
    const QQuaternion q = QQuick3DQuaternionUtils::fromAxisAndAngle(QVector3D(0, 1, 0), 90.0f);
    const QVector3D rotated = q.rotatedVector(QVector3D(1, 0, 0));
    QVERIFY(qFuzzyCompare(rotated, QVector3D(0.0f, 0.0f, -1.0f)));
}

void tst_qquick3dquaternionutils::fromAxisAndAngle_overloads()
{
    const QQuaternion q1 = QQuick3DQuaternionUtils::fromAxisAndAngle(0, 0, 1, 45.0f);
    const QQuaternion q2 = QQuick3DQuaternionUtils::fromAxisAndAngle(QVector3D(0, 0, 1), 45.0f);
    QVERIFY(fuzzyQuaternionCompare(q1, q2));
}

void tst_qquick3dquaternionutils::fromAxisAndAngle_negativeAngle()
{
    const QQuaternion q = QQuick3DQuaternionUtils::fromAxisAndAngle(QVector3D(0, 1, 0), -90.0f);
    const QVector3D rotated = q.rotatedVector(QVector3D(1, 0, 0));
    QVERIFY(qFuzzyCompare(rotated, QVector3D(0.0f, 0.0f, 1.0f)));
}

void tst_qquick3dquaternionutils::fromAxisAndAngle_zeroAxis()
{
    const QQuaternion q = QQuick3DQuaternionUtils::fromAxisAndAngle(QVector3D(0, 0, 0), 90.0f);
    QVERIFY(q.isIdentity());
}

void tst_qquick3dquaternionutils::fromAxisAndAngle_fullRotation()
{
    const QQuaternion q = QQuick3DQuaternionUtils::fromAxisAndAngle(QVector3D(0, 1, 0), 360.0f);
    QVERIFY(fuzzyQuaternionCompare(q, QQuaternion())); // Fuzzy check identity
}

void tst_qquick3dquaternionutils::fromEulerAngles_identity()
{
    const QQuaternion q = QQuick3DQuaternionUtils::fromEulerAngles(0.0f, 0.0f, 0.0f);
    QVERIFY(q.isIdentity());
}

void tst_qquick3dquaternionutils::fromEulerAngles_overloads()
{
    const QQuaternion q1 = QQuick3DQuaternionUtils::fromEulerAngles(10, 20, 30);
    const QQuaternion q2 = QQuick3DQuaternionUtils::fromEulerAngles(QVector3D(10, 20, 30));
    QVERIFY(fuzzyQuaternionCompare(q1, q2));
}

void tst_qquick3dquaternionutils::fromEulerAngles_rotation()
{
    const QQuaternion q = QQuick3DQuaternionUtils::fromEulerAngles(0, 90, 0);
    const QVector3D rotated = q.rotatedVector(QVector3D(1, 0, 0));
    QVERIFY(qFuzzyIsNull(rotated.x()));
    QVERIFY(qFuzzyCompare(rotated.z(), -1.0f));
}

void tst_qquick3dquaternionutils::fromEulerAngles_largeAngles()
{
    const QQuaternion q = QQuick3DQuaternionUtils::fromEulerAngles(0, 450, 0); // 360+90
    const QVector3D rotated = q.rotatedVector(QVector3D(1, 0, 0));
    QVERIFY(qFuzzyCompare(rotated.normalized(), QVector3D(0, 0, -1)));
}

void tst_qquick3dquaternionutils::fromEulerAngles_gimbalLock()
{
    const QQuaternion q = QQuick3DQuaternionUtils::fromEulerAngles(0, 90, 90);
    QVERIFY(!q.isIdentity());
    QVERIFY(qFuzzyCompare(q.length(), 1.0f));
}

void tst_qquick3dquaternionutils::fromAxesAndAngles_twoAxes()
{
    const QQuaternion q = QQuick3DQuaternionUtils::fromAxesAndAngles(QVector3D(1, 0, 0), 90, QVector3D(0, 1, 0), 90);
    QVERIFY(!q.isIdentity());
    QVERIFY(qFuzzyCompare(q.length(), 1.0f));
}

void tst_qquick3dquaternionutils::fromAxesAndAngles_threeAxes()
{
    const QQuaternion q = QQuick3DQuaternionUtils::fromAxesAndAngles(QVector3D(1, 0, 0), 30, QVector3D(0, 1, 0), 45, QVector3D(0, 0, 1), 60);
    QVERIFY(!q.isIdentity());
    QVERIFY(qFuzzyCompare(q.length(), 1.0f));
}

void tst_qquick3dquaternionutils::fromAxesAndAngles_zeroAngles()
{
    const QQuaternion q = QQuick3DQuaternionUtils::fromAxesAndAngles(QVector3D(1, 0, 0), 0, QVector3D(0, 1, 0), 0);
    QVERIFY(q.isIdentity());
}

void tst_qquick3dquaternionutils::fromAxesAndAngles_nonOrthogonal()
{
    const QQuaternion q = QQuick3DQuaternionUtils::fromAxesAndAngles(QVector3D(1, 1, 0).normalized(),
                                                                     45,
                                                                     QVector3D(0, 1, 1).normalized(),
                                                                     30);
    QVERIFY(!q.isIdentity());
    QVERIFY(qFuzzyCompare(q.length(), 1.0f));
}

void tst_qquick3dquaternionutils::fromAxesAndAngles_vectorCheck_twoAxes()
{
    const QQuaternion q = QQuick3DQuaternionUtils::fromAxesAndAngles(QVector3D(1, 0, 0), 90.0f, QVector3D(0, 1, 0), 90.0f);

    // Rotate the basis vectors
    const QVector3D xr = q.rotatedVector(QVector3D(1, 0, 0));
    const QVector3D yr = q.rotatedVector(QVector3D(0, 1, 0));
    const QVector3D zr = q.rotatedVector(QVector3D(0, 0, 1));

    // Check that lengths remain 1 (normalization sanity)
    QVERIFY(qFuzzyCompare(xr.length(), 1.0f));
    QVERIFY(qFuzzyCompare(yr.length(), 1.0f));
    QVERIFY(qFuzzyCompare(zr.length(), 1.0f));

    // Check that axes are still orthogonal
    QVERIFY(qFuzzyIsNull(QVector3D::dotProduct(xr, yr)));
    QVERIFY(qFuzzyIsNull(QVector3D::dotProduct(xr, zr)));
    QVERIFY(qFuzzyIsNull(QVector3D::dotProduct(yr, zr)));

    // Optionally, check that the rotation produced a permutation of the original axes
    // (e.g., xr might be along original Z, etc.)
    QVERIFY(!qFuzzyCompare(xr, QVector3D(1, 0, 0)));
    QVERIFY(!qFuzzyCompare(yr, QVector3D(0, 1, 0)));
    QVERIFY(!qFuzzyCompare(zr, QVector3D(0, 0, 1)));
}

void tst_qquick3dquaternionutils::fromAxesAndAngles_vectorCheck_threeAxes()
{
    const QQuaternion q = QQuick3DQuaternionUtils::fromAxesAndAngles(QVector3D(1, 0, 0), 30, QVector3D(0, 1, 0), 45, QVector3D(0, 0, 1), 60);
    const QVector3D x(1, 0, 0);
    const QVector3D y(0, 1, 0);
    const QVector3D z(0, 0, 1);
    QVERIFY(qFuzzyCompare(q.rotatedVector(x).length(), 1.0f));
    QVERIFY(qFuzzyCompare(q.rotatedVector(y).length(), 1.0f));
    QVERIFY(qFuzzyCompare(q.rotatedVector(z).length(), 1.0f));
    QVERIFY(!qFuzzyCompare(q.rotatedVector(x), x));
    QVERIFY(!qFuzzyCompare(q.rotatedVector(y), y));
    QVERIFY(!qFuzzyCompare(q.rotatedVector(z), z));
}

void tst_qquick3dquaternionutils::lookAt_identity()
{
    const QQuaternion q = QQuick3DQuaternionUtils::lookAt(QVector3D(0, 0, 0), QVector3D(0, 0, -1));
    QVERIFY(q.isIdentity());
}

void tst_qquick3dquaternionutils::lookAt_simple()
{
    const QQuaternion q = QQuick3DQuaternionUtils::lookAt(QVector3D(0, 0, 0), QVector3D(1, 0, 0));
    const QVector3D rotated = q.rotatedVector(QVector3D(0, 0, -1));
    QVERIFY(qFuzzyCompare(rotated.normalized(), QVector3D(1, 0, 0)));
}

void tst_qquick3dquaternionutils::lookAt_sourceEqualsTarget()
{
    const QQuaternion q = QQuick3DQuaternionUtils::lookAt(QVector3D(1, 2, 3), QVector3D(1, 2, 3));
    QEXPECT_FAIL("", "lookAt does not handle same start end position", Continue);
    QVERIFY(q.isIdentity());
}

void tst_qquick3dquaternionutils::lookAt_parallelUpForward()
{
    const QQuaternion q = QQuick3DQuaternionUtils::lookAt(QVector3D(0, 0, 0), QVector3D(0, 0, 1), QVector3D(0, 0, 1), QVector3D(0, 0, 1));
    const QVector3D rotated = q.rotatedVector(QVector3D(0, 0, 1)).normalized();
    QVERIFY(qFuzzyCompare(rotated, QVector3D(0, 0, 1)));
}

void tst_qquick3dquaternionutils::lookAt_behindSource()
{
    const QQuaternion q = QQuick3DQuaternionUtils::lookAt(QVector3D(0, 0, 0), QVector3D(0, 0, 1));
    const QVector3D rotated = q.rotatedVector(QVector3D(0, 0, -1)).normalized();
    QVERIFY(qFuzzyCompare(rotated, QVector3D(0, 0, 1)));
}

void tst_qquick3dquaternionutils::lookAt_vectorCheck_simple()
{
    QVector3D source(0, 0, 0);
    QVector3D target(1, 0, 0);
    const QQuaternion q = QQuick3DQuaternionUtils::lookAt(source, target);
    QVector3D rotated = q.rotatedVector(QVector3D(0, 0, -1)).normalized();
    QVERIFY(qFuzzyCompare(rotated, QVector3D(1, 0, 0)));
}

void tst_qquick3dquaternionutils::lookAt_vectorCheck_behind()
{
    QVector3D source(0, 0, 0);
    QVector3D target(0, 0, 1);
    const QQuaternion q = QQuick3DQuaternionUtils::lookAt(source, target);
    QVector3D rotated = q.rotatedVector(QVector3D(0, 0, -1)).normalized();
    QVERIFY(qFuzzyCompare(rotated, QVector3D(0, 0, 1)));
}

void tst_qquick3dquaternionutils::lookAt_vectorCheck_arbitrary()
{
    QVector3D source(1, 2, 3);
    QVector3D target(4, 6, 3);
    const QQuaternion q = QQuick3DQuaternionUtils::lookAt(source, target);
    QVector3D rotated = q.rotatedVector(QVector3D(0, 0, -1)).normalized();
    QVector3D expected = (target - source).normalized();
    QVERIFY(qFuzzyCompare(rotated, expected));
}

void tst_qquick3dquaternionutils::lookAt_vectorCheck_parallelUpForward()
{
    QVector3D source(0, 0, 0);
    QVector3D target(0, 0, 1);
    QVector3D forward(0, 0, 1);
    QVector3D up(0, 0, 1);
    const QQuaternion q = QQuick3DQuaternionUtils::lookAt(source, target, forward, up);
    QVector3D rotated = q.rotatedVector(forward).normalized();
    QVector3D expected = (target - source).normalized();
    QVERIFY(qFuzzyCompare(rotated, expected));
}

void tst_qquick3dquaternionutils::lookAt_QTBUG_143886()
{
    QVector3D source(0.47212299704551697, -0.05714830011129379, -0.03842129930853844);
    QVector3D target(0.0174984410405159, -0.1452823281288147, -0.038421452045440674);
    QVector3D forward(-0.9816991090774536, -0.19043900072574615, 0);
    QVector3D up(0, 1, 0);

    const QQuaternion q = QQuick3DQuaternionUtils::lookAt(source, target, forward, up);
    QVERIFY(fuzzyQuaternionCompare(q, QQuaternion()));
}

QTEST_APPLESS_MAIN(tst_qquick3dquaternionutils)

#include "tst_qquick3dquaternionutils.moc"
