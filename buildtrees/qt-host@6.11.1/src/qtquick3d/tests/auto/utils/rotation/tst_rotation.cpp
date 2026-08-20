// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>

#include <QtQuick3DUtils/private/qssgutils_p.h>

#include <QtGui/qquaternion.h>

class tst_RotationDataClass : public QObject
{
    Q_OBJECT

public:
    tst_RotationDataClass() = default;
    ~tst_RotationDataClass() = default;

    using Dirty = RotationData::Dirty;
    static bool isDirty(const RotationData &rd) { return (rd.m_dirty != Dirty::None); };

private slots:
    void test_initialState();
    void test_construct();
    void test_eulerAssign();
    void test_quatAssign();
    void test_aba();
    void test_compare();
    void test_compare2();
    void test_fuzzyQuatCompare();
    void test_compare_precision();
};

void tst_RotationDataClass::test_initialState()
{

    // Initial state
    RotationData rotation;
    QCOMPARE(isDirty(rotation), false);
    QVERIFY(qFuzzyCompare(QVector3D(rotation), QVector3D()));
    QVERIFY(qFuzzyCompare(QQuaternion(rotation), QQuaternion()));
}

void tst_RotationDataClass::test_construct()
{
    QVector3D eulerRot = QVector3D(1.0f, 0.0f, 0.0f);
    QQuaternion quatRot = QQuaternion::fromEulerAngles(eulerRot);

    RotationData a(eulerRot);
    QCOMPARE(isDirty(a), true);

    RotationData b(quatRot);
    QCOMPARE(isDirty(b), true);

    QEXPECT_FAIL("", "fromEulerAngles() introduces too much changes for RotationData's compare function", Continue);
    QCOMPARE(a, b);
    QCOMPARE(a.m_quatRot, b.m_quatRot); // Compensate for the XFAIL. Verify that the two quaternions a componentwise equal.
    // NOTE: Comparison is done based on the stored quaternion
    QCOMPARE(isDirty(a), false); // Dirty is cleared as 'a' was set using euler angles
    QCOMPARE(isDirty(b), true); // Still dirty, as the Euler angles where never compared and therefore not calculated
    QCOMPARE(b.m_dirty, Dirty::Euler);
    QVERIFY(qFuzzyCompare(QVector3D(b), eulerRot));
    QCOMPARE(isDirty(b), false); // Both internal values queried, so neither should be dirty


}

void tst_RotationDataClass::test_eulerAssign()
{
    RotationData rotation;
    QCOMPARE(isDirty(rotation), false);
    QVERIFY(qFuzzyCompare(QVector3D(rotation), QVector3D()));
    QVERIFY(qFuzzyCompare(QQuaternion(rotation), QQuaternion()));

    QVector3D eulerRot = QVector3D(1.0f, 0.0f, 0.0f);
    QQuaternion quatRot = QQuaternion::fromEulerAngles(eulerRot);

    rotation = eulerRot;
    QVERIFY(qFuzzyCompare(rotation.m_eulerRot, eulerRot));
    QCOMPARE(isDirty(rotation), true);

    {
        QCOMPARE(rotation.m_dirty, RotationData::Dirty::Quaternion);
        QQuaternion retQuatRot(rotation);
        QCOMPARE(isDirty(rotation), false);
        QVERIFY(qFuzzyCompare(retQuatRot, quatRot));
        QVERIFY(qFuzzyCompare(rotation.m_quatRot, quatRot));
    }
}

void tst_RotationDataClass::test_quatAssign()
{
    RotationData rotation;
    QCOMPARE(isDirty(rotation), false);
    QVERIFY(qFuzzyCompare(QVector3D(rotation), QVector3D()));
    QVERIFY(qFuzzyCompare(QQuaternion(rotation), QQuaternion()));

    QVector3D eulerRot = QVector3D(1.0f, 0.0f, 0.0f);
    QQuaternion quatRot = QQuaternion::fromEulerAngles(eulerRot);

    rotation = quatRot;
    QVERIFY(qFuzzyCompare(rotation.m_quatRot, quatRot));
    QCOMPARE(isDirty(rotation), true);

    {
        QCOMPARE(rotation.m_dirty, RotationData::Dirty::Euler);
        QVector3D retEulerRot(rotation);
        QCOMPARE(isDirty(rotation), false);
        QVERIFY(qFuzzyCompare(retEulerRot, eulerRot));
        QVERIFY(qFuzzyCompare(rotation.m_eulerRot, eulerRot));
    }
}

void tst_RotationDataClass::test_aba()
{
    // Dirty state should be mutually exclusive and the value
    // updated appropriately each time
    // (i.e., not skipping a valid write by testing agains an old value).
    RotationData rotation; // Init { 0, 0, 0}
    rotation = QVector3D(1.0f, 0.0f, 0.0f);
    // Quat dirty
    rotation = QQuaternion::fromEulerAngles(QVector3D(0.0f, 0.0f, 0.0f));
    // Euler dirty
    QCOMPARE(rotation, RotationData{});
}

void tst_RotationDataClass::test_compare()
{
    {
        RotationData a;
        RotationData b;
        QVERIFY(a == b);
        QVERIFY(b == a);

        a = RotationData(QVector3D(1.0f, 1.0f, 1.0f));
        QVERIFY(a != b);
        QVERIFY(b != a);

        a = RotationData(QQuaternion());
        QVERIFY(a == b);
        QVERIFY(b == a);
    }

    {
        RotationData a;
        QVector3D b;
        QVERIFY(a == b);
        QVERIFY(b == a);

        a = QVector3D(1.0f, 1.0f, 1.0f);
        QVERIFY(a != b);
        QVERIFY(b != a);

        a = QQuaternion();
        QVERIFY(a == b);
        QVERIFY(b == a);
    }

    {
        RotationData a;
        QQuaternion b;
        QVERIFY(a == b);
        QVERIFY(b == a);

        a = QVector3D(1.0f, 1.0f, 1.0f);
        QVERIFY(a != b);
        QVERIFY(b != a);

        a = QQuaternion();
        QVERIFY(a == b);
        QVERIFY(b == a);
    }
}

void tst_RotationDataClass::test_compare2()
{
    RotationData a;
    RotationData b;
    QVERIFY(a == b);
    QVERIFY(b == a);

    {
        const auto v = QVector3D();
        const auto qa = QQuaternion::fromEulerAngles(v);

        // Quat stored as normalized value ?
        QQuaternion qaNormalized = qa.normalized();
        a = qa;
        QVERIFY(a == qaNormalized);
        QVERIFY(a == v);

        b = -a;
        QVERIFY(a == b);
        QVERIFY(b == a);
    }

    {
        const auto v = QVector3D(0.0f, 0.0f, 45.0f);
        const auto qa = QQuaternion::fromEulerAngles(v);

        // Quat stored as normalized value ?
        QQuaternion qaNormalized = qa.normalized();
        {
            // Assumption 1: QQuaternion can be non-normalized but the RotationData class always
            // contains the normalized quaternion.
            {
                const auto qaNonNormalized = qa * 30.0f;
                QVERIFY(!qFuzzyCompare(qaNonNormalized, qaNormalized));
                {
                    RotationData rd(qaNonNormalized);
                    QVERIFY(rd.m_quatRot == qaNormalized);
                }

                {
                    RotationData rd;
                    rd = qaNonNormalized;
                    QVERIFY(rd.m_quatRot == qaNormalized);
                }
            }

            // Assumption 2: compare or fuzzy compare of QQuaternion is strictly a component by component compare
            auto qb = -qa;
            QVERIFY(!qFuzzyCompare(qb, qa));
        }
        a = qa;
        QEXPECT_FAIL("", "Normalization introduces too much changes for RotationData's compare function", Continue);
        QVERIFY(a == qaNormalized);
        QVERIFY(a.m_quatRot == qaNormalized); // Compensate for the XFAIL
        QVERIFY(a == v);

        b = -a;
        QEXPECT_FAIL("", "Negation introduces too much changes for RotationData's compare function", Continue);
        QVERIFY(a == b);
        QVERIFY(-a.m_quatRot == b.m_quatRot); // Compensate for the XFAIL
        QEXPECT_FAIL("", "Negation introduces too much changes for RotationData's compare function", Continue);
        QVERIFY(b == a);
        QVERIFY(b.m_quatRot == -a.m_quatRot); // Compensate for the XFAIL
    }
}

void tst_RotationDataClass::test_fuzzyQuatCompare()
{
    RotationData a;
    RotationData b;
    QVERIFY(a == b);
    QVERIFY(b == a);

    { // QTBUG-105918 (Verify that the divergence from 1.0f does not result in a negative number).
      // (i.e., -0.00001 <= e)
        const QQuaternion qa = QQuaternion(0.5f, 0.5f, 0.5f, 0.5f);
        const QQuaternion qb = QQuaternion(0.5f + 0.0007f, 0.5f, 0.5f, 0.5f);
        QVERIFY(qa != qb);
        QVERIFY(!qFuzzyCompare(qa, qb));
        a = qa;
        b = qb;
        QVERIFY(a != b);
        b = -qb;
        QVERIFY(a != b);
    }
}

void tst_RotationDataClass::test_compare_precision()
{
    // Compare precision between the component-by-component compare and the fuzzyQuaternion compare
    // These are just for monitoring, it is possible that these fail if e.g., the fudge factor is changed.
    RotationData a;
    RotationData b;
    QVERIFY(a == b);
    QVERIFY(b == a);

    {
        const QQuaternion qa = QQuaternion(0.5f, 0.5f, 0.5f, 0.5f);
        const QQuaternion qb = QQuaternion(0.5f + 0.0007f, 0.5f, 0.5f, 0.5f);
        QVERIFY(qa != qb);
        QVERIFY(!qFuzzyCompare(qa, qb));
        a = qa;
        b = qb;
        QVERIFY(a != b);
        b = -qb;
        QVERIFY(a != b);
    }
    {
        const QQuaternion qa = QQuaternion(0.5f, 0.5f, 0.5f, 0.5f);
        const QQuaternion qb = QQuaternion(0.5f, 0.5f + 0.001f, 0.5f, 0.5f);
        QVERIFY(qa != qb);
        QVERIFY(!qFuzzyCompare(qa, qb));
        a = qa;
        b = qb;
        QVERIFY(a != b);
        b = -qb;
        QVERIFY(a != b);
    }
    {
        const QQuaternion qa = QQuaternion(0.5f, 0.5f, 0.5f, 0.5f);
        const QQuaternion qb = QQuaternion(0.5f, 0.5f + 0.0007f, 0.5f, 0.5f);
        QVERIFY(qa != qb);
        QVERIFY(!qFuzzyCompare(qa, qb));
        a = qa;
        b = qb;
        QVERIFY(a != b);
        b = -qb;
        QVERIFY(a != b);
    }
    {
        const QQuaternion qa = QQuaternion(0.5f, 0.5f, 0.5f, 0.5f);
        const QQuaternion qb = QQuaternion(0.5f, 0.5f + 0.0007f, 0.5f, 0.5f);
        QVERIFY(qa != qb);
        QVERIFY(!qFuzzyCompare(qa, qb));
        a = qa;
        b = qb;
        QVERIFY(a != b);
        b = -qb;
        QVERIFY(a != b);
    }
    {
        const QQuaternion qa = QQuaternion(1000.f, 0.5f, 0.5f, 0.5f);
        const QQuaternion qb = QQuaternion(1000.f + 0.000001f, 0.5f + 0.000001f, 0.5f, 0.5f);
        QVERIFY(qa != qb);
        QVERIFY(qFuzzyCompare(qa, qb));
        a = qa;
        b = qb;
        QEXPECT_FAIL("", "Difference is too big...", Continue);
        QVERIFY(a == b);
        QVERIFY(qFuzzyCompare(a.m_quatRot, b.m_quatRot)); // Compensate for XFAIL
        b = -qb;
        QEXPECT_FAIL("", "Negation introduces change", Continue);
        QVERIFY(a == b);
        QVERIFY(qFuzzyCompare(-a.m_quatRot, b.m_quatRot)); // Compensate for XFAIL
    }
    // RotationData is preciser than qFuzzyCompare(QQuaternion)
    {
        const float e = std::numeric_limits<float>::epsilon();
        float fa = 1.0f;
        float fb = fa + e;
        QVERIFY(fa != fb);
        const QQuaternion qa = QQuaternion(1.0f, fa, 0.f, 0.f);
        const QQuaternion qb = QQuaternion(1.0f, fb, 0.f, 0.f);

        QVERIFY(qa != qb); // Component wise compare
        QVERIFY(qFuzzyCompare(qa, qb));
        a = qa;
        b = qb;
        QVERIFY(a != b);
        b = -qb;
        QVERIFY(a != b);
        { // Push QQuaternion past it's limits and verify that RotationData still picks up the difference
            const auto qad = qa / 2.0f;
            const auto qbd = qa / 2.0f;

            QVERIFY(qad == qbd); // Component wise compare (now below the threshold).
            QVERIFY(qFuzzyCompare(qad, qbd));
            a = qad;
            b = qbd;
            QVERIFY(a.m_quatRot == b.m_quatRot);
            // Rotation data should still treat these as being different
            QVERIFY(a != b);
            b = -qbd;
            QVERIFY(a != b);
        }
    }
    {
        const QQuaternion qa = QQuaternion(0.5f, 0.5f, 0.5f, 0.5f);
        const QQuaternion qb = QQuaternion(0.5f, 0.5f + 0.000001f, 0.5f, 0.5f);
        QVERIFY(qa != qb);
        QVERIFY(qFuzzyCompare(qa, qb));
        a = qa;
        b = qb;
        QVERIFY(a != b);
        b = -qb;
        QVERIFY(a != b);
    }
    {
        const QQuaternion qa = QQuaternion(0.5f, 0.5f, 0.5f, 0.5f);
        const QQuaternion qb = QQuaternion(0.5f + 0.000001f, 0.5f + 0.000001f, 0.5f, 0.5f);
        QVERIFY(qa != qb);
        QVERIFY(qFuzzyCompare(qa, qb));
        a = qa;
        b = qb;
        QVERIFY(a != b);
        b = -qb;
        QVERIFY(a != b);
    }
}

QTEST_APPLESS_MAIN(tst_RotationDataClass)

#include "tst_rotation.moc"
