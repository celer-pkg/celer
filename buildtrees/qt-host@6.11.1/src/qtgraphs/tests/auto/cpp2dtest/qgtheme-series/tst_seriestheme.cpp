// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGraphs/QSeriesTheme>
#include <QtTest/QtTest>

class tst_seriestheme : public QObject
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

    void setResetSeries();

private:
    QSeriesTheme *m_theme;
};

void tst_seriestheme::initTestCase() {}

void tst_seriestheme::cleanupTestCase() {}

void tst_seriestheme::init()
{
    m_theme = new QSeriesTheme();
}

void tst_seriestheme::cleanup()
{
    delete m_theme;
}

void tst_seriestheme::construct()
{
    QSeriesTheme *theme = new QSeriesTheme();
    QVERIFY(theme);
    delete theme;
}

void tst_seriestheme::initialProperties()
{
    QVERIFY(m_theme);

    QCOMPARE(m_theme->colorTheme(), QSeriesTheme::SeriesColorTheme::SeriesTheme1);
    QCOMPARE(m_theme->colors(), {});
    QCOMPARE(m_theme->borderColors(), {});
    QCOMPARE(m_theme->borderWidth(), 0);
}

void tst_seriestheme::initializeProperties()
{
    QVERIFY(m_theme);

    QList<QColor> colors = {"#ff0000", "#ee0000"};
    QList<QColor> bordercolors = {"#ffff00", "#eeee00"};

    m_theme->setColorTheme(QSeriesTheme::SeriesColorTheme::SeriesTheme2);
    m_theme->setColors(colors);             // override colors
    m_theme->setBorderColors(bordercolors); // override bordercolors
    m_theme->setBorderWidth(1.0);

    QCOMPARE(m_theme->colorTheme(), QSeriesTheme::SeriesColorTheme::SeriesTheme2);
    QCOMPARE(m_theme->colors(), colors);
    QCOMPARE(m_theme->borderColors(), bordercolors);
    QCOMPARE(m_theme->borderWidth(), 1.0);
}

void tst_seriestheme::invalidProperties()
{
    QVERIFY(m_theme);

    m_theme->setBorderWidth(-1.0);

    QCOMPARE(m_theme->borderWidth(), 0.0);
}

void tst_seriestheme::setResetSeries()
{
    QVERIFY(m_theme);

    QList<QColor> colors1 = {"#3d9c73",
                             "#63b179",
                             "#88c580",
                             "#aed987",
                             "#d6ec91",
                             "#ffff9d",
                             "#fee17e",
                             "#fcc267",
                             "#f7a258",
                             "#ef8250",
                             "#e4604e",
                             "#d43d51"};
    QList<QColor> bordercolors1 = {"#ffffff"};

    QList<QColor> colors2 = {"#00429d",
                             "#485ba8",
                             "#6c77b3",
                             "#8a94be",
                             "#a4b2ca",
                             "#b9d4d6",
                             "#ffd3bf",
                             "#ffa59e",
                             "#f4777f",
                             "#dd4c65",
                             "#be214d",
                             "#93003a"};
    QList<QColor> bordercolors2 = {"#ffffff"};

    // Count
    QCOMPARE(m_theme->graphSeriesCount(), 0);

    // Set theme to SeriesTheme1
    m_theme->setColorTheme(QSeriesTheme::SeriesColorTheme::SeriesTheme1);

    QCOMPARE(m_theme->colors(), colors1);
    QCOMPARE(m_theme->borderColors(), bordercolors1);
    QCOMPARE(m_theme->graphSeriesColor(5),
             "#ff3d9c73"); // 0 series, this should return the first color

    // Set theme to SeriesTheme2
    m_theme->setColorTheme(QSeriesTheme::SeriesColorTheme::SeriesTheme2);

    QCOMPARE(m_theme->colors(), colors2);
    QCOMPARE(m_theme->borderColors(), bordercolors2);
    QCOMPARE(m_theme->indexColorFrom(m_theme->colors(), 2),
             "#ff00429d"); // 0 series, this should return the first color

    // Reset theme
    m_theme->resetColorTheme();

    QCOMPARE(m_theme->colorTheme(), QSeriesTheme::SeriesColorTheme::SeriesTheme1);
    QCOMPARE(m_theme->colors(), colors1);
    QCOMPARE(m_theme->borderColors(), bordercolors1);

    // Change series count
    m_theme->setGraphSeriesCount(12);

    QCOMPARE(m_theme->graphSeriesCount(), 12);
    QCOMPARE(m_theme->graphSeriesColor(5), "#ffffff9d");
    QCOMPARE(m_theme->graphSeriesBorderColor(0), "#ffffffff");
    QCOMPARE(m_theme->indexColorFrom(m_theme->colors(), 2), "#ff88c580");
}

QTEST_MAIN(tst_seriestheme)
#include "tst_seriestheme.moc"
