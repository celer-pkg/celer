// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "interceptor_helper.h"

#include <QtGrpc/qgrpcinterceptorchain.h>

#include <QtTest/qtest.h>

namespace {

const QtGrpc::InterceptorCapabilities
    AllCapabilities = QtGrpc::InterceptorCapabilities(Capability::Start)
    | Capability::InitialMetadata | Capability::MessageReceived | Capability::WriteMessage
    | Capability::WritesDone | Capability::TrailingMetadata | Capability::Finished
    | Capability::Cancel;

}

class QtGrpcInterceptorChainTest : public QObject
{
    Q_OBJECT
public:
    QtGrpcInterceptorChainTest() { QTest::failOnWarning(); }

    template <bool Owning>
    void addInterceptor() const;

    template <bool Owning>
    void setInterceptor() const;

private Q_SLOTS:
    void emptyByDefault() const;
    void addInterceptorOwning() const { addInterceptor<true>(); };
    void addInterceptorNonOwning() const { addInterceptor<false>(); };
    void setInterceptorOwning() const { setInterceptor<true>(); };
    void setInterceptorNonOwning() const { setInterceptor<false>(); };
    void aggregatesCapabilities() const;
    void capabilitiesAt() const;
};

void QtGrpcInterceptorChainTest::emptyByDefault() const
{
    QGrpcInterceptorChain chain;
    QCOMPARE(chain.size(), 0);
    QVERIFY(chain.isEmpty());
    QVERIFY(chain.empty());

    const auto caps = chain.capabilities();
    QCOMPARE_EQ(caps, QtGrpc::InterceptorCapabilities{});
}

template <bool Owning>
void QtGrpcInterceptorChainTest::addInterceptor() const
{
    QGrpcInterceptorChain chain;

    QtGrpc::InterceptorCapabilities
        expectedPartial = QtGrpc::InterceptorCapabilities(Capability::Start) | Capability::Finished;

    auto i1 = std::make_unique<PartialInterceptor>("Partial");
    if constexpr (Owning) {
        QVERIFY(chain.add(std::move(i1)));
        QVERIFY(!i1); // Owning: takes control
    } else {
        QVERIFY(chain.add(i1.get()));
        QVERIFY(i1); // Non-Owning: takes no control
    }
    QCOMPARE_EQ(chain.size(), 1);
    QCOMPARE_EQ(chain.capabilities(), expectedPartial);

    auto i2 = std::make_unique<LoggingInterceptor>("Logging");
    if constexpr (Owning) {
        QVERIFY(chain.add(std::move(i2)));
        QVERIFY(!i2);
    } else {
        QVERIFY(chain.add(i2.get()));
        QVERIFY(i2);
    }
    QCOMPARE_EQ(chain.size(), 2);
    QCOMPARE_EQ(chain.capabilities(), AllCapabilities);

    // Invalid add: null interceptor
    QTest::ignoreMessage(QtWarningMsg, "Cannot add null interceptor");
    auto nullInterceptor = std::unique_ptr<LoggingInterceptor>(nullptr);
    if constexpr (Owning) {
        QVERIFY(!chain.add(std::move(nullInterceptor)));
    } else {
        QVERIFY(!chain.add(nullInterceptor.get()));
    }
    QCOMPARE_EQ(chain.size(), 2); // Chain unchanged

    // Valid add after failure
    auto i3 = std::make_unique<PartialInterceptor>("AfterNull");
    if constexpr (Owning) {
        QVERIFY(chain.add(std::move(i3)));
        QVERIFY(!i3);
    } else {
        QVERIFY(chain.add(i3.get()));
        QVERIFY(i3);
    }
    QCOMPARE_EQ(chain.size(), 3);
}

template <bool Owning>
void QtGrpcInterceptorChainTest::setInterceptor() const
{
    QGrpcInterceptorChain chain;

    auto initial = std::make_unique<LoggingInterceptor>("Initial");
    QVERIFY(chain.add(std::move(initial)));
    QCOMPARE_EQ(chain.size(), 1);

    // set() with null should fail and not modify chain
    auto i1 = std::make_unique<LoggingInterceptor>("I1");
    auto i2 = std::make_unique<LoggingInterceptor>("I2");
    auto nullInterceptor = std::unique_ptr<LoggingInterceptor>(nullptr);

    if constexpr (Owning) {
        QTest::ignoreMessage(QtWarningMsg, "Cannot set null interceptor at index 2 of 2");
        QVERIFY(!chain.set(std::move(i1), std::move(i2), std::move(nullInterceptor)));
        // All interceptors should NOT be moved from (all-or-nothing semantics)
        QVERIFY(i1);
        QVERIFY(i2);
        QVERIFY(!nullInterceptor);
    } else {
        QTest::ignoreMessage(QtWarningMsg, "Cannot set null interceptor at index 1 of 2");
        QVERIFY(!chain.set(i1.get(), nullInterceptor.get(), i2.get()));
        QVERIFY(i1);
        QVERIFY(!nullInterceptor);
        QVERIFY(i2);
    }

    QCOMPARE_EQ(chain.size(), 1); // Chain should be unchanged

    if constexpr (Owning) {
        QVERIFY(chain.set(std::move(i1), std::move(i2)));
        QVERIFY(!i1);
        QVERIFY(!i2);
    } else {
        QVERIFY(chain.set(i1.get(), i2.get()));
        QVERIFY(i1);
        QVERIFY(i2);
    }

    QCOMPARE_EQ(chain.size(), 2);
}

void QtGrpcInterceptorChainTest::aggregatesCapabilities() const
{
    QGrpcInterceptorChain chain;

    QCOMPARE_EQ(chain.capabilities(), QtGrpc::InterceptorCapabilities{});
    QVERIFY(!chain.hasHandlerFor(Capability::Start));
    QVERIFY(!chain.hasHandlerFor(Capability::Finished));

    QVERIFY(chain.add(std::make_unique<PartialInterceptor>("P1")));

    const QtGrpc::InterceptorCapabilities
        partialCaps = QtGrpc::InterceptorCapabilities(Capability::Start) | Capability::Finished;

    QCOMPARE_EQ(chain.capabilities(), partialCaps);
    QVERIFY(chain.hasHandlerFor(Capability::Start));
    QVERIFY(chain.hasHandlerFor(Capability::Finished));
    QVERIFY(!chain.hasHandlerFor(Capability::InitialMetadata));

    class InitialMetaOnly : public QGrpcInitialMetadataInterceptor
    {
    public:
        void onInitialMetadata(QGrpcInterceptionContext &,
                               QMultiHash<QByteArray, QByteArray> &) override
        {
        }
    };

    QVERIFY(chain.add(std::make_unique<InitialMetaOnly>()));

    const QtGrpc::InterceptorCapabilities expectedCaps = partialCaps | Capability::InitialMetadata;

    QCOMPARE_EQ(chain.capabilities(), expectedCaps);

    QVERIFY(chain.hasHandlerFor(Capability::Start));
    QVERIFY(chain.hasHandlerFor(Capability::InitialMetadata));
    QVERIFY(chain.hasHandlerFor(Capability::Finished));
    QVERIFY(!chain.hasHandlerFor(Capability::MessageReceived));

    QVERIFY(chain.add(std::make_unique<LoggingInterceptor>("Logging")));
    QCOMPARE_EQ(chain.capabilities(), AllCapabilities);

    for (size_t i = 0; i < QtGrpcPrivate::InterceptorTypes::size; ++i) {
        auto cap = static_cast<Capability>(1 << i);
        QVERIFY(chain.hasHandlerFor(cap));
    }
}

void QtGrpcInterceptorChainTest::capabilitiesAt() const
{
    QGrpcInterceptorChain chain;

    const QtGrpc::InterceptorCapabilities
        partialCaps = QtGrpc::InterceptorCapabilities(Capability::Start) | Capability::Finished;

    auto i1 = std::make_unique<LoggingInterceptor>("A");
    auto i2 = std::make_unique<PartialInterceptor>("P");
    QVERIFY(chain.add(std::move(i1)));
    QVERIFY(chain.add(std::move(i2)));
    QCOMPARE_EQ(chain.size(), 2);

    // Validate capabilities per index
    QCOMPARE_EQ(chain.capabilitiesAt(0), AllCapabilities);
    QCOMPARE_EQ(chain.capabilitiesAt(1), partialCaps);

    // clear and re-add
    chain.clear();
    QCOMPARE_EQ(chain.size(), 0);

    auto i3 = std::make_unique<PartialInterceptor>("AfterClear");
    QVERIFY(chain.add(std::move(i3)));
    QCOMPARE_EQ(chain.size(), 1);
    QCOMPARE_EQ(chain.capabilitiesAt(0), partialCaps);
    QCOMPARE_EQ(chain.capabilities(), partialCaps);
}

QTEST_MAIN(QtGrpcInterceptorChainTest)

#include "tst_grpc_interceptorchain.moc"
