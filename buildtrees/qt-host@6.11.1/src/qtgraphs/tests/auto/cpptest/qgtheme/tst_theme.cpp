// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtGraphs/QGraphsTheme>

class tst_theme: public QObject
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
    void initializeGraphsLine();

private:
    QGraphsTheme *m_theme;
};

void tst_theme::initTestCase()
{
}

void tst_theme::cleanupTestCase()
{
}

void tst_theme::init()
{
    m_theme = new QGraphsTheme();
    m_theme->setColorScheme(QGraphsTheme::ColorScheme::Light);
}

void tst_theme::cleanup()
{
    delete m_theme;
}

void tst_theme::construct()
{
    QGraphsTheme *theme = new QGraphsTheme();
    QVERIFY(theme);
    delete theme;

    theme = new QGraphsTheme();
    theme->setTheme(QGraphsTheme::Theme::MixSeries);
    theme->setColorScheme(QGraphsTheme::ColorScheme::Light);
    QVERIFY(theme);
    QCOMPARE(theme->plotAreaBackgroundColor(), QColor(QRgb(0xFCFCFC)));
    QCOMPARE(theme->isPlotAreaBackgroundVisible(), true);
    QCOMPARE(theme->seriesColors().size(), 5);
    QCOMPARE(theme->seriesColors().at(0), QColor(QRgb(0xFFA615)));
    QCOMPARE(theme->seriesColors().at(4), QColor(QRgb(0x0128F8)));
    QCOMPARE(theme->seriesGradients().size(), 5);
    QCOMPARE(theme->seriesGradients().at(0).stops().at(1).second, QColor(QRgb(0xFFA615)));
    QCOMPARE(theme->seriesGradients().at(4).stops().at(1).second, QColor(QRgb(0x0128F8)));
    QCOMPARE(theme->colorStyle(), QGraphsTheme::ColorStyle::Uniform);
    QCOMPARE(theme->labelFont(), QFont("Arial"));
    QCOMPARE(theme->isGridVisible(), true);
    QCOMPARE(theme->grid().mainColor(), QColor(QRgb(0x545151)));
    QCOMPARE(theme->grid().subColor(), QColor(QRgb(0xAFAFAF)));
    QCOMPARE(theme->grid().mainWidth(), 2.0f);
    QCOMPARE(theme->grid().subWidth(), 1.0f);
    QCOMPARE(theme->labelBackgroundColor(), QColor(QRgb(0xE7E7E7)));
    QCOMPARE(theme->isLabelBackgroundVisible(), true);
    QCOMPARE(theme->isLabelBorderVisible(), true);
    QCOMPARE(theme->labelTextColor(), QColor(QRgb(0x6A6A6A)));
    QCOMPARE(theme->multiHighlightColor(), QColor(QRgb(0x22D47B)));
    QCOMPARE(theme->multiHighlightGradient().stops().at(1).second, QColor(QRgb(0x22D47B)));
    QCOMPARE(theme->singleHighlightColor(), QColor(QRgb(0xCCDC00)));
    QCOMPARE(theme->singleHighlightGradient().stops().at(1).second, QColor(QRgb(0xCCDC00)));
    QCOMPARE(theme->theme(), QGraphsTheme::Theme::MixSeries);
    QCOMPARE(theme->backgroundColor(), QColor(QRgb(0xF2F2F2)));
    QCOMPARE(theme->isBackgroundVisible(), true);
    delete theme;
}

void tst_theme::initialProperties()
{
    QVERIFY(m_theme);

    QCOMPARE(m_theme->plotAreaBackgroundColor(), QColor(QRgb(0xFCFCFC)));
    QCOMPARE(m_theme->isPlotAreaBackgroundVisible(), true);
    QCOMPARE(m_theme->seriesColors().size(), 5);
    QCOMPARE(m_theme->seriesColors().at(0), QColor(QRgb(0xD5F8E7)));
    QCOMPARE(m_theme->seriesGradients().size(), 5);
    QCOMPARE(m_theme->seriesGradients().at(0).stops().at(0).second, QColor(QRgb(0x6A7C73)));
    QCOMPARE(m_theme->seriesGradients().at(0).stops().at(1).second, QColor(QRgb(0xD5F8E7)));
    QCOMPARE(m_theme->labelFont(), QFont(QLatin1String("Arial")));
    QCOMPARE(m_theme->isGridVisible(), true);
    QCOMPARE(m_theme->grid().mainColor(), QColor(QRgb(0x545151)));
    QCOMPARE(m_theme->grid().subColor(), QColor(QRgb(0xAFAFAF)));
    QCOMPARE(m_theme->grid().mainWidth(), 2.0f);
    QCOMPARE(m_theme->grid().subWidth(), 1.0f);
    QCOMPARE(m_theme->labelBackgroundColor(), QColor(QRgb(0xE7E7E7)));
    QCOMPARE(m_theme->isLabelBackgroundVisible(), true);
    QCOMPARE(m_theme->isLabelBorderVisible(), true);
    QCOMPARE(m_theme->labelTextColor(), QColor(QRgb(0x6A6A6A)));
    QCOMPARE(m_theme->multiHighlightColor(), QColor(QRgb(0x22D47B)));
    QCOMPARE(m_theme->multiHighlightGradient().stops().at(1).second, QColor(QRgb(0x22D47B)));
    QCOMPARE(m_theme->singleHighlightColor(), QColor(QRgb(0xCCDC00)));
    QCOMPARE(m_theme->singleHighlightGradient().stops().at(1).second, QColor(QRgb(0xCCDC00)));
    QCOMPARE(m_theme->theme(), QGraphsTheme::Theme::QtGreen);
    QCOMPARE(m_theme->backgroundColor(), QColor(QRgb(0xF2F2F2)));
    QCOMPARE(m_theme->isBackgroundVisible(), true);
}

void tst_theme::initializeProperties()
{
    QVERIFY(m_theme);

    QSignalSpy colorSchemeSpy(m_theme, &QGraphsTheme::colorSchemeChanged);
    QSignalSpy themeSpy(m_theme, &QGraphsTheme::themeChanged);
    QSignalSpy colorStyleSpy(m_theme, &QGraphsTheme::colorStyleChanged);
    QSignalSpy backgroundColorSpy(m_theme, &QGraphsTheme::backgroundColorChanged);
    QSignalSpy backgroundVisibleSpy(m_theme, &QGraphsTheme::backgroundVisibleChanged);
    QSignalSpy plotAreaBackgroundColorSpy(m_theme, &QGraphsTheme::plotAreaBackgroundColorChanged);
    QSignalSpy plotAreaBackgroundVisibleSpy(m_theme, &QGraphsTheme::plotAreaBackgroundVisibleChanged);
    QSignalSpy gridVisibleSpy(m_theme, &QGraphsTheme::gridVisibleChanged);

    QSignalSpy axisXLabelFontSpy(m_theme, &QGraphsTheme::axisXLabelFontChanged);
    QSignalSpy axisYLabelFontSpy(m_theme, &QGraphsTheme::axisYLabelFontChanged);
    QSignalSpy axisZLabelFontSpy(m_theme, &QGraphsTheme::axisZLabelFontChanged);

    QSignalSpy gridSpy(m_theme, &QGraphsTheme::gridChanged);
    QSignalSpy axisXSpy(m_theme, &QGraphsTheme::axisXChanged);
    QSignalSpy axisYSpy(m_theme, &QGraphsTheme::axisYChanged);
    QSignalSpy axisZSpy(m_theme, &QGraphsTheme::axisZChanged);

    QSignalSpy labelFontSpy(m_theme, &QGraphsTheme::labelFontChanged);
    QSignalSpy labelsVisibleSpy(m_theme, &QGraphsTheme::labelsVisibleChanged);
    QSignalSpy labelBackgroundColorSpy(m_theme, &QGraphsTheme::labelBackgroundColorChanged);
    QSignalSpy labelTextColorSpy(m_theme, &QGraphsTheme::labelTextColorChanged);
    QSignalSpy labelBackgroundVisibleSpy(m_theme, &QGraphsTheme::labelBackgroundVisibleChanged);
    QSignalSpy labelBorderVisibleSpy(m_theme, &QGraphsTheme::labelBorderVisibleChanged);

    QSignalSpy seriesGradientsSpy(m_theme, &QGraphsTheme::seriesGradientsChanged);
    QSignalSpy seriesColorsSpy(m_theme, &QGraphsTheme::seriesColorsChanged);
    QSignalSpy borderColorsSpy(m_theme, &QGraphsTheme::borderColorsChanged);
    QSignalSpy borderWidthSpy(m_theme, &QGraphsTheme::borderWidthChanged);

    QSignalSpy singleHighlightColorSpy(m_theme, &QGraphsTheme::singleHighlightColorChanged);
    QSignalSpy multiHighlightColorSpy(m_theme, &QGraphsTheme::multiHighlightColorChanged);
    QSignalSpy singleHighlightGradientSpy(m_theme, &QGraphsTheme::singleHighlightGradientChanged);
    QSignalSpy multiHighlightGradientSpy(m_theme, &QGraphsTheme::multiHighlightGradientChanged);


    QLinearGradient gradient1;
    QLinearGradient gradient2;
    QLinearGradient gradient3(QPoint(0.0f, 0.0f), QPoint(10.0f, 10.0f));
    QLinearGradient gradient4(QPoint(0.0f, 0.0f), QPoint(10.0f, 10.0f));

    QList<QColor> basecolors;
    basecolors << QColor(Qt::red) << QColor(Qt::blue);

    QList<QLinearGradient> basegradients;
    basegradients << gradient1 << gradient2;

    m_theme->setTheme(QGraphsTheme::Theme::OrangeSeries); // We'll override default values with the following setters
    m_theme->setColorScheme(QGraphsTheme::ColorScheme::Dark);
    m_theme->setPlotAreaBackgroundColor(QColor(Qt::red));
    m_theme->setPlotAreaBackgroundVisible(false);
    m_theme->setSeriesColors(basecolors);
    m_theme->setSeriesGradients(basegradients);
    m_theme->setColorStyle(QGraphsTheme::ColorStyle::RangeGradient);
    m_theme->setLabelFont(QFont("Times"));
    m_theme->setGridVisible(false);
    QGraphsLine grid = m_theme->grid();
    grid.setMainColor(QColor(Qt::green));
    grid.setSubColor(QColor(Qt::red));
    grid.setMainWidth(0.8f);
    grid.setSubWidth(0.5f);
    m_theme->setGrid(grid);
    m_theme->setLabelBackgroundColor(QColor(Qt::gray));
    m_theme->setLabelBackgroundVisible(false);
    m_theme->setLabelBorderVisible(false);
    m_theme->setLabelTextColor(QColor(Qt::cyan));
    m_theme->setMultiHighlightColor(QColor(Qt::darkBlue));
    m_theme->setMultiHighlightGradient(gradient3);
    m_theme->setSingleHighlightColor(QColor(Qt::darkRed));
    m_theme->setSingleHighlightGradient(gradient4);
    m_theme->setBackgroundColor(QColor(Qt::darkYellow));
    m_theme->setBackgroundVisible(false);
    m_theme->setAxisXLabelFont(QFont("helvetica"));
    m_theme->setAxisYLabelFont(QFont("helvetica"));
    m_theme->setAxisZLabelFont(QFont("Helvetica"));
    m_theme->setLabelsVisible(false);
    m_theme->setBorderColors(basecolors);
    m_theme->setBorderWidth(10.0f);

    QCOMPARE(m_theme->plotAreaBackgroundColor(), QColor(Qt::red));
    QCOMPARE(m_theme->isPlotAreaBackgroundVisible(), false);
    QCOMPARE(m_theme->seriesColors().size(), 2);
    QCOMPARE(m_theme->seriesColors().at(0), QColor(Qt::red));
    QCOMPARE(m_theme->seriesColors().at(1), QColor(Qt::blue));
    QCOMPARE(m_theme->seriesGradients().size(), 2);
    QCOMPARE(m_theme->seriesGradients().at(0), gradient1);
    QCOMPARE(m_theme->seriesGradients().at(0), gradient2);
    QCOMPARE(m_theme->colorStyle(), QGraphsTheme::ColorStyle::RangeGradient);
    QCOMPARE(m_theme->labelFont(), QFont("Times"));
    QCOMPARE(m_theme->isGridVisible(), false);
    QCOMPARE(m_theme->grid().mainColor(), QColor(Qt::green));
    QCOMPARE(m_theme->grid().subColor(), QColor(Qt::red));
    QCOMPARE(m_theme->grid().mainWidth(), 0.8f);
    QCOMPARE(m_theme->grid().subWidth(), 0.5f);
    QCOMPARE(m_theme->labelBackgroundColor(), QColor(Qt::gray));
    QCOMPARE(m_theme->isLabelBackgroundVisible(), false);
    QCOMPARE(m_theme->isLabelBorderVisible(), false);
    QCOMPARE(m_theme->labelTextColor(), QColor(Qt::cyan));
    QCOMPARE(m_theme->multiHighlightColor(), QColor(Qt::darkBlue));
    QCOMPARE(m_theme->multiHighlightGradient(), gradient3);
    QCOMPARE(m_theme->singleHighlightColor(), QColor(Qt::darkRed));
    QCOMPARE(m_theme->singleHighlightGradient(), gradient4);
    QCOMPARE(m_theme->theme(), QGraphsTheme::Theme::OrangeSeries);
    QCOMPARE(m_theme->backgroundColor(), QColor(Qt::darkYellow));
    QCOMPARE(m_theme->isBackgroundVisible(), false);

    QCOMPARE(colorSchemeSpy.size(), 1);
    QCOMPARE(themeSpy.size(), 1);
    QCOMPARE(colorStyleSpy.size(), 1);
    QCOMPARE(backgroundColorSpy.size(), 1);
    QCOMPARE(backgroundVisibleSpy.size(), 1);
    QCOMPARE(plotAreaBackgroundColorSpy.size(), 1);
    QCOMPARE(plotAreaBackgroundVisibleSpy.size(), 1);
    QCOMPARE(gridVisibleSpy.size(), 1);

    QCOMPARE(colorSchemeSpy.size(), 1);
    QCOMPARE(themeSpy.size(), 1);
    QCOMPARE(colorStyleSpy.size(), 1);
    QCOMPARE(backgroundColorSpy.size(), 1);
    QCOMPARE(backgroundVisibleSpy.size(), 1);
    QCOMPARE(plotAreaBackgroundColorSpy.size(), 1);
    QCOMPARE(plotAreaBackgroundVisibleSpy.size(), 1);
    QCOMPARE(gridVisibleSpy.size(), 1);

    QCOMPARE(axisXLabelFontSpy.size(), 1);
    QCOMPARE(axisYLabelFontSpy.size(), 1);
    QCOMPARE(axisZLabelFontSpy.size(), 1);

    QCOMPARE(gridSpy.size(), 2);
    QCOMPARE(axisXSpy.size(), 2);
    QCOMPARE(axisYSpy.size(), 2);
    QCOMPARE(axisZSpy.size(), 2);

    QCOMPARE(labelFontSpy.size(), 1);
    QCOMPARE(labelsVisibleSpy.size(), 1);
    QCOMPARE(labelBackgroundColorSpy.size(), 1);
    QCOMPARE(labelTextColorSpy.size(), 1);
    QCOMPARE(labelBackgroundVisibleSpy.size(), 1);
    QCOMPARE(labelBorderVisibleSpy.size(), 1);

    QCOMPARE(seriesGradientsSpy.size(), 1);
    QCOMPARE(seriesColorsSpy.size(), 1);
    QCOMPARE(borderColorsSpy.size(), 1);
    QCOMPARE(borderWidthSpy.size(), 1);

    QCOMPARE(singleHighlightColorSpy.size(), 1);
    QCOMPARE(multiHighlightColorSpy.size(), 1);
    QCOMPARE(singleHighlightGradientSpy.size(), 1);
    QCOMPARE(multiHighlightGradientSpy.size(), 1);
}

void tst_theme::initializeGraphsLine()
{
    QGraphsLine line;

    QCOMPARE(line.mainColor(), QColor());
    QCOMPARE(line.subColor(), QColor());
    QCOMPARE(line.labelTextColor(), QColor());
    QCOMPARE(line.mainWidth(), 2.0f);
    QCOMPARE(line.subWidth(), 1.0f);

    line.setMainColor(Qt::red);
    line.setSubColor(Qt::green);
    line.setLabelTextColor(Qt::gray);
    line.setMainWidth(25);
    line.setSubWidth(10);

    QCOMPARE(line.mainColor(), Qt::red);
    QCOMPARE(line.subColor(), Qt::green);
    QCOMPARE(line.labelTextColor(), Qt::gray);
    QCOMPARE(line.mainWidth(), 25);
    QCOMPARE(line.subWidth(), 10);

    QGraphsLine line2;

    line2.setMainColor(Qt::green);
    line2.setSubColor(Qt::red);
    line2.setLabelTextColor(Qt::darkGray);
    line2.setMainWidth(30);
    line2.setSubWidth(5);

    line = line2;

    QCOMPARE(line.mainColor(), line2.mainColor());
    QCOMPARE(line.subColor(), line2.subColor());
    QCOMPARE(line.labelTextColor(), line2.labelTextColor());
    QCOMPARE(line.mainWidth(), line2.mainWidth());
    QCOMPARE(line.subWidth(), line2.subWidth());

    QGraphsLine swapLine;

    swapLine.setMainColor(Qt::darkRed);
    swapLine.setSubColor(Qt::darkGreen);
    swapLine.setLabelTextColor(Qt::darkYellow);
    swapLine.setMainWidth(5);
    swapLine.setSubWidth(2);

    line.swap(swapLine);

    QCOMPARE(line.mainColor(), Qt::darkRed);
    QCOMPARE(line.subColor(), Qt::darkGreen);
    QCOMPARE(line.labelTextColor(), Qt::darkYellow);
    QCOMPARE(line.mainWidth(), 5);
    QCOMPARE(line.subWidth(), 2);

    QCOMPARE(swapLine.mainColor(), line2.mainColor());
    QCOMPARE(swapLine.subColor(), line2.subColor());
    QCOMPARE(swapLine.labelTextColor(), line2.labelTextColor());
    QCOMPARE(swapLine.mainWidth(), line2.mainWidth());
    QCOMPARE(swapLine.subWidth(), line2.subWidth());
}

QTEST_MAIN(tst_theme)
#include "tst_theme.moc"
