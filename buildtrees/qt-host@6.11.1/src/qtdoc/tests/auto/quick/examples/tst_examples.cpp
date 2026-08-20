// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qtest.h>
#include <QLibraryInfo>
#include <QDir>
#include <QDebug>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickView>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlError>

#include <algorithm>
#include <memory>

using namespace Qt::StringLiterals;

static QtMessageHandler testlibMsgHandler = nullptr;
void msgHandlerFilter(QtMsgType type, const QMessageLogContext &ctxt, const QString &msg)
{
    if (type == QtCriticalMsg || type == QtFatalMsg)
        (*testlibMsgHandler)(type, ctxt, msg);
}

class tst_examples : public QObject
{
    Q_OBJECT
public:
    tst_examples();

private slots:
    void init();
    void cleanup();

    void sgexamples_data();
    void sgexamples();

    void namingConvention();
private:
    QStringList excludedDirs;
    QStringList excludedFiles;

    void namingConvention(const QDir &);
    QStringList findQmlFiles(const QDir &);
    bool isExcluded(const QDir &d) const;

    QQmlEngine engine;
};

tst_examples::tst_examples()
{
    // Add files to exclude here
    excludedFiles << "snippets/qml/listmodel/listmodel.qml"_L1 //Just a ListModel, no root QQuickItem
        << "examples/quick/demos/photosurface/photosurface.qml"_L1; // root item is Window rather than Item

    // Add directories you want excluded here
    excludedDirs << "shared"_L1 // Not an example
        << "snippets/qml/path"_L1 // No root QQuickItem
        << "examples/qml/qmlextensionplugins"_L1 // Requires special import search path
        << "examples/quick/tutorials/gettingStartedQml"_L1; // C++ example, but no cpp files in root dir

    // These snippets are not expected to run on their own.
    excludedDirs << "snippets/qml/visualdatamodel_rootindex"_L1
        << "snippets/qml/qtbinding"_L1
        << "snippets/qml/imports"_L1;
    excludedFiles << "snippets/qml/image-ext.qml"_L1
        << "examples/quick/shapes/content/main.qml"_L1 // relies on resources
        << "examples/quick/shapes/content/interactive.qml"_L1 // relies on resources
        << "examples/demos/addressbook/qml/main.qml"_L1 // relies on resources
        << "examples/demos/robotarm/main.qml"_L1 // relies on custom import
        << "examples/demos/FX_Material_Showroom/main.qml"_L1 // relies on custom import
        << "examples/demos/car-configurator/main.qml"_L1; // relies on custom import

#if !QT_CONFIG(opengl)
    //No support for Particles
    excludedFiles << "examples/qml/dynamicscene/dynamicscene.qml"_L1
        << "examples/quick/animation/basics/color-animation.qml"_L1
        << "examples/quick/particles/affectors/content/age.qml"_L1
        << "examples/quick/touchinteraction/multipointtouch/bearwhack.qml"_L1
        << "examples/quick/touchinteraction/multipointtouch/multiflame.qml"_L1;
    excludedDirs << "examples/quick/particles"_L1;
    // No Support for ShaderEffect
    excludedFiles << "src/quick/doc/snippets/qml/animators.qml"_L1;
#endif

}

void tst_examples::init()
{
    if (!qstrcmp(QTest::currentTestFunction(), "sgsnippets"))
        testlibMsgHandler = qInstallMessageHandler(msgHandlerFilter);
}

void tst_examples::cleanup()
{
    if (!qstrcmp(QTest::currentTestFunction(), "sgsnippets"))
        qInstallMessageHandler(testlibMsgHandler);
}

bool tst_examples::isExcluded(const QDir &d) const
{
    const QString absPath = d.absolutePath();
    return std::any_of(excludedDirs.cbegin(), excludedDirs.cend(),
                       [absPath](const QString &excludedDir) {
                           return absPath.endsWith(excludedDir);
                       });
}

static bool startsWithLower(const QString &file)
{
    return !file.isEmpty() && file.at(0).isLower();
}

/*
This tests that the examples follow the naming convention required
to have them tested by the examples() test.
*/
void tst_examples::namingConvention(const QDir &d)
{
    if (isExcluded(d))
            return;

    const QStringList files = d.entryList({"*.qml"_L1}, QDir::Files);

    bool seenQml = !files.isEmpty();
    const bool seenLowercase = std::any_of(files.cbegin(), files.cend(), startsWithLower);

    if (!seenQml) {
        const QStringList dirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for (const QString &dir : dirs) {
            QDir sub = d;
            sub.cd(dir);
            namingConvention(sub);
        }
    } else if (!seenLowercase) {
        // QTBUG-28271 don't fail, but rather warn only
        qWarning() << QString::fromLatin1(
            "Directory %1 violates naming convention; expected at least one qml file "
            "starting with lower case, got: %2"
        ).arg(d.absolutePath()).arg(files.join(u','));

//        QFAIL(qPrintable(QString(
//            "Directory %1 violates naming convention; expected at least one qml file "
//            "starting with lower case, got: %2"
//        ).arg(d.absolutePath()).arg(files.join(","))));
    }
}

void tst_examples::namingConvention()
{
    const QString examplesPath = QLibraryInfo::path(QLibraryInfo::ExamplesPath);
    for (const auto &examples : {"/qml"_L1, "/quick"_L1}) {
        QDir d(examplesPath + examples);
        if (d.exists())
            namingConvention(d);
    }
}

QStringList tst_examples::findQmlFiles(const QDir &d)
{
    if (isExcluded(d))
        return {};

    QStringList rv;

    const QStringList cppfiles = d.entryList({"*.cpp"_L1}, QDir::Files);
    if (cppfiles.isEmpty()) {
        const QStringList files = d.entryList({"*.qml"_L1}, QDir::Files);
        for (const QString &file : files) {
            if (startsWithLower(file)) {
                const QString absPath = d.absoluteFilePath(file);
                const bool superContinue = std::any_of(excludedFiles.cbegin(), excludedFiles.cend(),
                                                       [absPath](const QString &e) { return absPath.endsWith(e); });
                if (superContinue)
                    continue;
                rv << d.absoluteFilePath(file);
            }
        }
    }


    const QStringList dirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot |
                                         QDir::NoSymLinks);
    for (const QString &dir : dirs) {
        QDir sub = d;
        sub.cd(dir);
        rv << findQmlFiles(sub);
    }

    return rv;
}

/*
This test runs all the examples in the QtQml UI source tree and ensures
that they start and exit cleanly.

Examples are any .qml files under the examples/ directory that start
with a lower case letter.
*/
void tst_examples::sgexamples_data()
{
    QTest::addColumn<QString>("file");

    QString examples = QLatin1StringView(SRCDIR) + "/../../../../examples/"_L1;

    const QStringList files = findQmlFiles(QDir(examples));

    for (const QString &file : files)
        QTest::newRow(qPrintable(file)) << file;
}

void tst_examples::sgexamples()
{
#ifdef Q_OS_ANDROID
    QSKIP("The test needs changes to package examples files to the Android package (QTBUG-103597)");
#endif
    QFETCH(QString, file);
    QQuickWindow window;
    QString title = file;
    if (auto slash = title.lastIndexOf(u'/'); slash != -1)
        title.remove(0, slash + 1);
    window.setTitle(title);
    window.setPersistentGraphics(true);
    window.setPersistentSceneGraph(true);

    QQmlComponent component(&engine, QUrl::fromLocalFile(file));
    if (component.status() == QQmlComponent::Error)
        qWarning() << component.errors();
    QCOMPARE(component.status(), QQmlComponent::Ready);

    std::unique_ptr<QObject> object(component.beginCreate(engine.rootContext()));
    QQuickItem *root = qobject_cast<QQuickItem *>(object.get());
    if (!root)
        component.completeCreate();
    QVERIFY(root);

    window.resize(240, 320);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    root->setParentItem(window.contentItem());
    component.completeCreate();

    QCoreApplication::processEvents();
}

QTEST_MAIN(tst_examples)

#include "tst_examples.moc"
