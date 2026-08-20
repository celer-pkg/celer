// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include "testserverlib.h"
#include <testserver/testserverlib.h>
#include "testserver/testserver.h"
#include "../conversion/comutil_p.h"
#include <QtAxBase/private/qbstr_p.h>
#include <QtCore/private/qcomptr_p.h>

class tst_qaxobjectcom : public QObject
{
    Q_OBJECT
private slots:
    void comObject_receivesVARIANT_whenCalledWithQVariant_data()
    {
        QTest::addColumn<QVariant>("value");
        QTest::addColumn<ComVariant>("expected");

        QTest::addRow("bool") << QVariant{ true } << ComVariant{ VARIANT_TRUE };
        QTest::addRow("short") << QVariant{ static_cast<short>(3) }
                               << ComVariant{ static_cast<int>(3) }; // TODO: Fixme
        QTest::addRow("unsigned short") << QVariant{ static_cast<ushort>(3) }
                                        << ComVariant{ static_cast<int>(3) }; // TODO: Fixme
        QTest::addRow("int") << QVariant{ 3 } << ComVariant{ 3 };
        QTest::addRow("unsigned int") << QVariant{ 3u } << ComVariant{ 3u };
        QTest::addRow("long long") << QVariant{ 3ll } << ComVariant{ 3ll };
        QTest::addRow("unsigned long long") << QVariant{ 3ull } << ComVariant{ 3ull };
        QTest::addRow("float") << QVariant{ 3.3f } << ComVariant{ 3.3f };
        QTest::addRow("double") << QVariant{ 3.3 } << ComVariant{ 3.3 };
    }

    void comObject_receivesVARIANT_whenCalledWithQVariant()
    {
        QFETCH(const QVariant, value);
        QFETCH(const ComVariant, expected);

        // Arrange
        ComServerLib::TestServer server;
        const ComPtr<Receiver> observer = makeComObject<Receiver>();
        server.SetObserver(observer.Get());

        // Act
        server.VariantIn(value);

        // Assert
        QCOMPARE_EQ(observer->lastArg, expected);
    }

    void comObject_receivesVariantContainingBSTRArray_whenCalledWithQVariantStringList()
    {
        // Arrange
        ComServerLib::TestServer server;
        const ComPtr<Receiver> observer = makeComObject<Receiver>();
        server.SetObserver(observer.Get());

        const QVariant value{ QList{ QVariant{ "A" }, QVariant{ "BC" } } };

        // Act
        server.VariantIn(value);

        // Assert
        QVERIFY(V_ISARRAY(&observer->lastArg));

        const SafeArray safeArray{ V_ARRAY(&observer->lastArg) };

        const QSpan<ComVariant> items = *safeArray.data<ComVariant>();

        QCOMPARE_EQ(items[0], ComVariant{ QBStr{ L"A" } });
        QCOMPARE_EQ(items[1], ComVariant{ QBStr{ L"BC" } });
    }

    void comObject_receivesVariantContainingUnsignedCharArray_whenCalledWithQVariantByteArray()
    {
        // Arrange
        ComServerLib::TestServer server;
        const ComPtr<Receiver> observer = makeComObject<Receiver>();
        server.SetObserver(observer.Get());

        QByteArray byteArray{ "Hello world" };
        const QVariant value = QVariant::fromValue(byteArray);

        // Act
        server.VariantIn(value);

        // Assert
        QVERIFY(V_ISARRAY(&observer->lastArg));

        const SafeArray receivedArg{ V_ARRAY(&observer->lastArg) };

        QByteArray data = QByteArray(*receivedArg.data<unsigned char>());

        QCOMPARE_EQ(byteArray, data);
    }
};

QTEST_MAIN(tst_qaxobjectcom)
#include "tst_qaxobjectcom.moc"
