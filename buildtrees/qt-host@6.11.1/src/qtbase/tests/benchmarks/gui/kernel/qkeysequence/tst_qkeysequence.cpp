// Copyright (C) 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qtest.h>
#include <QtGui/qkeysequence.h>

class tst_QKeySequence : public QObject
{
    Q_OBJECT

private slots:
    void fromString_data();
    void fromString();
};

void tst_QKeySequence::fromString_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<QKeySequence::SequenceFormat>("format");

    QTest::newRow("empty (portable)") << QString() << QKeySequence::PortableText;
    QTest::newRow("empty (native)") << QString() << QKeySequence::NativeText;

    QTest::newRow("invalid (portable)") << QStringLiteral("pizza") << QKeySequence::PortableText;
    QTest::newRow("invalid (native)") << QStringLiteral("pizza") << QKeySequence::NativeText;

    QTest::newRow("F (portable)") << QStringLiteral("F") << QKeySequence::PortableText;
    QTest::newRow("F (native)") << QStringLiteral("F") << QKeySequence::NativeText;

    QTest::newRow("Ctrl+F (portable)") << QStringLiteral("Ctrl+F") << QKeySequence::PortableText;
    QTest::newRow("Ctrl+F (native)") << QStringLiteral("Ctrl+F") << QKeySequence::NativeText;

    QTest::newRow("Meta+Ctrl+Alt+Shift+F (portable)") << QStringLiteral("Meta+Ctrl+Alt+Shift+F") << QKeySequence::PortableText;
    QTest::newRow("Meta+Ctrl+Alt+Shift+F (native)") << QStringLiteral("Meta+Ctrl+Alt+Shift+F") << QKeySequence::NativeText;
}

void tst_QKeySequence::fromString()
{
    QFETCH(QString, text);
    QFETCH(QKeySequence::SequenceFormat, format);

    QBENCHMARK {
        auto sequence = QKeySequence::fromString(text, format);
        Q_UNUSED(sequence)
    }
}

QTEST_MAIN(tst_QKeySequence)
#include "tst_qkeysequence.moc"
