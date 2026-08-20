// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/qscopedpointer.h>
#include <QTest>
#include <QtQml>
#include <QtQuickControls2/qquickstyle.h>

#include <QtQuickTestUtils/private/viewtestutils_p.h>
#include <QtQuickTestUtils/private/visualtestutils_p.h>
#include <QtQuickControlsTestUtils/private/controlstestutils_p.h>

using namespace QQuickControlsTestUtils;
using namespace QQuickVisualTestUtils;

class tst_CreationTime : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    void basic();
    void basic_data();

    void fusion();
    void fusion_data();

    void imagine();
    void imagine_data();

    void material();
    void material_data();

    void universal();
    void universal_data();

private:
    QQuickStyleHelper styleHelper;
};

void tst_CreationTime::initTestCase()
{
    styleHelper.engine.reset(new QQmlEngine);
}

void tst_CreationTime::init()
{
    styleHelper.engine->clearComponentCache();
}

static void doBenchmark(QQuickStyleHelper &styleHelper, const QUrl &url)
{
    const QString tagStr = QString::fromLatin1(QTest::currentDataTag());
    QStringList styleAndFileName = tagStr.split('/');
    QCOMPARE(styleAndFileName.size(), 2);
    QString style = styleAndFileName.first();
    style[0] = style.at(0).toUpper();
    QVERIFY(styleHelper.updateStyle(style));

    QQmlComponent component(styleHelper.engine.data());
    component.loadUrl(url);

    QObjectList objects;
    objects.reserve(4096);
    QBENCHMARK {
        QObject *object = component.create();
        QVERIFY2(object, qPrintable(component.errorString()));
        objects += object;
    }
    qDeleteAll(objects);
}

void tst_CreationTime::basic()
{
    QFETCH(QUrl, url);
    doBenchmark(styleHelper, url);
}

static const QStringList commonExclusions = {
    "ApplicationWindow",
    // Singleton; can't be created.
    "Calendar",
    // Can only be created by HorizontalHeaderView.
    "HorizontalHeaderViewDelegate",
    // Can only be created by TreeView.
    "TreeViewDelegate",
    // Can only be created by TableView.
    "TableViewDelegate",
    // Can only be created by VerticalHeaderView.
    "VerticalHeaderViewDelegate"
};

void tst_CreationTime::basic_data()
{
    QTest::addColumn<QUrl>("url");
    addTestRowForEachControl(styleHelper.engine.data(), QQC2_IMPORT_PATH, "basic",
        "QtQuick/Controls/Basic", commonExclusions);
}

void tst_CreationTime::fusion()
{
    QFETCH(QUrl, url);
    doBenchmark(styleHelper, url);
}

void tst_CreationTime::fusion_data()
{
    QTest::addColumn<QUrl>("url");
    addTestRowForEachControl(styleHelper.engine.data(), QQC2_IMPORT_PATH, "fusion",
        "QtQuick/Controls/Fusion", QStringList()
            << commonExclusions
            << "ButtonPanel"
            << "CheckIndicator"
            << "RadioIndicator"
            << "SliderGroove"
            << "SliderHandle"
            << "SwitchIndicator");
}

void tst_CreationTime::imagine()
{
    QFETCH(QUrl, url);
    doBenchmark(styleHelper, url);
}

void tst_CreationTime::imagine_data()
{
    QTest::addColumn<QUrl>("url");
    addTestRowForEachControl(styleHelper.engine.data(), QQC2_IMPORT_PATH, "imagine",
        "QtQuick/Controls/Imagine", commonExclusions);
}

void tst_CreationTime::material()
{
    QFETCH(QUrl, url);
    doBenchmark(styleHelper, url);
}

void tst_CreationTime::material_data()
{
    QTest::addColumn<QUrl>("url");
    addTestRowForEachControl(styleHelper.engine.data(), QQC2_IMPORT_PATH, "material",
        "QtQuick/Controls/Material", QStringList()
            << commonExclusions
            << "BoxShadow"
            << "CheckIndicator"
            << "CursorDelegate"
            << "ElevationEffect"
            << "RadioIndicator"
            << "Ripple"
            << "SliderHandle"
            << "SwitchIndicator");
}

void tst_CreationTime::universal()
{
    QFETCH(QUrl, url);
    doBenchmark(styleHelper, url);
}

void tst_CreationTime::universal_data()
{
    QTest::addColumn<QUrl>("url");
    addTestRowForEachControl(styleHelper.engine.data(), QQC2_IMPORT_PATH, "universal",
        "QtQuick/Controls/Universal", QStringList()
            << commonExclusions
            << "CheckIndicator"
            << "RadioIndicator"
            << "SwitchIndicator");
}

QTEST_MAIN(tst_CreationTime)

#include "tst_creationtime.moc"
