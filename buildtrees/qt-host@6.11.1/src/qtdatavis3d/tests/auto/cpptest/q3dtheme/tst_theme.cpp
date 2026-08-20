// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QtDataVisualization/Q3DTheme>

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
    void invalidProperties();

private:
    Q3DTheme *m_theme;
};

void tst_theme::initTestCase()
{
}

void tst_theme::cleanupTestCase()
{
}

void tst_theme::init()
{
    m_theme = new Q3DTheme();
}

void tst_theme::cleanup()
{
    delete m_theme;
}

void tst_theme::construct()
{
    Q3DTheme *theme = new Q3DTheme();
    QVERIFY(theme);
    delete theme;

    theme = new Q3DTheme(Q3DTheme::ThemeEbony);
    QVERIFY(theme);
    QCOMPARE(theme->ambientLightStrength(), 0.5f);
    QCOMPARE(theme->backgroundColor(), QColor(Qt::black));
    QCOMPARE(theme->isBackgroundEnabled(), true);
    QCOMPARE(theme->baseColors().size(), 5);
    QCOMPARE(theme->baseColors().at(0), QColor(Qt::white));
    QCOMPARE(theme->baseColors().at(4), QColor(QRgb(0x6b6b6b)));
    QCOMPARE(theme->baseGradients().size(), 5);
    QCOMPARE(theme->baseGradients().at(0).stops().at(1).second, QColor(Qt::white));
    QCOMPARE(theme->baseGradients().at(4).stops().at(1).second, QColor(QRgb(0x6b6b6b)));
    QCOMPARE(theme->colorStyle(), Q3DTheme::ColorStyleUniform);
    QCOMPARE(theme->font(), QFont("Arial"));
    QCOMPARE(theme->isGridEnabled(), true);
    QCOMPARE(theme->gridLineColor(), QColor(QRgb(0x35322f)));
    QCOMPARE(theme->highlightLightStrength(), 5.0f);
    QCOMPARE(theme->labelBackgroundColor(), QColor(0x00, 0x00, 0x00, 0xcd));
    QCOMPARE(theme->isLabelBackgroundEnabled(), true);
    QCOMPARE(theme->isLabelBorderEnabled(), false);
    QCOMPARE(theme->labelTextColor(), QColor(QRgb(0xaeadac)));
    QCOMPARE(theme->lightColor(), QColor(Qt::white));
    QCOMPARE(theme->lightStrength(), 5.0f);
    QCOMPARE(theme->multiHighlightColor(), QColor(QRgb(0xd72222)));
    QCOMPARE(theme->multiHighlightGradient().stops().at(1).second, QColor(QRgb(0xd72222)));
    QCOMPARE(theme->singleHighlightColor(), QColor(QRgb(0xf5dc0d)));
    QCOMPARE(theme->singleHighlightGradient().stops().at(1).second, QColor(QRgb(0xf5dc0d)));
    QCOMPARE(theme->type(), Q3DTheme::ThemeEbony);
    QCOMPARE(theme->windowColor(), QColor(Qt::black));
    delete theme;
}

void tst_theme::initialProperties()
{
    QVERIFY(m_theme);

    QCOMPARE(m_theme->ambientLightStrength(), 0.25f);
    QCOMPARE(m_theme->backgroundColor(), QColor(Qt::black));
    QCOMPARE(m_theme->isBackgroundEnabled(), true);
    QCOMPARE(m_theme->baseColors().size(), 1);
    QCOMPARE(m_theme->baseColors().at(0), QColor(Qt::black));
    QCOMPARE(m_theme->baseGradients().size(), 1);
    QCOMPARE(m_theme->baseGradients().at(0).stops().at(0).second, QColor(Qt::black));
    QCOMPARE(m_theme->baseGradients().at(0).stops().at(1).second, QColor(Qt::white));
    QCOMPARE(m_theme->colorStyle(), Q3DTheme::ColorStyleUniform);
    QCOMPARE(m_theme->font(), QFont());
    QCOMPARE(m_theme->isGridEnabled(), true);
    QCOMPARE(m_theme->gridLineColor(), QColor(Qt::white));
    QCOMPARE(m_theme->highlightLightStrength(), 7.5f);
    QCOMPARE(m_theme->labelBackgroundColor(), QColor(Qt::gray));
    QCOMPARE(m_theme->isLabelBackgroundEnabled(), true);
    QCOMPARE(m_theme->isLabelBorderEnabled(), true);
    QCOMPARE(m_theme->labelTextColor(), QColor(Qt::white));
    QCOMPARE(m_theme->lightColor(), QColor(Qt::white));
    QCOMPARE(m_theme->lightStrength(), 5.0f);
    QCOMPARE(m_theme->multiHighlightColor(), QColor(Qt::blue));
    QCOMPARE(m_theme->multiHighlightGradient().stops(), QLinearGradient().stops());
    QCOMPARE(m_theme->singleHighlightColor(), QColor(Qt::red));
    QCOMPARE(m_theme->singleHighlightGradient().stops(), QLinearGradient().stops());
    QCOMPARE(m_theme->type(), Q3DTheme::ThemeUserDefined);
    QCOMPARE(m_theme->windowColor(), QColor(Qt::black));
}

void tst_theme::initializeProperties()
{
    QVERIFY(m_theme);

    QLinearGradient gradient1;
    QLinearGradient gradient2;
    QLinearGradient gradient3;
    QLinearGradient gradient4;

    QList<QColor> basecolors;
    basecolors << QColor(Qt::red) << QColor(Qt::blue);

    QList<QLinearGradient> basegradients;
    basegradients << gradient1 << gradient2;

    m_theme->setType(Q3DTheme::ThemeQt); // We'll override default values with the following setters
    m_theme->setAmbientLightStrength(0.3f);
    m_theme->setBackgroundColor(QColor(Qt::red));
    m_theme->setBackgroundEnabled(false);
    m_theme->setBaseColors(basecolors);
    m_theme->setBaseGradients(basegradients);
    m_theme->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
    m_theme->setFont(QFont("Arial"));
    m_theme->setGridEnabled(false);
    m_theme->setGridLineColor(QColor(Qt::green));
    m_theme->setHighlightLightStrength(5.0f);
    m_theme->setLabelBackgroundColor(QColor(Qt::gray));
    m_theme->setLabelBackgroundEnabled(false);
    m_theme->setLabelBorderEnabled(false);
    m_theme->setLabelTextColor(QColor(Qt::cyan));
    m_theme->setLightColor(QColor(Qt::yellow));
    m_theme->setLightStrength(2.5f);
    m_theme->setMultiHighlightColor(QColor(Qt::darkBlue));
    m_theme->setMultiHighlightGradient(gradient3);
    m_theme->setSingleHighlightColor(QColor(Qt::darkRed));
    m_theme->setSingleHighlightGradient(gradient4);
    m_theme->setWindowColor(QColor(Qt::darkYellow));

    QCOMPARE(m_theme->ambientLightStrength(), 0.3f);
    QCOMPARE(m_theme->backgroundColor(), QColor(Qt::red));
    QCOMPARE(m_theme->isBackgroundEnabled(), false);
    QCOMPARE(m_theme->baseColors().size(), 2);
    QCOMPARE(m_theme->baseColors().at(0), QColor(Qt::red));
    QCOMPARE(m_theme->baseColors().at(1), QColor(Qt::blue));
    QCOMPARE(m_theme->baseGradients().size(), 2);
    QCOMPARE(m_theme->baseGradients().at(0), gradient1);
    QCOMPARE(m_theme->baseGradients().at(0), gradient2);
    QCOMPARE(m_theme->colorStyle(), Q3DTheme::ColorStyleRangeGradient);
    QCOMPARE(m_theme->font(), QFont("Arial"));
    QCOMPARE(m_theme->isGridEnabled(), false);
    QCOMPARE(m_theme->gridLineColor(), QColor(Qt::green));
    QCOMPARE(m_theme->highlightLightStrength(), 5.0f);
    QCOMPARE(m_theme->labelBackgroundColor(), QColor(Qt::gray));
    QCOMPARE(m_theme->isLabelBackgroundEnabled(), false);
    QCOMPARE(m_theme->isLabelBorderEnabled(), false);
    QCOMPARE(m_theme->labelTextColor(), QColor(Qt::cyan));
    QCOMPARE(m_theme->lightColor(), QColor(Qt::yellow));
    QCOMPARE(m_theme->lightStrength(), 2.5f);
    QCOMPARE(m_theme->multiHighlightColor(), QColor(Qt::darkBlue));
    QCOMPARE(m_theme->multiHighlightGradient(), gradient3);
    QCOMPARE(m_theme->singleHighlightColor(), QColor(Qt::darkRed));
    QCOMPARE(m_theme->singleHighlightGradient(), gradient4);
    QCOMPARE(m_theme->type(), Q3DTheme::ThemeQt);
    QCOMPARE(m_theme->windowColor(), QColor(Qt::darkYellow));
}

void tst_theme::invalidProperties()
{
    m_theme->setAmbientLightStrength(-1.0f);
    QCOMPARE(m_theme->ambientLightStrength(), 0.25f);
    m_theme->setAmbientLightStrength(1.1f);
    QCOMPARE(m_theme->ambientLightStrength(), 0.25f);

    m_theme->setHighlightLightStrength(-1.0f);
    QCOMPARE(m_theme->highlightLightStrength(), 7.5f);
    m_theme->setHighlightLightStrength(10.1f);
    QCOMPARE(m_theme->highlightLightStrength(), 7.5f);

    m_theme->setLightStrength(-1.0f);
    QCOMPARE(m_theme->lightStrength(), 5.0f);
    m_theme->setLightStrength(10.1f);
    QCOMPARE(m_theme->lightStrength(), 5.0f);
}

QTEST_MAIN(tst_theme)
#include "tst_theme.moc"
