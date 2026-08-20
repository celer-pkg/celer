// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/private/qcomparisontesthelper_p.h>
#include <QtTest/qtest.h>

#include <QtGrpc/qgrpcchanneloptions.h>
#include <QtGrpc/qtgrpcnamespace.h>

#if QT_CONFIG(ssl)
#  include <QtNetwork/qsslconfiguration.h>
#endif

#include <QtCore/qdebug.h>
#include <QtCore/qhash.h>
#include <QtCore/qstring.h>

using namespace std::chrono_literals;

template <typename T>
class GrpcCommonOptionsTest
{
public:
    void hasSpecialMemberFunctions() const
    {
        T o1;
        QVERIFY(!o1.deadlineTimeout());
        QVERIFY(o1.metadata(QtGrpc::MultiValue).empty());

        o1.setDeadlineTimeout(100ms);

        T o2(o1);
        QCOMPARE_EQ(o1.deadlineTimeout(), o2.deadlineTimeout());

        T o3 = o1;
        QCOMPARE_EQ(o1.deadlineTimeout(), o3.deadlineTimeout());

        T o4(std::move(o1));
        QCOMPARE_EQ(o4.deadlineTimeout(), o2.deadlineTimeout());

        o1 = std::move(o4);
        QCOMPARE_EQ(o1.deadlineTimeout(), o2.deadlineTimeout());
    }
    void hasImplicitQVariant() const
    {
        T o1;
        o1.setDeadlineTimeout(250ms);
        o1.setMetadata({
            { "keyA", "valA" },
            { "keyB", "valB" },
        });

        QVariant v = o1;
        QCOMPARE_EQ(v.metaType(), QMetaType::fromType<T>());
        const auto o2 = v.value<T>();
        QCOMPARE_EQ(o1.metadata(QtGrpc::MultiValue), o2.metadata(QtGrpc::MultiValue));
        QCOMPARE_EQ(o1.deadlineTimeout(), o2.deadlineTimeout());
    }
    void hasMemberSwap() const
    {
        constexpr std::chrono::milliseconds Dur = 50ms;

        T o1;
        o1.setDeadlineTimeout(Dur);
        T o2;

        QCOMPARE_EQ(o1.deadlineTimeout(), Dur);
        QVERIFY(!o2.deadlineTimeout());
        o2.swap(o1);
        QCOMPARE_EQ(o2.deadlineTimeout(), Dur);
        QVERIFY(!o1.deadlineTimeout());
        swap(o2, o1);
        QCOMPARE_EQ(o1.deadlineTimeout(), Dur);
        QVERIFY(!o2.deadlineTimeout());
    }

#if QT_DEPRECATED_SINCE(6, 13)
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED

    void deprecatedPropertyMetadata() const
    {
        QHash<QByteArray, QByteArray> data = {
            { "keyA", "valA" },
            { "keyB", "valB" },
        };

        // cref setter
        T o1;
        auto o1Detach = o1;
        o1.setMetadata(data);
        QCOMPARE_EQ(o1.metadata(), data);
        QCOMPARE_NE(o1.metadata(), o1Detach.metadata());

        // rvalue setter
        T o2;
        auto o2Detach = o2;
        auto dataMoved = data;
        o2.setMetadata(std::move(dataMoved));
        QCOMPARE_EQ(o2.metadata(), data);
        QCOMPARE_NE(o2.metadata(), o2Detach.metadata());

        // rvalue-this getter
        T o3;
        o3.setMetadata(data);
        auto movedMd = std::move(o3).metadata();
        QCOMPARE_EQ(movedMd, data);
    }
    void propertyMetadataCompat() const
    {
        auto toMulti = [](const QHash<QByteArray, QByteArray> &m) {
            return QMultiHash<QByteArray, QByteArray>(m);
        };
        QMultiHash<QByteArray, QByteArray> multiMd = {
            { "keyA", "valA1" },
            { "keyA", "valA2" },
            { "keyB", "valB1" },
            { "keyB", "valB2" },
            { "keyC", "valC"  },
        };
        QHash<QByteArray, QByteArray> md = {
            { "keyA", "valA2" },
            { "keyB", "valB2" },
            { "keyC", "valC"  },
        };

        T o1;
        o1.setMetadata(md);

        const auto &mdRef = o1.metadata();
        const auto &multiMdRef = o1.metadata(QtGrpc::MultiValue);

        QCOMPARE_EQ(mdRef, md);
        QCOMPARE_EQ(multiMdRef, toMulti(mdRef));
        QCOMPARE_NE(typeid(mdRef), typeid(multiMdRef));

        // Check that the handed out reference gets updates for QMultiHash setter
        o1.setMetadata(multiMd);
        QCOMPARE_EQ(multiMdRef, multiMd);
        QCOMPARE_EQ(mdRef, md);
        multiMd.insert("keyD", "valD");
        o1.setMetadata(multiMd);
        QCOMPARE_EQ(multiMdRef, multiMd);
        QCOMPARE_NE(mdRef, md);
        md.insert("keyD", "valD");
        QCOMPARE_EQ(mdRef, md);
        o1.setMetadata(md);
        QCOMPARE_EQ(mdRef, md);

        // Check shared state mutation due to lazy evaluation in metadata()
        auto mdCopy = md;
        T o2;
        o2.setMetadata(mdCopy);
        auto o2Detach = o2;
        const auto &o2Md = o2.metadata();
        const auto &o2DetachMd = o2Detach.metadata();
        QCOMPARE_EQ(o2Md, mdCopy);
        QCOMPARE_EQ(o2Md, o2DetachMd);
        mdCopy.insert("keyX", "valX");
        o2.setMetadata(mdCopy); // trigger new merge
        const auto &o2MdAfter = o2.metadata();
        const auto &o2DetachMdAfter = o2Detach.metadata();
        QCOMPARE_NE(o2MdAfter, o2DetachMdAfter);

        T o3A;
        T o3B = o3A;
        const auto &o3aMd = o3A.metadata();
        o3B.setMetadata(QHash<QByteArray, QByteArray>{
            {"keyA","valA"}, {"keyB","valB"}
        });
        // o3aMd is not affected by the update since o3B deprecatedQHashRef is used
        QCOMPARE_NE(o3aMd, o3B.metadata());
        o3A.setMetadata(QHash<QByteArray, QByteArray>{
            {"keyA","valA"}, {"keyB","valB"}, {"keyC","valC"},
        });
        // o3aMd is updated accoringly though
        QCOMPARE_EQ(o3aMd, o3A.metadata());
        QCOMPARE_NE(o3B.metadata(), o3A.metadata());
    }

QT_WARNING_POP
#endif
    void propertyMetadata() const
    {
        std::initializer_list<std::pair<QByteArray, QByteArray>> list = {
            { "keyA", "valA1" },
            { "keyA", "valA2" },
            { "keyB", "valB"  },
        };
        QMultiHash<QByteArray, QByteArray> data(list);

        // cref setter
        T o1;
        auto o1Detach = o1;
        o1.setMetadata(data);
        QCOMPARE_EQ(o1.metadata(QtGrpc::MultiValue), data);
        QCOMPARE_NE(o1.metadata(QtGrpc::MultiValue), o1Detach.metadata(QtGrpc::MultiValue));

        // rvalue setter
        T o2;
        auto o2Detach = o2;
        auto dataMoved = data;
        o2.setMetadata(std::move(dataMoved));
        QCOMPARE_EQ(o2.metadata(QtGrpc::MultiValue), data);
        QCOMPARE_NE(o2.metadata(QtGrpc::MultiValue), o2Detach.metadata(QtGrpc::MultiValue));

        // rvalue-this getter
        T o3;
        o3.setMetadata(data);
        auto movedMd = std::move(o3).metadata(QtGrpc::MultiValue);
        QCOMPARE_EQ(movedMd, data);

        // std::initializer_list setter
        T o4;
        o4.setMetadata(list);
        QCOMPARE_EQ(o4.metadata(QtGrpc::MultiValue), data);

        // addMetadata
        T o5 = T{}.addMetadata("keyA", "valA1").addMetadata("keyA", "valA2");
        auto o5Detach = o5;
        QByteArray k = "keyB", v = "valB";
        o5.addMetadata(k, v);
        // Check that exact key-value pairs are discarded no matter the order.
        o5.addMetadata("keyA", "valA1");
        o5.addMetadata("keyA", "valA2");
        o5.addMetadata("keyB", "valB");
        QCOMPARE_EQ(o5.metadata(QtGrpc::MultiValue), data);
        QCOMPARE_NE(o5.metadata(QtGrpc::MultiValue), o5Detach.metadata(QtGrpc::MultiValue));
    }
    void propertyDeadline() const
    {
        constexpr std::chrono::milliseconds Dur = 50ms;

        T o1;
        auto o1Detach = o1;
        o1.setDeadlineTimeout(Dur);
        QCOMPARE_EQ(o1.deadlineTimeout(), Dur);
        QCOMPARE_NE(o1.deadlineTimeout(), o1Detach.deadlineTimeout());
    }
    void propertyFilterServerMetadata() const
    {
        T o1;
        QCOMPARE_EQ(o1.filterServerMetadata(), std::nullopt);
        auto o1Detach = o1;
        o1.setFilterServerMetadata(true);
        QCOMPARE_NE(o1.filterServerMetadata(), o1Detach.filterServerMetadata());
        QCOMPARE_EQ(o1.filterServerMetadata(), std::optional<bool>(true));
    }
    void streamsToDebug() const
    {
        T o;
        QString storage;
        QDebug dbg(&storage);
        dbg.noquote().nospace();

        dbg << o;
        QVERIFY(!storage.isEmpty());

        std::unique_ptr<char[]> ustr(QTest::toString(o));
        QCOMPARE_EQ(storage, QString::fromUtf8(ustr.get()));
    }
    void comparesEqual() const
    {
        QTestPrivate::testEqualityOperatorsCompile<T>();
        T o1;
        T o2 = o1;

        QT_TEST_EQUALITY_OPS(o1, o2, true);
        auto updateComparisonCheck = [&] {
            QT_TEST_EQUALITY_OPS(o1, o2, false);
            o2 = o1;
            QT_TEST_EQUALITY_OPS(o1, o2, true);
        };

        o1.addMetadata("new", "value");
        updateComparisonCheck();
        o1.setDeadlineTimeout(1s);
        updateComparisonCheck();
        o1.setFilterServerMetadata(true);
        updateComparisonCheck();

        if constexpr (std::is_same_v<T, QGrpcChannelOptions>) {
            o1.setSerializationFormat(QtGrpc::SerializationFormat::Protobuf);
            updateComparisonCheck();
#if QT_CONFIG(ssl)
            o1.setSslConfiguration(QSslConfiguration::defaultConfiguration());
            updateComparisonCheck();
#endif
        }
    }
};
