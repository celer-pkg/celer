// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <qnearfieldtarget_emulator_p.h>
#include <qnearfieldtagtype2_p.h>
#include <QtNfc/qndefmessage.h>
#include <QtNfc/qndefnfctextrecord.h>

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QNearFieldTarget*)

static const char * const deadbeef = "\xde\xad\xbe\xef";

class tst_QNearFieldTagType2 : public QObject
{
    Q_OBJECT

public:
    tst_QNearFieldTagType2();

private slots:
    void init();
    void cleanup();

    void staticMemoryModel();
    void dynamicMemoryModel();

    void ndefMessages();

private:
    void waitForMatchingTarget();

    QObject *targetParent;
    QNearFieldTagType2 *target;
};

tst_QNearFieldTagType2::tst_QNearFieldTagType2()
:   targetParent(0), target(0)
{
    QDir::setCurrent(QLatin1String(SRCDIR));

    qRegisterMetaType<QNdefMessage>();
    qRegisterMetaType<TagBase *>();
    qRegisterMetaType<QNearFieldTarget *>();
}

void tst_QNearFieldTagType2::init()
{
    targetParent = new QObject();

    TagActivator::instance()->initialize();
    TagActivator::instance()->start();
}

void tst_QNearFieldTagType2::cleanup()
{
    TagActivator::instance()->reset();

    delete targetParent;
    targetParent = 0;
    target = 0;
}

void tst_QNearFieldTagType2::waitForMatchingTarget()
{
    TagActivator *activator = TagActivator::instance();
    QSignalSpy targetDetectedSpy(activator, SIGNAL(tagActivated(TagBase*)));

    QTRY_VERIFY(!targetDetectedSpy.isEmpty());

    target = new TagType2(targetDetectedSpy.first().at(0).value<TagBase *>(), targetParent);

    QVERIFY(target);

    QCOMPARE(target->type(), QNearFieldTarget::NfcTagType2);
}

void tst_QNearFieldTagType2::staticMemoryModel()
{
    waitForMatchingTarget();
    if (QTest::currentTestFailed())
        return;

    QVERIFY(target->accessMethods() & QNearFieldTarget::TagTypeSpecificAccess);

    QCOMPARE(target->version(), quint8(0x10));

    // readBlock(), writeBlock()
    {
        for (int i = 0; i < 2; ++i) {
            QNearFieldTarget::RequestId id = target->readBlock(i);
            QVERIFY(target->waitForRequestCompleted(id));

            const QByteArray block = target->requestResponse(id).toByteArray();

            id = target->writeBlock(i, QByteArray(4, 0x55));
            QVERIFY(target->waitForRequestCompleted(id));
            QVERIFY(!target->requestResponse(id).toBool());

            id = target->readBlock(i);
            QVERIFY(target->waitForRequestCompleted(id));

            const QByteArray readBlock = target->requestResponse(id).toByteArray();
            QCOMPARE(readBlock, block);
        }

        for (int i = 3; i < 16; ++i) {
            // Read initial data
            QNearFieldTarget::RequestId id = target->readBlock(i);
            QVERIFY(target->waitForRequestCompleted(id));
            QByteArray initialBlock = target->requestResponse(id).toByteArray();

            // Write 0x55
            id = target->writeBlock(i, QByteArray(4, 0x55));
            QVERIFY(target->waitForRequestCompleted(id));
            QVERIFY(target->requestResponse(id).toBool());

            id = target->readBlock(i);
            QVERIFY(target->waitForRequestCompleted(id));
            QByteArray readBlock = target->requestResponse(id).toByteArray();
            QCOMPARE(readBlock, QByteArray(4, 0x55) + initialBlock.mid(4));

            // Write 0xaa
            id = target->writeBlock(i, QByteArray(4, char(0xaa)));
            QVERIFY(target->waitForRequestCompleted(id));
            QVERIFY(target->requestResponse(id).toBool());

            id = target->readBlock(i);
            QVERIFY(target->waitForRequestCompleted(id));
            readBlock = target->requestResponse(id).toByteArray();
            QCOMPARE(readBlock, QByteArray(4, char(0xaa)) + initialBlock.mid(4));
        }
    }
}

void tst_QNearFieldTagType2::dynamicMemoryModel()
{
    bool testedStatic = false;
    bool testedDynamic = false;

    QList<QByteArray> seenIds;
    forever {
        waitForMatchingTarget();
        if (QTest::currentTestFailed())
            return;

        QVERIFY(target->accessMethods() & QNearFieldTarget::TagTypeSpecificAccess);

        QNearFieldTarget::RequestId id = target->readBlock(0);
        QVERIFY(target->waitForRequestCompleted(id));

        const QByteArray data = target->requestResponse(id).toByteArray();
        const QByteArray uid = data.left(3) + data.mid(4, 4);

        if (seenIds.contains(uid))
            break;
        else
            seenIds.append(uid);

        QCOMPARE(target->version(), quint8(0x10));

        bool dynamic = target->memorySize() > 1024;

        if (dynamic) {
            testedDynamic = true;

            int totalBlocks = target->memorySize() / 4;
            int sector1Blocks = qMin(totalBlocks - 256, 256);

            // default sector is sector 0
            for (int i = 3; i < 256; ++i) {
                // Write 0x55
                QNearFieldTarget::RequestId id = target->writeBlock(i, deadbeef);
                QVERIFY(target->waitForRequestCompleted(id));
                QVERIFY(target->requestResponse(id).toBool());
            }

            // change to sector 1
            {
                QNearFieldTarget::RequestId id = target->selectSector(1);
                QVERIFY(target->waitForRequestCompleted(id));
                QVERIFY(target->requestResponse(id).toBool());
            }

            for (int i = 0; i < sector1Blocks; ++i) {
                QNearFieldTarget::RequestId id = target->readBlock(i);
                QVERIFY(target->waitForRequestCompleted(id));
                QByteArray initialBlock = target->requestResponse(id).toByteArray();

                QVERIFY(initialBlock.left(4) != deadbeef);

                // Write 0x55
                id = target->writeBlock(i, QByteArray(4, 0x55));
                QVERIFY(target->waitForRequestCompleted(id));
                QVERIFY(target->requestResponse(id).toBool());

                id = target->readBlock(i);
                QVERIFY(target->waitForRequestCompleted(id));
                QByteArray readBlock = target->requestResponse(id).toByteArray();
                QCOMPARE(readBlock, QByteArray(4, 0x55) + initialBlock.mid(4));

                // Write 0xaa
                id = target->writeBlock(i, QByteArray(4, char(0xaa)));
                QVERIFY(target->waitForRequestCompleted(id));
                QVERIFY(target->requestResponse(id).toBool());

                id = target->readBlock(i);
                QVERIFY(target->waitForRequestCompleted(id));
                readBlock = target->requestResponse(id).toByteArray();
                QCOMPARE(readBlock, QByteArray(4, char(0xaa)) + initialBlock.mid(4));
            }

            // change to sector 0
            {
                QNearFieldTarget::RequestId id = target->selectSector(0);
                QVERIFY(target->waitForRequestCompleted(id));
                QVERIFY(target->requestResponse(id).toBool());
            }

            for (int i = 3; i < 256; ++i) {
                QNearFieldTarget::RequestId id = target->readBlock(i);
                QVERIFY(target->waitForRequestCompleted(id));
                QByteArray readBlock = target->requestResponse(id).toByteArray();

                QVERIFY(readBlock.left(4) == deadbeef);
            }
        } else {
            testedStatic = true;

            QNearFieldTarget::RequestId id = target->selectSector(1);
            QVERIFY(target->waitForRequestCompleted(id));
            QVERIFY(!target->requestResponse(id).toBool());
        }
    }

    QVERIFY(testedStatic);
    QVERIFY(testedDynamic);
}

void tst_QNearFieldTagType2::ndefMessages()
{
    QSKIP("Not implemented");

    QByteArray firstId;
    forever {
        waitForMatchingTarget();
        if (QTest::currentTestFailed())
            return;

        QNearFieldTarget::RequestId id = target->readBlock(0);
        QVERIFY(target->waitForRequestCompleted(id));

        QByteArray uid = target->requestResponse(id).toByteArray().left(3);

        id = target->readBlock(1);
        QVERIFY(target->waitForRequestCompleted(id));
        uid.append(target->requestResponse(id).toByteArray());

        if (firstId.isEmpty())
            firstId = uid;
        else if (firstId == uid)
            break;

        QVERIFY(target->hasNdefMessage());

        QSignalSpy ndefMessageReadSpy(target, SIGNAL(ndefMessageRead(QNdefMessage)));

        target->readNdefMessages();

        QTRY_VERIFY(!ndefMessageReadSpy.isEmpty());

        QList<QNdefMessage> ndefMessages;
        for (qsizetype i = 0; i < ndefMessageReadSpy.size(); ++i)
            ndefMessages.append(ndefMessageReadSpy.at(i).first().value<QNdefMessage>());

        QList<QNdefMessage> messages;
        QNdefNfcTextRecord textRecord;
        textRecord.setText(QStringLiteral("tst_QNearFieldTagType2::ndefMessages"));

        QNdefMessage message;
        message.append(textRecord);

        if (target->memorySize() > 120) {
            QNdefRecord record;
            record.setTypeNameFormat(QNdefRecord::ExternalRtd);
            record.setType("org.qt-project:ndefMessagesTest");
            record.setPayload(QByteArray(120, quint8(0x55)));
            message.append(record);
        }

        messages.append(message);

        QSignalSpy requestCompleteSpy(target, &QNearFieldTagType2::requestCompleted);
        id = target->writeNdefMessages(messages);

        QTRY_VERIFY(!requestCompleteSpy.isEmpty());
        const auto completedId =
                requestCompleteSpy.takeFirst().first().value<QNearFieldTarget::RequestId>();
        QCOMPARE(completedId, id);

        QVERIFY(target->hasNdefMessage());

        ndefMessageReadSpy.clear();

        target->readNdefMessages();

        QTRY_VERIFY(!ndefMessageReadSpy.isEmpty());

        QList<QNdefMessage> storedMessages;
        for (qsizetype i = 0; i < ndefMessageReadSpy.size(); ++i)
            storedMessages.append(ndefMessageReadSpy.at(i).first().value<QNdefMessage>());

        QVERIFY(ndefMessages != storedMessages);

        QVERIFY(messages == storedMessages);
    }
}

QTEST_MAIN(tst_QNearFieldTagType2)

// Unset the moc namespace which is not required for the following include.
#undef QT_BEGIN_MOC_NAMESPACE
#define QT_BEGIN_MOC_NAMESPACE
#undef QT_END_MOC_NAMESPACE
#define QT_END_MOC_NAMESPACE

#include "tst_qnearfieldtagtype2.moc"
