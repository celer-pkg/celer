// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQmlDom/private/qqmldomtop_p.h>
#include <QtQmlDom/private/qqmldomitem_p.h>

#include <QtTest/QTest>
#include <QtCore/QLibraryInfo>

class tst_qmldomconstruction : public QObject
{
    Q_OBJECT

private slots:
    void domConstructionTime_data();
    void domConstructionTime();
};

void tst_qmldomconstruction::domConstructionTime_data()
{
    using namespace QQmlJS::Dom;
    using namespace Qt::StringLiterals;

    const auto baseDir = QLatin1String(SRCDIR) + QLatin1String("/data");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<DomCreationOption>("creationOption");

    QTest::addRow("default-tiger.qml")
            << baseDir + u"/longQmlFile.qml"_s << DomCreationOption::Default;
    QTest::addRow("default-deeplyNested.qml")
            << baseDir + u"/deeplyNested.qml"_s << DomCreationOption::Default;

    QTest::addRow("extended-tiger.qml") << baseDir + u"/longQmlFile.qml"_s << Extended;
    QTest::addRow("extended-deeplyNested.qml") << baseDir + u"/deeplyNested.qml"_s << Extended;

    QTest::addRow("minimal-tiger.qml") << baseDir + u"/longQmlFile.qml"_s << Minimal;
    QTest::addRow("minimal-deeplyNested.qml") << baseDir + u"/deeplyNested.qml"_s << Minimal;
}

void tst_qmldomconstruction::domConstructionTime()
{
    using namespace QQmlJS::Dom;
    QFETCH(QString, fileName);
    QFETCH(DomCreationOption, creationOption);

    const QStringList importPaths = {
        QLibraryInfo::path(QLibraryInfo::QmlImportsPath),
    };

    DomItem tFile;
    QBENCHMARK {
        auto envPtr = DomEnvironment::create(
                importPaths,
                QQmlJS::Dom::DomEnvironment::Option::SingleThreaded
                        | QQmlJS::Dom::DomEnvironment::Option::NoDependencies,
                creationOption);

        envPtr->loadFile(FileToLoad::fromFileSystem(envPtr, fileName),
                         [&tFile](Path, const DomItem &, const DomItem &newIt) {
                             tFile = newIt.fileObject();
                         });
        envPtr->loadPendingDependencies();

        // make sure the file was loaded
        QCOMPARE(tFile.field(Fields::components).size(), 1);
    }
}

QTEST_MAIN(tst_qmldomconstruction)
#include "tst_qmldomconstruction.moc"
