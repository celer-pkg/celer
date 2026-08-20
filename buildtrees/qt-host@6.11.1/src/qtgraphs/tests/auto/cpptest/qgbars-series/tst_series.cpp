// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QBar3DSeries>

#include <QtGui/qquaternion.h>

class tst_series: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void construct();

    void initialProperties();
    void initializeProperties();
    void invalidProperties();

private:
    QBar3DSeries *m_series;
};

void tst_series::initTestCase()
{
}

void tst_series::cleanupTestCase()
{
}

void tst_series::init()
{
    m_series = new QBar3DSeries();
}

void tst_series::cleanup()
{
    delete m_series;
}

void tst_series::construct()
{
    QBar3DSeries *series = new QBar3DSeries();
    QVERIFY(series);
    delete series;

    QBarDataProxy *proxy = new QBarDataProxy();

    series = new QBar3DSeries(proxy);
    QVERIFY(series);
    QCOMPARE(series->dataProxy(), proxy);
    delete series;
}

void tst_series::initialProperties()
{
    QVERIFY(m_series);

    QVERIFY(m_series->dataProxy());
    QCOMPARE(m_series->meshAngle(), 0.0f);
    QCOMPARE(m_series->selectedBar(), m_series->invalidSelectionPosition());
    QCOMPARE(m_series->rowColors().size(), 0);

    // Common properties
    QCOMPARE(m_series->baseColor(), QColor(Qt::black));
    QCOMPARE(m_series->baseGradient(), QLinearGradient());
    QCOMPARE(m_series->colorStyle(), QGraphsTheme::ColorStyle::Uniform);
    QCOMPARE(m_series->itemLabel(), QString(""));
    QCOMPARE(m_series->itemLabelFormat(), QString("@valueLabel"));
    QCOMPARE(m_series->isItemLabelVisible(), true);
    QCOMPARE(m_series->mesh(), QAbstract3DSeries::Mesh::BevelBar);
    QCOMPARE(m_series->meshRotation(), QQuaternion(1, 0, 0, 0));
    QCOMPARE(m_series->isMeshSmooth(), false);
    QCOMPARE(m_series->multiHighlightColor(), QColor(Qt::black));
    QCOMPARE(m_series->multiHighlightGradient(), QLinearGradient());
    QCOMPARE(m_series->lightingMode(), QAbstract3DSeries::LightingMode::Shaded);
    QCOMPARE(m_series->name(), QString(""));
    QCOMPARE(m_series->singleHighlightColor(), QColor(Qt::black));
    QCOMPARE(m_series->singleHighlightGradient(), QLinearGradient());
    QCOMPARE(m_series->type(), QAbstract3DSeries::SeriesType::Bar);
    QCOMPARE(m_series->userDefinedMesh(), QString(""));
    QCOMPARE(m_series->isVisible(), true);
    QCOMPARE(m_series->isValueColoringEnabled(), false);
}

void tst_series::initializeProperties()
{
    QVERIFY(m_series);

    QSignalSpy proxySpy(m_series, &QBar3DSeries::dataProxyChanged);
    QSignalSpy selectedBarSpy(m_series, &QBar3DSeries::selectedBarChanged);
    QSignalSpy meshAngleSpy(m_series, &QBar3DSeries::meshAngleChanged);
    QSignalSpy rowcolorSpy(m_series, &QBar3DSeries::rowColorsChanged);
    QSignalSpy rowLabelsSpy(m_series, &QBar3DSeries::rowLabelsChanged);
    QSignalSpy columnLabelsSpy(m_series, &QBar3DSeries::columnLabelsChanged);
    QSignalSpy dataArraySpy(m_series, &QBar3DSeries::dataArrayChanged);

    // Common signals
    QSignalSpy itemLabelFormatSpy(m_series, &QBar3DSeries::itemLabelFormatChanged);
    QSignalSpy visibleSpy(m_series, &QBar3DSeries::visibleChanged);
    QSignalSpy meshSpy(m_series, &QBar3DSeries::meshChanged);
    QSignalSpy meshSmoothSpy(m_series, &QBar3DSeries::meshSmoothChanged);
    QSignalSpy meshRotationSpy(m_series, &QBar3DSeries::meshRotationChanged);
    QSignalSpy userDefinedMeshSpy(m_series, &QBar3DSeries::userDefinedMeshChanged);
    QSignalSpy colorStyleSpy(m_series, &QBar3DSeries::colorStyleChanged);
    QSignalSpy baseColorSpy(m_series, &QBar3DSeries::baseColorChanged);
    QSignalSpy singleHighlightColorSpy(m_series, &QBar3DSeries::singleHighlightColorChanged);
    QSignalSpy singleHighlightGradientSpy(m_series, &QBar3DSeries::singleHighlightGradientChanged);
    QSignalSpy multiHighlightColorSpy(m_series, &QBar3DSeries::multiHighlightColorChanged);
    QSignalSpy multiHighlightGradientSpy(m_series, &QBar3DSeries::multiHighlightGradientChanged);
    QSignalSpy lightingModeSpy(m_series, &QBar3DSeries::lightingModeChanged);
    QSignalSpy nameSpy(m_series, &QBar3DSeries::nameChanged);
    QSignalSpy itemLabelSpy(m_series, &QBar3DSeries::itemLabelChanged);
    QSignalSpy itemLabelVisibleSpy(m_series, &QBar3DSeries::itemLabelVisibleChanged);

    m_series->setDataProxy(new QBarDataProxy());
    m_series->setMeshAngle(15.0f);
    m_series->setSelectedBar(QPoint(0, 0));

    QCOMPARE(m_series->meshAngle(), 15.0f);
    QCOMPARE(m_series->selectedBar(), QPoint(0, 0));

    QCOMPARE(proxySpy.size(), 1);
    QCOMPARE(meshAngleSpy.size(), 1);
    QCOMPARE(selectedBarSpy.size(), 1);

    QLinearGradient gradient1;
    gradient1.setColorAt(0.0, Qt::red);
    gradient1.setColorAt(1.0, Qt::blue);
    QLinearGradient gradient2;
    gradient2.setColorAt(0.0, Qt::yellow);
    gradient2.setColorAt(1.0, Qt::green);
    QLinearGradient gradient3;
    gradient3.setColorAt(0.0, Qt::white);
    gradient3.setColorAt(1.0, Qt::gray);

    QList<QColor> rowColors;
    rowColors.append(QColor(Qt::green));
    rowColors.append(QColor(Qt::blue));
    rowColors.append(QColor(Qt::red));

    QStringList rowLabels = { "monday", "tuesday", "wednesday"};
    QStringList columnLabels = { "2000", "2001", "2002"};

    m_series->setRowColors(rowColors);

    // Common properties
    m_series->setBaseColor(QColor(Qt::blue));
    m_series->setBaseGradient(gradient1);
    m_series->setColorStyle(QGraphsTheme::ColorStyle::RangeGradient);
    m_series->setItemLabelFormat("%f");
    m_series->setItemLabelVisible(false);
    m_series->setMesh(QAbstract3DSeries::Mesh::Cone);
    m_series->setMeshSmooth(true);
    m_series->setMultiHighlightColor(QColor(Qt::green));
    m_series->setMultiHighlightGradient(gradient2);
    m_series->setLightingMode(QAbstract3DSeries::LightingMode::Unshaded);
    m_series->setName("name");
    m_series->setSingleHighlightColor(QColor(Qt::red));
    m_series->setSingleHighlightGradient(gradient3);
    m_series->setUserDefinedMesh(":/customitem.mesh");
    m_series->setVisible(false);
    m_series->setValueColoringEnabled(true);
    m_series->setRowLabels(rowLabels);
    m_series->setColumnLabels(columnLabels);

    QCOMPARE(m_series->baseColor(), QColor(Qt::blue));
    QCOMPARE(m_series->baseGradient(), gradient1);
    QCOMPARE(m_series->baseGradient().stops().at(0).second, QColor(Qt::red));
    QCOMPARE(m_series->colorStyle(), QGraphsTheme::ColorStyle::RangeGradient);
    QCOMPARE(m_series->itemLabelFormat(), QString("%f"));
    QCOMPARE(m_series->isItemLabelVisible(), false);
    QCOMPARE(m_series->mesh(), QAbstract3DSeries::Mesh::Cone);
    QCOMPARE(m_series->isMeshSmooth(), true);
    QCOMPARE(m_series->multiHighlightColor(), QColor(Qt::green));
    QCOMPARE(m_series->multiHighlightGradient(), gradient2);
    QCOMPARE(m_series->multiHighlightGradient().stops().at(0).second, QColor(Qt::yellow));
    QCOMPARE(m_series->lightingMode(), QAbstract3DSeries::LightingMode::Unshaded);
    QCOMPARE(m_series->name(), QString("name"));
    QCOMPARE(m_series->singleHighlightColor(), QColor(Qt::red));
    QCOMPARE(m_series->singleHighlightGradient(), gradient3);
    QCOMPARE(m_series->singleHighlightGradient().stops().at(0).second, QColor(Qt::white));
    QCOMPARE(m_series->userDefinedMesh(), QString(":/customitem.mesh"));
    QCOMPARE(m_series->isVisible(), false);
    QCOMPARE(m_series->isValueColoringEnabled(), true);
    QCOMPARE(m_series->rowLabels(), rowLabels);
    QCOMPARE(m_series->columnLabels(), columnLabels);

    QCOMPARE(m_series->rowColors().size(), 3);
    QCOMPARE(m_series->rowColors().at(0), QColor(Qt::green));
    QCOMPARE(m_series->rowColors().at(1), QColor(Qt::blue));
    QCOMPARE(m_series->rowColors().at(2), QColor(Qt::red));

    QCOMPARE(rowcolorSpy.size(), 1);
    QCOMPARE(rowLabelsSpy.size(), 1);
    QCOMPARE(columnLabelsSpy.size(), 1);
    QCOMPARE(dataArraySpy.size(), 0); // signal is never emitted anywhere.

    QCOMPARE(itemLabelFormatSpy.size(), 1);
    QCOMPARE(visibleSpy.size(), 1);
    QCOMPARE(meshSpy.size(), 1);
    QCOMPARE(meshSmoothSpy.size(), 1);
    QCOMPARE(meshRotationSpy.size(), 1);
    QCOMPARE(userDefinedMeshSpy.size(), 1);
    QCOMPARE(colorStyleSpy.size(), 1);
    QCOMPARE(baseColorSpy.size(), 1);
    QCOMPARE(singleHighlightColorSpy.size(), 1);
    QCOMPARE(singleHighlightGradientSpy.size(), 1);
    QCOMPARE(multiHighlightColorSpy.size(), 1);
    QCOMPARE(multiHighlightGradientSpy.size(), 1);
    QCOMPARE(lightingModeSpy.size(), 1);
    QCOMPARE(nameSpy.size(), 1);
    QCOMPARE(itemLabelSpy.size(), 0);
    QCOMPARE(itemLabelVisibleSpy.size(), 1);
}

void tst_series::invalidProperties()
{
    m_series->setMesh(QAbstract3DSeries::Mesh::Minimal);

    QCOMPARE(m_series->mesh(), QAbstract3DSeries::Mesh::BevelBar);
}

QTEST_MAIN(tst_series)
#include "tst_series.moc"
