// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/private/qrandomaccessasyncfile_p.h>

#include <QtCore/qrandom.h>
#include <QtCore/qtemporaryfile.h>

#include <QtTest/qsignalspy.h>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;

template <typename T>
static bool spanIsEqualToByteArray(QSpan<T> lhs, QByteArrayView rhs) noexcept
{
    const auto leftBytes = as_bytes(lhs);
    const auto rightBytes = as_bytes(QSpan{rhs});
    return std::equal(leftBytes.begin(), leftBytes.end(),
                      rightBytes.begin(), rightBytes.end());
}

class tst_QRandomAccessAsyncFile : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void size();
    void roundtripOwning();
    void roundtripNonOwning();
    void roundtripVectored();
    void readLessThanMax();
    void flushIsBarrier();
    void errorHandling_data();
    void errorHandling();
    void fileClosedInProgress_data();
    void fileClosedInProgress();
    void fileRemovedInProgress_data();
    void fileRemovedInProgress();
    void operationsDeletedInProgress_data();
    void operationsDeletedInProgress();

    void readWriteNoBuffers_data();
    void readWriteNoBuffers();

    void asyncOpenErrors();
    void closeCornerCases();

private:
    enum class ReadWriteOp : quint8
    {
        OwningRead,
        OwningWrite,
        SingleSpanRead,
        SingleSpanWrite,
        MultiSpanRead,
        MultiSpanWrite,
    };

    enum class Ownership : quint8
    {
        Owning,
        NonOwning,
    };
    void generateOperationColumns();

    // Write 100 Mb of random data to the file.
    // We use such a large amount, because some of the backends will report
    // the progress of async operations in chunks, and we want to test it.
    static constexpr qint64 FileSize = 100 * 1024 * 1024;
    QTemporaryFile m_file;
};

void tst_QRandomAccessAsyncFile::initTestCase()
{
    QVERIFY(m_file.open());

    QByteArray data(FileSize, Qt::Uninitialized);
    auto *ptr = data.data();
    for (qsizetype i = 0; i < FileSize; ++i)
        ptr[i] = char(i % 256);

    qint64 written = m_file.write(data);
    QCOMPARE_EQ(written, FileSize);
}

void tst_QRandomAccessAsyncFile::cleanupTestCase()
{
    m_file.close();
    using namespace std::chrono_literals;
    QDeadlineTimer dt(10s);
    // Loop a little bit in case there is an access race on Windows:
    bool success = false;
    while (!(success = m_file.remove()) && !dt.hasExpired())
        QThread::msleep(100);
    QVERIFY2(success, qPrintable(m_file.errorString()));
}

void tst_QRandomAccessAsyncFile::size()
{
    QRandomAccessAsyncFile file;

    // File not opened -> size unknown
    QCOMPARE_EQ(file.size(), -1);

    QIOOperation *openOp = file.open(m_file.fileName(), QIODeviceBase::ReadOnly);
    QTRY_COMPARE(openOp->isFinished(), true);
    QCOMPARE_EQ(openOp->error(), QIOOperation::Error::None);

    QCOMPARE(file.size(), FileSize);
}

void tst_QRandomAccessAsyncFile::roundtripOwning()
{
    QRandomAccessAsyncFile file;
    QIOOperation *openOp = file.open(m_file.fileName(), QIODeviceBase::ReadWrite);
    QTRY_COMPARE(openOp->isFinished(), true);
    QCOMPARE_EQ(openOp->error(), QIOOperation::Error::None);

    // All operations will be deleted together with the file

    // Write some data into the file

    const qsizetype offset1 = 1024 * 1024;
    const qsizetype size1 = 10 * 1024 * 1024;

    // Testing const ref overload
    const QByteArray dataToWrite(size1, 'a');
    QIOWriteOperation *write1 = file.write(offset1, dataToWrite);
    QSignalSpy write1Spy(write1, &QIOOperation::finished);
    QSignalSpy write1ErrorSpy(write1, &QIOOperation::errorOccurred);

    const qsizetype offset2 = 20 * 1024 * 1024;
    const qsizetype size2 = 5 * 1024 * 1024;

    // Testing rvalue overload
    QIOWriteOperation *write2 = file.write(offset2, QByteArray(size2, 'b'));
    QSignalSpy write2Spy(write2, &QIOOperation::finished);
    QSignalSpy write2ErrorSpy(write2, &QIOOperation::errorOccurred);

    QTRY_COMPARE_EQ(write1Spy.size(), 1);
    QCOMPARE_EQ(write1ErrorSpy.size(), 0);
    QCOMPARE_EQ(write1->error(), QIOOperation::Error::None);
    QCOMPARE_EQ(write1->isFinished(), true);
    QCOMPARE_EQ(write1->offset(), offset1);
    QCOMPARE_EQ(write1->numBytesProcessed(), size1);

    QTRY_COMPARE_EQ(write2Spy.size(), 1);
    QCOMPARE_EQ(write2ErrorSpy.size(), 0);
    QCOMPARE_EQ(write2->error(), QIOOperation::Error::None);
    QCOMPARE_EQ(write2->isFinished(), true);
    QCOMPARE_EQ(write2->offset(), offset2);
    QCOMPARE_EQ(write2->numBytesProcessed(), size2);

    // Now read what we have written

    QIOReadOperation *read1 = file.read(offset1, size1);
    QSignalSpy read1Spy(read1, &QIOOperation::finished);
    QSignalSpy read1ErrorSpy(read1, &QIOOperation::errorOccurred);

    QIOReadOperation *read2 = file.read(offset2, size2);
    QSignalSpy read2Spy(read2, &QIOOperation::finished);
    QSignalSpy read2ErrorSpy(read2, &QIOOperation::errorOccurred);

    QTRY_COMPARE_EQ(read1Spy.size(), 1);
    QCOMPARE_EQ(read1ErrorSpy.size(), 0);
    QCOMPARE_EQ(read1->error(), QIOOperation::Error::None);
    QCOMPARE_EQ(read1->isFinished(), true);
    QCOMPARE_EQ(read1->offset(), offset1);
    QCOMPARE_EQ(read1->data(), dataToWrite);

    QTRY_COMPARE_EQ(read2Spy.size(), 1);
    QCOMPARE_EQ(read2ErrorSpy.size(), 0);
    QCOMPARE_EQ(read2->error(), QIOOperation::Error::None);
    QCOMPARE_EQ(read2->isFinished(), true);
    QCOMPARE_EQ(read2->offset(), offset2);
    QCOMPARE_EQ(read2->data(), QByteArray(size2, 'b'));
}

void tst_QRandomAccessAsyncFile::roundtripNonOwning()
{
    QRandomAccessAsyncFile file;
    QIOOperation *openOp = file.open(m_file.fileName(), QIODeviceBase::ReadWrite);
    QTRY_COMPARE(openOp->isFinished(), true);
    QCOMPARE_EQ(openOp->error(), QIOOperation::Error::None);

    // All operations will be deleted together with the file

    // Write some data into the file

    const qsizetype offset1 = 1024 * 1024;
    const qsizetype size1 = 10 * 1024 * 1024;

    // QSpan is an lvalue
    const QByteArray dataToWrite(size1, 'a');
    const QSpan<const std::byte> spanToWrite(as_bytes(QSpan{dataToWrite}));
    QIOVectoredWriteOperation *write1 = file.writeFrom(offset1, spanToWrite);
    QSignalSpy write1Spy(write1, &QIOOperation::finished);
    QSignalSpy write1ErrorSpy(write1, &QIOOperation::errorOccurred);

    const qsizetype offset2 = 20 * 1024 * 1024;
    const qsizetype size2 = 5 * 1024 * 1024;

    // QSpan is an rvalue
    const QByteArray otherDataToWrite(size2, 'b');
    QIOVectoredWriteOperation *write2 =
            file.writeFrom(offset2, as_bytes(QSpan{otherDataToWrite}));
    QSignalSpy write2Spy(write2, &QIOOperation::finished);
    QSignalSpy write2ErrorSpy(write2, &QIOOperation::errorOccurred);

    QTRY_COMPARE_EQ(write1Spy.size(), 1);
    QCOMPARE_EQ(write1ErrorSpy.size(), 0);
    QCOMPARE_EQ(write1->error(), QIOOperation::Error::None);
    QCOMPARE_EQ(write1->isFinished(), true);
    QCOMPARE_EQ(write1->offset(), offset1);
    QCOMPARE_EQ(write1->numBytesProcessed(), size1);

    QTRY_COMPARE_EQ(write2Spy.size(), 1);
    QCOMPARE_EQ(write2ErrorSpy.size(), 0);
    QCOMPARE_EQ(write2->error(), QIOOperation::Error::None);
    QCOMPARE_EQ(write2->isFinished(), true);
    QCOMPARE_EQ(write2->offset(), offset2);
    QCOMPARE_EQ(write2->numBytesProcessed(), size2);

    // Now read what we have written

    // QSpan is an lvalue
    QByteArray buffer1(size1, Qt::Uninitialized);
    QSpan<std::byte> spanToRead = as_writable_bytes(QSpan{buffer1});
    QIOVectoredReadOperation *read1 = file.readInto(offset1, spanToRead);
    QSignalSpy read1Spy(read1, &QIOOperation::finished);
    QSignalSpy read1ErrorSpy(read1, &QIOOperation::errorOccurred);

    // QSpan is an rvalue
    QByteArray buffer2(size2, Qt::Uninitialized);
    QIOVectoredReadOperation *read2 =
            file.readInto(offset2, as_writable_bytes(QSpan{buffer2}));
    QSignalSpy read2Spy(read2, &QIOOperation::finished);
    QSignalSpy read2ErrorSpy(read2, &QIOOperation::errorOccurred);

    QTRY_COMPARE_EQ(read1Spy.size(), 1);
    QCOMPARE_EQ(read1ErrorSpy.size(), 0);
    QCOMPARE_EQ(read1->error(), QIOOperation::Error::None);
    QCOMPARE_EQ(read1->isFinished(), true);
    QCOMPARE_EQ(read1->offset(), offset1);
    QVERIFY(spanIsEqualToByteArray(read1->data().front(), dataToWrite));

    QTRY_COMPARE_EQ(read2Spy.size(), 1);
    QCOMPARE_EQ(read2ErrorSpy.size(), 0);
    QCOMPARE_EQ(read2->error(), QIOOperation::Error::None);
    QCOMPARE_EQ(read2->isFinished(), true);
    QCOMPARE_EQ(read2->offset(), offset2);
    QVERIFY(spanIsEqualToByteArray(read2->data().front(), otherDataToWrite));
}

void tst_QRandomAccessAsyncFile::roundtripVectored()
{
    QRandomAccessAsyncFile file;
    QIOOperation *openOp = file.open(m_file.fileName(), QIODeviceBase::ReadWrite);
    QTRY_COMPARE(openOp->isFinished(), true);
    QCOMPARE_EQ(openOp->error(), QIOOperation::Error::None);

    // All operations will be deleted together with the file

    // Write some data into the file

    const qsizetype offset = 1024 * 1024;
    const qsizetype size1 = 10 * 1024 * 1024;
    const QByteArray dataToWrite(size1, 'a');

    const qsizetype size2 = 5 * 1024 * 1024;
    const QByteArray otherDataToWrite(size2, 'b');

    // vectored write
    QIOVectoredWriteOperation *write =
            file.writeFrom(offset, { as_bytes(QSpan{dataToWrite}),
                                     as_bytes(QSpan{otherDataToWrite}) });
    QSignalSpy writeSpy(write, &QIOOperation::finished);
    QSignalSpy writeErrorSpy(write, &QIOOperation::errorOccurred);

    QTRY_COMPARE_EQ(writeSpy.size(), 1);
    QCOMPARE_EQ(writeErrorSpy.size(), 0);
    QCOMPARE_EQ(write->error(), QIOOperation::Error::None);
    QCOMPARE_EQ(write->isFinished(), true);
    QCOMPARE_EQ(write->offset(), offset);
    QCOMPARE_EQ(write->numBytesProcessed(), size1 + size2);

    // Now read what we have written

    QByteArray buffer1(size1, Qt::Uninitialized);
    QByteArray buffer2(size2, Qt::Uninitialized);

    QIOVectoredReadOperation *read =
            file.readInto(offset, { as_writable_bytes(QSpan{buffer1}),
                                    as_writable_bytes(QSpan{buffer2}) });
    QSignalSpy readSpy(read, &QIOOperation::finished);
    QSignalSpy readErrorSpy(read, &QIOOperation::errorOccurred);

    QTRY_COMPARE_EQ(readSpy.size(), 1);
    QCOMPARE_EQ(readErrorSpy.size(), 0);
    QCOMPARE_EQ(read->error(), QIOOperation::Error::None);
    QCOMPARE_EQ(read->isFinished(), true);
    QCOMPARE_EQ(read->offset(), offset);

    std::array<const QByteArray*, 2> expectedResults = {&dataToWrite, &otherDataToWrite};

    const auto buffers = read->data();
    QCOMPARE_EQ(size_t(buffers.size()), expectedResults.size());
    for (size_t i = 0; i < expectedResults.size(); ++i)
        QVERIFY(spanIsEqualToByteArray(buffers[i], *expectedResults[i]));
}

void tst_QRandomAccessAsyncFile::readLessThanMax()
{
    QRandomAccessAsyncFile file;
    QIOOperation *openOp = file.open(m_file.fileName(), QIODeviceBase::ReadOnly);
    QTRY_COMPARE(openOp->isFinished(), true);
    QCOMPARE_EQ(openOp->error(), QIOOperation::Error::None);

    constexpr qint64 offsetFromEnd = 100;

    // owning
    {
        QIOReadOperation *op = file.read(FileSize - offsetFromEnd, 1024);
        QSignalSpy spy(op, &QIOOperation::finished);

        QTRY_COMPARE_EQ(spy.size(), 1);
        QCOMPARE_EQ(op->error(), QIOOperation::Error::None);
        QCOMPARE_EQ(op->numBytesProcessed(), offsetFromEnd);
        // we only read what we could
        QCOMPARE_EQ(op->data().size(), offsetFromEnd);
    }

    // non-owning single buffer
    {
        QByteArray buffer(1024, Qt::Uninitialized);
        QIOVectoredReadOperation *op =
                file.readInto(FileSize - offsetFromEnd, as_writable_bytes(QSpan{buffer}));
        QSignalSpy spy(op, &QIOOperation::finished);

        QTRY_COMPARE_EQ(spy.size(), 1);
        QCOMPARE_EQ(op->error(), QIOOperation::Error::None);
        QCOMPARE_EQ(op->numBytesProcessed(), offsetFromEnd);
        // we only read what we could
        QCOMPARE_EQ(op->data().front().size(), offsetFromEnd);
    }

    // non-owning vectored read
    {
        constexpr qsizetype size1 = 50;
        constexpr qsizetype size2 = 150;
        constexpr qsizetype size3 = size2;

        QByteArray buffer1(50, Qt::Uninitialized);
        QByteArray buffer2(size2, Qt::Uninitialized);
        QByteArray buffer3(size3, Qt::Uninitialized);

        std::array<QSpan<std::byte>, 3> buffers{ as_writable_bytes(QSpan{buffer1}),
                                                 as_writable_bytes(QSpan{buffer2}),
                                                 as_writable_bytes(QSpan{buffer3}) };

        QIOVectoredReadOperation *op =
                file.readInto(FileSize - offsetFromEnd, buffers);
        QSignalSpy spy(op, &QIOOperation::finished);
        QTRY_COMPARE_EQ(spy.size(), 1);
        QCOMPARE_EQ(op->error(), QIOOperation::Error::None);
        QCOMPARE_EQ(op->numBytesProcessed(), offsetFromEnd);

        const auto results = op->data();
        QCOMPARE_EQ(size_t(results.size()), buffers.size());

        // first buffer should be fully populated
        QCOMPARE_EQ(results[0].size(), size1);

        // second buffer should only be partially populated
        constexpr qsizetype expectedSize2 = offsetFromEnd - size1;
        QCOMPARE_EQ(results[1].size(), expectedSize2);

        // third buffer should be empty
        QCOMPARE_EQ(results[2].size(), 0);
    }
}

void tst_QRandomAccessAsyncFile::flushIsBarrier()
{
    QRandomAccessAsyncFile file;
    QIOOperation *openOp = file.open(m_file.fileName(), QIODeviceBase::ReadWrite);
    QTRY_COMPARE(openOp->isFinished(), true);
    QCOMPARE_EQ(openOp->error(), QIOOperation::Error::None);

    // All operations will be deleted together with the file

    // Write some data into the file

    const qsizetype offset = 1024 * 1024;
    const qsizetype sizeA = 10 * 1024 * 1024;
    const QByteArray dataToWrite(sizeA, 'a');

    const qsizetype sizeB = 5 * 1024 * 1024;
    const QByteArray otherDataToWrite(sizeB, 'b');

    // This test tries to verify that flush() acts like a barrier.
    // The logic is as follows:
    // 1. submit a write() operation of 10Mb of a's followed by 5Mb of b's.
    // 2. submit a flush().
    // 3. submit another write() of 10 c's that overlap a's and b's.
    // 4. submit another flush().
    // 5. submit a read() of 20 elements from the overlapping region.
    // 6. wait until the read() is completed. If flush() works as expected,
    //    we should get "aaaaaccccccccccbbbbb".

    // First write()
    QIOVectoredWriteOperation *write1 =
            file.writeFrom(offset, { as_bytes(QSpan{dataToWrite}),
                                     as_bytes(QSpan{otherDataToWrite}) });

    // First flush()
    QIOOperation *flush1 = file.flush();

    // Second write()
    const qsizetype offset2 = offset + sizeA - 5;
    const qsizetype sizeC = 10;
    QIOWriteOperation *write2 = file.write(offset2, QByteArray(sizeC, 'c'));

    // Second flush()
    QIOOperation *flush2 = file.flush();

    // Read
    const qsizetype readOffset = offset2 - 5;
    const qsizetype readSize = 20;
    QIOReadOperation *read = file.read(readOffset, readSize);

    QSignalSpy readSpy(read, &QIOOperation::finished);

    // Wait until the read() operation completes
    QTRY_COMPARE_EQ(readSpy.size(), 1);

    // Make sure that all operations have successfully finished.
    QCOMPARE_EQ(write1->isFinished(), true);
    QCOMPARE_EQ(write1->error(), QIOOperation::Error::None);
    QCOMPARE_EQ(write1->numBytesProcessed(), sizeA + sizeB);

    QCOMPARE_EQ(flush1->isFinished(), true);
    QCOMPARE_EQ(flush1->error(), QIOOperation::Error::None);

    QCOMPARE_EQ(write2->isFinished(), true);
    QCOMPARE_EQ(write2->error(), QIOOperation::Error::None);
    QCOMPARE_EQ(write2->numBytesProcessed(), sizeC);

    QCOMPARE_EQ(flush2->isFinished(), true);
    QCOMPARE_EQ(flush2->error(), QIOOperation::Error::None);

    const QByteArray expectedReadResult = "aaaaaccccccccccbbbbb";
    QCOMPARE_EQ(read->isFinished(), true);
    QCOMPARE_EQ(read->error(), QIOOperation::Error::None);
    QCOMPARE_EQ(read->numBytesProcessed(), expectedReadResult.size());
    QCOMPARE_EQ(read->data(), expectedReadResult);
}

void tst_QRandomAccessAsyncFile::errorHandling_data()
{
    QTest::addColumn<QIOOperation::Type>("operation");
    QTest::addColumn<QIODeviceBase::OpenModeFlag>("openMode");
    QTest::addColumn<qint64>("offset");
    QTest::addColumn<QIOOperation::Error>("expectedError");

    QTest::newRow("read_not_open")
            << QIOOperation::Type::Read << QIODeviceBase::ReadWrite
            << qint64(0) << QIOOperation::Error::FileNotOpen;
    QTest::newRow("read_writeonly")
            << QIOOperation::Type::Read << QIODeviceBase::WriteOnly
            << qint64(0)
#ifdef Q_OS_DARWIN
            << QIOOperation::Error::FileNotOpen;
#else
            << QIOOperation::Error::Read;
#endif
    QTest::newRow("read_negative_offset")
            << QIOOperation::Type::Read << QIODeviceBase::ReadOnly
            << qint64(-1) << QIOOperation::Error::IncorrectOffset;
    // lseek() allows it. Other backends might behave differently
    // QTest::newRow("read_past_the_end")
    //         << QIOOperationBase::Type::Read << QIODeviceBase::ReadOnly
    //         << qint64(FileSize + 1) << QIOOperationBase::Error::IncorrectOffset;

    QTest::newRow("write_not_open")
            << QIOOperation::Type::Write << QIODeviceBase::ReadWrite
            << qint64(0) << QIOOperation::Error::FileNotOpen;
    QTest::newRow("write_readonly")
            << QIOOperation::Type::Write << QIODeviceBase::ReadOnly
            << qint64(0)
#ifdef Q_OS_DARWIN
            << QIOOperation::Error::FileNotOpen;
#else
            << QIOOperation::Error::Write;
#endif
    QTest::newRow("write_negative_offset")
            << QIOOperation::Type::Write << QIODeviceBase::WriteOnly
            << qint64(-1) << QIOOperation::Error::IncorrectOffset;
    // lseek() allows it. Other backends might behave differently
    // QTest::newRow("write_past_the_end")
    //         << QIOOperationBase::Type::Write << QIODeviceBase::ReadWrite
    //         << qint64(FileSize + 1) << QIOOperationBase::Error::IncorrectOffset;

    QTest::newRow("flush_not_open")
            << QIOOperation::Type::Flush << QIODeviceBase::ReadWrite
            << qint64(0) << QIOOperation::Error::FileNotOpen;
}

void tst_QRandomAccessAsyncFile::errorHandling()
{
    QFETCH(const QIOOperation::Type, operation);
    QFETCH(const QIODeviceBase::OpenModeFlag, openMode);
    QFETCH(const qint64, offset);
    QFETCH(const QIOOperation::Error, expectedError);

    QRandomAccessAsyncFile file;
    if (expectedError != QIOOperation::Error::FileNotOpen) {
        QIOOperation *openOp = file.open(m_file.fileName(), openMode);
        QTRY_COMPARE(openOp->isFinished(), true);
        QCOMPARE_EQ(openOp->error(), QIOOperation::Error::None);
    }

    QIOOperation *op = nullptr;
    if (operation == QIOOperation::Type::Read)
        op = file.read(offset, 100);
    else if (operation == QIOOperation::Type::Write)
        op = file.write(offset, QByteArray(100, 'c'));
    else if (operation == QIOOperation::Type::Flush)
        op = file.flush();

    QVERIFY(op);

    QSignalSpy finishedSpy(op, &QIOOperation::finished);
    QSignalSpy errorSpy(op, &QIOOperation::errorOccurred);

    // error should always come before finished
    QTRY_COMPARE_EQ(finishedSpy.size(), 1);
    QCOMPARE_EQ(errorSpy.size(), 1);

    QCOMPARE_EQ(errorSpy.at(0).at(0).value<QIOOperation::Error>(), expectedError);
    QCOMPARE_EQ(op->error(), expectedError);
}

void tst_QRandomAccessAsyncFile::fileClosedInProgress_data()
{
    generateOperationColumns();
}

void tst_QRandomAccessAsyncFile::fileClosedInProgress()
{
    QFETCH(const Ownership, ownership);
    QFETCH(const QIOOperation::Type, operation);

    QRandomAccessAsyncFile file;
    if (operation != QIOOperation::Type::Open) {
        QIOOperation *openOp = file.open(m_file.fileName(), QIODeviceBase::ReadWrite);
        QTRY_COMPARE(openOp->isFinished(), true);
        QCOMPARE_EQ(openOp->error(), QIOOperation::Error::None);
    }

    constexpr qint64 OneMb = 1024 * 1024;
    constexpr size_t NumOps = 5;
    std::array<QIOOperation *, NumOps> operations;
    std::array<QByteArray, NumOps> buffers;

    for (size_t i = 0; i < operations.size(); ++i) {
        const qint64 offset = i * OneMb;
        QIOOperation *op = nullptr;
        if (operation == QIOOperation::Type::Read) {
            if (ownership == Ownership::Owning) {
                op = file.read(offset, OneMb);
            } else {
                buffers[i].resizeForOverwrite(OneMb);
                op = file.readInto(offset, as_writable_bytes(QSpan{buffers[i]}));
            }
        } else if (operation == QIOOperation::Type::Write) {
            if (ownership == Ownership::Owning) {
                op = file.write(offset, QByteArray(OneMb, 'd'));
            } else {
                buffers[i] = QByteArray(OneMb, 'd');
                op = file.writeFrom(offset, as_bytes(QSpan{buffers[i]}));
            }
        } else if (operation == QIOOperation::Type::Flush) {
            op = file.flush();
        } else if (operation == QIOOperation::Type::Open) {
            op = file.open(m_file.fileName(), QIODeviceBase::ReadWrite);
        }
        QVERIFY(op);
        operations[i] = op;
    }
    file.close();

    auto isAbortedOrCompleteOrFailedToOpen = [](QIOOperation *op) {
        return op->error() == QIOOperation::Error::Aborted // Aborted
                || op->error() == QIOOperation::Error::Open // Failed, because other op is in progress
                || op->error() == QIOOperation::Error::None; // Completed
    };
    for (auto op : operations) {
        QTRY_VERIFY(op->isFinished());
        QVERIFY2(isAbortedOrCompleteOrFailedToOpen(op),
                 qPrintable("Expected Aborted, Open or None, got %1"_L1.arg(
                         QDebug::toString(op->error()))));
    }
}

void tst_QRandomAccessAsyncFile::fileRemovedInProgress_data()
{
    generateOperationColumns();
}

void tst_QRandomAccessAsyncFile::fileRemovedInProgress()
{
    QFETCH(const Ownership, ownership);
    QFETCH(const QIOOperation::Type, operation);

    constexpr qint64 OneMb = 1024 * 1024;
    constexpr size_t NumOps = 5;
    std::array<QIOOperation *, NumOps> operations;
    std::array<QByteArray, NumOps> buffers;

    {
        QRandomAccessAsyncFile file;
        if (operation != QIOOperation::Type::Open) {
            QIOOperation *openOp = file.open(m_file.fileName(), QIODeviceBase::ReadWrite);
            QTRY_COMPARE(openOp->isFinished(), true);
            QCOMPARE_EQ(openOp->error(), QIOOperation::Error::None);
        }

        for (size_t i = 0; i < operations.size(); ++i) {
            const qint64 offset = i * OneMb;
            QIOOperation *op = nullptr;
            if (operation == QIOOperation::Type::Read) {
                if (ownership == Ownership::Owning) {
                    op = file.read(offset, OneMb);
                } else {
                    buffers[i].resizeForOverwrite(OneMb);
                    op = file.readInto(offset, as_writable_bytes(QSpan{buffers[i]}));
                }
            } else if (operation == QIOOperation::Type::Write) {
                if (ownership == Ownership::Owning) {
                    op = file.write(offset, QByteArray(OneMb, 'd'));
                } else {
                    buffers[i] = QByteArray(OneMb, 'd');
                    op = file.writeFrom(offset, as_bytes(QSpan{buffers[i]}));
                }
            } else if (operation == QIOOperation::Type::Flush) {
                op = file.flush();
            } else if (operation == QIOOperation::Type::Open) {
                op = file.open(m_file.fileName(), QIODeviceBase::ReadWrite);
            }
            QVERIFY(op);
            operations[i] = op;
        }
    }
    // The file and all operations are removed at this point.
    // We're just checking that nothing crashes.
}

void tst_QRandomAccessAsyncFile::operationsDeletedInProgress_data()
{
    generateOperationColumns();
}

void tst_QRandomAccessAsyncFile::operationsDeletedInProgress()
{
    QFETCH(const Ownership, ownership);
    QFETCH(const QIOOperation::Type, operation);

    QRandomAccessAsyncFile file;
    if (operation != QIOOperation::Type::Open) {
        QIOOperation *openOp = file.open(m_file.fileName(), QIODeviceBase::ReadWrite);
        QTRY_COMPARE(openOp->isFinished(), true);
        QCOMPARE_EQ(openOp->error(), QIOOperation::Error::None);
    }

    constexpr qint64 OneMb = 1024 * 1024;
    constexpr size_t NumOps = 5;
    std::array<QIOOperation *, NumOps> operations;
    std::array<QByteArray, NumOps> buffers;

    for (size_t i = 0; i < operations.size(); ++i) {
        const qint64 offset = i * OneMb;
        QIOOperation *op = nullptr;
        if (operation == QIOOperation::Type::Read) {
            if (ownership == Ownership::Owning) {
                op = file.read(offset, OneMb);
            } else {
                buffers[i].resizeForOverwrite(OneMb);
                op = file.readInto(offset, as_writable_bytes(QSpan{buffers[i]}));
            }
        } else if (operation == QIOOperation::Type::Write) {
            if (ownership == Ownership::Owning) {
                op = file.write(offset, QByteArray(OneMb, 'd'));
            } else {
                buffers[i] = QByteArray(OneMb, 'd');
                op = file.writeFrom(offset, as_bytes(QSpan{buffers[i]}));
            }
        } else if (operation == QIOOperation::Type::Flush) {
            op = file.flush();
        } else if (operation == QIOOperation::Type::Open) {
            op = file.open(m_file.fileName(), QIODeviceBase::ReadWrite);
        }
        QVERIFY(op);
        operations[i] = op;
    }

    // Make sure some operation is started
    QCoreApplication::processEvents();

    // Delete all operations. We simply make sure that nothing crashes.
    for (auto op : operations)
        delete op;
}

void tst_QRandomAccessAsyncFile::generateOperationColumns()
{
    QTest::addColumn<Ownership>("ownership");
    QTest::addColumn<QIOOperation::Type>("operation");

    constexpr struct OwnershipInfo {
        Ownership own;
        const char name[10];
    } values[] = {
        { Ownership::Owning, "owning" },
        { Ownership::NonOwning, "nonowning" }
    };

    for (const auto &v : values) {
        QTest::addRow("read_%s", v.name) << v.own << QIOOperation::Type::Read;
        QTest::addRow("write_%s", v.name) << v.own << QIOOperation::Type::Write;
    }
    QTest::newRow("flush") << Ownership::NonOwning /* ignored */ << QIOOperation::Type::Flush;
    QTest::newRow("open") << Ownership::NonOwning /* ignored */ << QIOOperation::Type::Open;
}

void tst_QRandomAccessAsyncFile::readWriteNoBuffers_data()
{
    QTest::addColumn<ReadWriteOp>("op");
    QTest::addColumn<qint64>("maxSize"); // for owning read only

    QTest::newRow("OwningRead_zero") << ReadWriteOp::OwningRead << 0LL;
    QTest::newRow("OwningRead_negative") << ReadWriteOp::OwningRead << -1LL;
    QTest::newRow("OwningWrite") << ReadWriteOp::OwningWrite << 0LL;
    QTest::newRow("SingleSpanRead") << ReadWriteOp::SingleSpanRead << 0LL;
    QTest::newRow("SingleSpanWrite") << ReadWriteOp::SingleSpanWrite << 0LL;
    QTest::newRow("MultiSpanRead") << ReadWriteOp::MultiSpanRead << 0LL;
    QTest::newRow("MultiSpanWrite") << ReadWriteOp::MultiSpanWrite << 0LL;
}

void tst_QRandomAccessAsyncFile::readWriteNoBuffers()
{
    QFETCH(const ReadWriteOp, op);
    QFETCH(const qint64, maxSize); // for OwningRead only

    QRandomAccessAsyncFile file;
    QIOOperation *openOp = file.open(m_file.fileName(), QIODeviceBase::ReadWrite);
    QTRY_COMPARE(openOp->isFinished(), true);
    QCOMPARE_EQ(openOp->error(), QIOOperation::Error::None);

    constexpr qint64 offset = 1024 * 1024;
    QByteArray emptyBuffer;
    QIOReadWriteOperationBase *opBase = nullptr;
    switch (op) {
    case ReadWriteOp::OwningRead:
        if (maxSize < 0) {
            QTest::ignoreMessage(QtWarningMsg,
                                 "Using a negative maxSize in QRandomAccessAsyncFile::read() "
                                 "is incorrect. Resetting to zero!");
        }
        opBase = file.read(offset, maxSize);
        break;
    case ReadWriteOp::OwningWrite:
        opBase = file.write(offset, emptyBuffer);
        break;
    case ReadWriteOp::SingleSpanRead:
        opBase = file.readInto(offset, as_writable_bytes(QSpan(emptyBuffer)));
        break;
    case ReadWriteOp::SingleSpanWrite:
        opBase = file.writeFrom(offset, as_bytes(QSpan(emptyBuffer)));
        break;
    case ReadWriteOp::MultiSpanRead:
        opBase = file.readInto(offset, { as_writable_bytes(QSpan(emptyBuffer)),
                                         as_writable_bytes(QSpan(emptyBuffer)) });
        break;
    case ReadWriteOp::MultiSpanWrite:
        opBase = file.writeFrom(offset, { as_bytes(QSpan(emptyBuffer)),
                                          as_bytes(QSpan(emptyBuffer)) });
        break;
    }
    QVERIFY(opBase);

    QSignalSpy finishedSpy(opBase, &QIOOperation::finished);
    QSignalSpy errorSpy(opBase, &QIOOperation::errorOccurred);

    QTRY_COMPARE_EQ(finishedSpy.size(), 1);
    QCOMPARE_EQ(errorSpy.size(), 0);
    QCOMPARE_EQ(opBase->error(), QIOOperation::Error::None);
    QCOMPARE_EQ(opBase->isFinished(), true);
    QCOMPARE_EQ(opBase->numBytesProcessed(), 0);

    if (op == ReadWriteOp::SingleSpanRead || op == ReadWriteOp::MultiSpanRead) {
        auto *readOp = static_cast<QIOVectoredReadOperation *>(opBase);
        const auto buffers = readOp->data();
        QCOMPARE_GE(buffers.size(), 1);
        for (auto buf : buffers)
            QCOMPARE_EQ(buf.size(), 0);
    } else if (op == ReadWriteOp::OwningRead) {
        auto *readOp = static_cast<QIOReadOperation *>(opBase);
        QCOMPARE_EQ(readOp->data().size(), 0);
    }
}

void tst_QRandomAccessAsyncFile::asyncOpenErrors()
{
    QRandomAccessAsyncFile file;

    // open with incorrect filename
    {
        auto *op = file.open(QString(), QIODeviceBase::ReadWrite);
        QTRY_COMPARE_EQ(op->isFinished(), true);
        QCOMPARE_EQ(op->error(), QIOOperation::Error::Open);
    }

    // second open() fails
    {
        auto *op1 = file.open(m_file.fileName(), QIODeviceBase::ReadOnly);
        auto *op2 = file.open(m_file.fileName(), QIODeviceBase::WriteOnly);

        QTRY_COMPARE_EQ(op1->isFinished(), true);
        QCOMPARE_EQ(op1->error(), QIOOperation::Error::None);

        QTRY_COMPARE_EQ(op2->isFinished(), true);
        QCOMPARE_EQ(op2->error(), QIOOperation::Error::Open);
    }
}

void tst_QRandomAccessAsyncFile::closeCornerCases()
{
    // close() without open does not hang
    {
        QRandomAccessAsyncFile file;
        file.close();
    }

    // re-open after close() works
    {
        QRandomAccessAsyncFile file;
        auto *op = file.open(m_file.fileName(), QIODeviceBase::ReadWrite);
        QTRY_COMPARE_EQ(op->isFinished(), true);
        QCOMPARE_EQ(op->error(), QIOOperation::Error::None);

        file.close();

        op = file.open(m_file.fileName(), QIODeviceBase::ReadWrite);
        QTRY_COMPARE_EQ(op->isFinished(), true);
        QCOMPARE_EQ(op->error(), QIOOperation::Error::None);
    }
}

QTEST_MAIN(tst_QRandomAccessAsyncFile)

#include "tst_qrandomaccessasyncfile.moc"
