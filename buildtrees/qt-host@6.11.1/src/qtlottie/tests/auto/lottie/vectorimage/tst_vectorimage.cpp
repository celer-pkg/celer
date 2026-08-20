// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QTest>
#include <QtTest/QSignalSpy>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdiriterator.h>
#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcomponent.h>
#include <QtQuick/qquickitem.h>

#include <QtQuickTestUtils/private/qmlutils_p.h>
#include <QtQuickTestUtils/private/viewtestutils_p.h>
#include <QtQuickTestUtils/private/visualtestutils_p.h>

class tst_VectorImage : public QQmlDataTest
{
    Q_OBJECT
public:
    tst_VectorImage();

private slots:
    void parseFiles_data();
    void parseFiles();
    void parseBrokenFile();
    void parseNoAssumeTrustedSource();
};

tst_VectorImage::tst_VectorImage()
    : QQmlDataTest(QT_QMLTEST_DATADIR)
{
}

void tst_VectorImage::parseFiles_data()
{
    QTest::addColumn<QString>("fileName");

    {
        QDirIterator it(QStringLiteral(":/json"),
                        { QStringLiteral("*.json") });
        QVERIFY(it.hasNext());

        while (it.hasNext()) {
            QFileInfo info(it.next());

            QString fileName = info.fileName();
            QByteArray rowName = fileName.toUtf8();

            QTest::newRow(rowName) << fileName;
        }
    }

    {
        QDirIterator it(QStringLiteral(":/json/precomp"),
                        { QStringLiteral("*.json") });
        QVERIFY(it.hasNext());

        while (it.hasNext()) {
            QFileInfo info(it.next());

            QString fileName = QStringLiteral("precomp/") + info.fileName();
            QByteArray rowName = fileName.toUtf8();

            QTest::newRow(rowName) << fileName;
        }
    }

    {
        QDirIterator it(QStringLiteral(":/json/mod"),
                        { QStringLiteral("*.json") });
        QVERIFY(it.hasNext());

        while (it.hasNext()) {
            QFileInfo info(it.next());

            QString fileName = QStringLiteral("mod/") + info.fileName();
            QByteArray rowName = fileName.toUtf8();

            QTest::newRow(rowName) << fileName;
        }
    }
}

void tst_VectorImage::parseFiles()
{
    QFETCH(QString, fileName);

    QQmlEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("fileName"), QStringLiteral("qrc:/json/%1").arg(fileName));

    QQmlComponent c(&engine, testFileUrl("vectorimage.qml"));
    QQuickItem *item = qobject_cast<QQuickItem *>(c.create());
    auto cleanup = qScopeGuard([&item] {
        delete item;
        item = nullptr;
    });

    QVERIFY(item != nullptr);
    QVERIFY(!item->childItems().isEmpty());
    QVERIFY(!item->childItems().first()->size().isNull());
}

void tst_VectorImage::parseBrokenFile()
{
    QQmlEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("fileName"), testFileUrl("json/broken.json"));

    QQmlComponent c(&engine, testFileUrl("vectorimage.qml"));
    QQuickItem *item = qobject_cast<QQuickItem *>(c.create());
    auto cleanup = qScopeGuard([&item] {
        delete item;
        item = nullptr;
    });

    QVERIFY(item != nullptr);
    QVERIFY(!item->childItems().isEmpty());
    QVERIFY(item->childItems().first()->size().isNull());
}

void tst_VectorImage::parseNoAssumeTrustedSource()
{
    QQmlEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("fileName"), testFileUrl("json/ok.json"));

    {
        QQmlComponent c(&engine, testFileUrl("vectorimage.qml"));
        QQuickItem *item = qobject_cast<QQuickItem *>(c.create());
        auto cleanup = qScopeGuard([&item] {
            delete item;
            item = nullptr;
        });

        QVERIFY(item != nullptr);
        QVERIFY(!item->childItems().isEmpty());
        QVERIFY(!item->childItems().first()->size().isNull());
    }

    {
        QQmlComponent c(&engine, testFileUrl("vectorimage_noassumetrustedsource.qml"));
        QQuickItem *item = qobject_cast<QQuickItem *>(c.create());
        auto cleanup = qScopeGuard([&item] {
            delete item;
            item = nullptr;
        });

        QVERIFY(item != nullptr);
        QVERIFY(!item->childItems().isEmpty());
        QVERIFY(item->childItems().first()->size().isNull());
    }
}

QTEST_MAIN(tst_VectorImage)

#include "tst_vectorimage.moc"
