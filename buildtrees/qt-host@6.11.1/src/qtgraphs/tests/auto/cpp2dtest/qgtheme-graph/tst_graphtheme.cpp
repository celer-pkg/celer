// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGraphs/QGraphTheme>
#include <QtTest/QtTest>

class tst_graphtheme : public QObject
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

    void overrideProperties();
    void setReset();

private:
    QGraphTheme *m_theme;
};

void tst_graphtheme::initTestCase() {}

void tst_graphtheme::cleanupTestCase() {}

void tst_graphtheme::init()
{
    m_theme = new QGraphTheme();
}

void tst_graphtheme::cleanup()
{
    delete m_theme;
}

void tst_graphtheme::construct()
{
    QGraphTheme *theme = new QGraphTheme();
    QVERIFY(theme);
    delete theme;
}

void tst_graphtheme::initialProperties()
{
    QVERIFY(m_theme);

    QCOMPARE(m_theme->colorTheme(), QGraphTheme::ColorTheme::Dark);
    QCOMPARE(m_theme->gridMajorBarsWidth(), 2.0);
    QCOMPARE(m_theme->gridMinorBarsWidth(), 1.0);
    QCOMPARE(m_theme->gridSmoothing(), 1.0);
    QCOMPARE(m_theme->gridMajorBarsColor(), QColor());
    QCOMPARE(m_theme->gridMinorBarsColor(), QColor());
    QCOMPARE(m_theme->axisYMajorColor(), QColor());
    QCOMPARE(m_theme->axisYMinorColor(), QColor());
    QCOMPARE(m_theme->axisYMajorBarWidth(), 2.0);
    QCOMPARE(m_theme->axisYMinorBarWidth(), 1.0);
    QCOMPARE(m_theme->axisYSmoothing(), 1.0);
    QCOMPARE(m_theme->axisYLabelsColor(), QColor());
    QCOMPARE(m_theme->axisYLabelsFont(), QFont());
    QCOMPARE(m_theme->axisXMajorColor(), QColor());
    QCOMPARE(m_theme->axisXMinorColor(), QColor());
    QCOMPARE(m_theme->axisXMajorBarWidth(), 2.0);
    QCOMPARE(m_theme->axisXMinorBarWidth(), 1.0);
    QCOMPARE(m_theme->axisXSmoothing(), 1.0);
    QCOMPARE(m_theme->axisXLabelsColor(), QColor());
    QCOMPARE(m_theme->axisXLabelsFont(), QFont());
    QCOMPARE(m_theme->shadowVisible(), false);
    QCOMPARE(m_theme->shadowColor(), QColor());
    QCOMPARE(m_theme->shadowBarWidth(), 0.0);
    QCOMPARE(m_theme->shadowXOffset(), 0.0);
    QCOMPARE(m_theme->shadowYOffset(), 0.0);
    QCOMPARE(m_theme->shadowSmoothing(), 5.0);
}

void tst_graphtheme::initializeProperties()
{
    QVERIFY(m_theme);

    m_theme->setColorTheme(QGraphTheme::ColorTheme::Dark);
    m_theme->setGridMajorBarsWidth(3.5);
    m_theme->setGridMinorBarsWidth(1.5);
    m_theme->setGridSmoothing(0.5);
    m_theme->setAxisYMajorBarWidth(4.5);
    m_theme->setAxisYMinorBarWidth(2.5);
    m_theme->setAxisYSmoothing(2.0);
    m_theme->setAxisYLabelsFont(QFont("Arial", 30));
    m_theme->setAxisXMajorBarWidth(6.0);
    m_theme->setAxisXMinorBarWidth(5.0);
    m_theme->setAxisXSmoothing(3.0);
    m_theme->setAxisXLabelsFont(QFont("Arial", 20));
    m_theme->setShadowVisible(true);
    m_theme->setShadowColor("#66000000");
    m_theme->setShadowBarWidth(2.0);
    m_theme->setShadowXOffset(1.0);
    m_theme->setShadowYOffset(-1.0);
    m_theme->setShadowSmoothing(2.0);

    QCOMPARE(m_theme->colorTheme(), QGraphTheme::ColorTheme::Dark);
    QCOMPARE(m_theme->gridMajorBarsWidth(), 3.5);
    QCOMPARE(m_theme->gridMinorBarsWidth(), 1.5);
    QCOMPARE(m_theme->gridSmoothing(), 0.5);
    QCOMPARE(m_theme->gridMajorBarsColor(), "#fffafafa");
    QCOMPARE(m_theme->gridMinorBarsColor(), "#ff969696");
    QCOMPARE(m_theme->axisYMajorColor(), "#fffafafa");
    QCOMPARE(m_theme->axisYMinorColor(), "#ff969696");
    QCOMPARE(m_theme->axisYMajorBarWidth(), 4.5);
    QCOMPARE(m_theme->axisYMinorBarWidth(), 2.5);
    QCOMPARE(m_theme->axisYSmoothing(), 2.0);
    QCOMPARE(m_theme->axisYLabelsColor(), "#fffafafa");
    QCOMPARE(m_theme->axisYLabelsFont(), QFont("Arial", 30));
    QCOMPARE(m_theme->axisXMajorColor(), "#fffafafa");
    QCOMPARE(m_theme->axisXMinorColor(), "#ff969696");
    QCOMPARE(m_theme->axisXMajorBarWidth(), 6.0);
    QCOMPARE(m_theme->axisXMinorBarWidth(), 5.0);
    QCOMPARE(m_theme->axisXSmoothing(), 3.0);
    QCOMPARE(m_theme->axisXLabelsColor(), "#fffafafa");
    QCOMPARE(m_theme->axisXLabelsFont(), QFont("Arial", 20));
    QCOMPARE(m_theme->shadowVisible(), true);
    QCOMPARE(m_theme->shadowColor(), "#66000000");
    QCOMPARE(m_theme->shadowBarWidth(), 2.0);
    QCOMPARE(m_theme->shadowXOffset(), 1.0);
    QCOMPARE(m_theme->shadowYOffset(), -1.0);
    QCOMPARE(m_theme->shadowSmoothing(), 2.0);
}

void tst_graphtheme::overrideProperties()
{
    QVERIFY(m_theme);

    // Calling this manually to initialize the theming
    m_theme->componentComplete();

    // Set a theme
    m_theme->setColorTheme(QGraphTheme::ColorTheme::Light);

    QCOMPARE(m_theme->colorTheme(), QGraphTheme::ColorTheme::Light);
    QCOMPARE(m_theme->gridMajorBarsColor(), "#ff141414");
    QCOMPARE(m_theme->gridMinorBarsColor(), "#ff323232");
    QCOMPARE(m_theme->axisYMajorColor(), "#ff141414");
    QCOMPARE(m_theme->axisYMinorColor(), "#ff323232");
    QCOMPARE(m_theme->axisXMajorColor(), "#ff141414");
    QCOMPARE(m_theme->axisXMinorColor(), "#ff323232");
    QCOMPARE(m_theme->axisYLabelsColor(), "#ff141414");
    QCOMPARE(m_theme->axisXLabelsColor(), "#ff141414");

    // Override colors individually
    m_theme->setGridMajorBarsColor("red");
    m_theme->setGridMinorBarsColor("green");
    m_theme->setAxisYMajorColor("yellow");
    m_theme->setAxisYMinorColor("blue");
    m_theme->setAxisXMajorColor("#ffddeeff");
    m_theme->setAxisXMinorColor("#ff112233");
    m_theme->setAxisYLabelsColor("#ffaabbcc");
    m_theme->setAxisXLabelsColor("#ff445566");

    QCOMPARE(m_theme->colorTheme(), QGraphTheme::ColorTheme::Light);
    QCOMPARE(m_theme->gridMajorBarsColor(), "red");
    QCOMPARE(m_theme->gridMinorBarsColor(), "green");
    QCOMPARE(m_theme->axisYMajorColor(), "yellow");
    QCOMPARE(m_theme->axisYMinorColor(), "blue");
    QCOMPARE(m_theme->axisXMajorColor(), "#ffddeeff");
    QCOMPARE(m_theme->axisXMinorColor(), "#ff112233");
    QCOMPARE(m_theme->axisYLabelsColor(), "#ffaabbcc");
    QCOMPARE(m_theme->axisXLabelsColor(), "#ff445566");

    // Override with a theme
    m_theme->setColorTheme(QGraphTheme::ColorTheme::Dark);

    QCOMPARE(m_theme->colorTheme(), QGraphTheme::ColorTheme::Dark);
    QCOMPARE(m_theme->gridMajorBarsColor(), "#fffafafa");
    QCOMPARE(m_theme->gridMinorBarsColor(), "#ff969696");
    QCOMPARE(m_theme->axisYMajorColor(), "#fffafafa");
    QCOMPARE(m_theme->axisYMinorColor(), "#ff969696");
    QCOMPARE(m_theme->axisXMajorColor(), "#fffafafa");
    QCOMPARE(m_theme->axisXMinorColor(), "#ff969696");
    QCOMPARE(m_theme->axisYLabelsColor(), "#fffafafa");
    QCOMPARE(m_theme->axisXLabelsColor(), "#fffafafa");
}

void tst_graphtheme::setReset()
{
    QVERIFY(m_theme);

    // Calling this manually to initialize the theming
    m_theme->componentComplete();

    // Set a theme
    m_theme->setColorTheme(QGraphTheme::ColorTheme::Light);

    QCOMPARE(m_theme->colorTheme(), QGraphTheme::ColorTheme::Light);
    QCOMPARE(m_theme->gridMajorBarsColor(), "#ff141414");
    QCOMPARE(m_theme->gridMinorBarsColor(), "#ff323232");
    QCOMPARE(m_theme->axisYMajorColor(), "#ff141414");
    QCOMPARE(m_theme->axisYMinorColor(), "#ff323232");
    QCOMPARE(m_theme->axisXMajorColor(), "#ff141414");
    QCOMPARE(m_theme->axisXMinorColor(), "#ff323232");
    QCOMPARE(m_theme->axisYLabelsColor(), "#ff141414");
    QCOMPARE(m_theme->axisXLabelsColor(), "#ff141414");

    // Reset theme
    m_theme->resetColorTheme();

    QCOMPARE(m_theme->colorTheme(), QGraphTheme::ColorTheme::Dark);
    QCOMPARE(m_theme->gridMajorBarsColor(), "#fffafafa");
    QCOMPARE(m_theme->gridMinorBarsColor(), "#ff969696");
    QCOMPARE(m_theme->axisYMajorColor(), "#fffafafa");
    QCOMPARE(m_theme->axisYMinorColor(), "#ff969696");
    QCOMPARE(m_theme->axisXMajorColor(), "#fffafafa");
    QCOMPARE(m_theme->axisXMinorColor(), "#ff969696");
    QCOMPARE(m_theme->axisYLabelsColor(), "#fffafafa");
    QCOMPARE(m_theme->axisXLabelsColor(), "#fffafafa");

    // Set another theme
    m_theme->setColorTheme(QGraphTheme::ColorTheme::Light);

    QCOMPARE(m_theme->colorTheme(), QGraphTheme::ColorTheme::Light);
    QCOMPARE(m_theme->gridMajorBarsColor(), "#ff141414");
    QCOMPARE(m_theme->gridMinorBarsColor(), "#ff323232");
    QCOMPARE(m_theme->axisYMajorColor(), "#ff141414");
    QCOMPARE(m_theme->axisYMinorColor(), "#ff323232");
    QCOMPARE(m_theme->axisXMajorColor(), "#ff141414");
    QCOMPARE(m_theme->axisXMinorColor(), "#ff323232");
    QCOMPARE(m_theme->axisYLabelsColor(), "#ff141414");
    QCOMPARE(m_theme->axisXLabelsColor(), "#ff141414");
}

QTEST_MAIN(tst_graphtheme)
#include "tst_graphtheme.moc"
