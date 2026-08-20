// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#include <QtTest/QTest>
#include <QIcon>
#include <QImage>
#include <QImageReader>
#include <QList>
#include <QPixmap>
#include <QRegularExpression>
#include <QSize>
#include <QString>

class tst_QIcon_Svg : public QObject
{
    Q_OBJECT
public:

public slots:
    void initTestCase();

private slots:
    void svgActualSize();
    void svg();
    void availableSizes();
    void isNull();
    void sizeInPercent();
    void fromTheme_data();
    void fromTheme();

private:
    QString prefix;
};

void tst_QIcon_Svg::initTestCase()
{
    prefix = QFINDTESTDATA("icons/");
    if (prefix.isEmpty())
        QFAIL("Can't find images directory!");
    if (!QImageReader::supportedImageFormats().contains("svg"))
        QFAIL("SVG support is not available");
    QCOMPARE(QImageReader::imageFormat(prefix + "triangle.svg"), QByteArray("svg"));
}

void tst_QIcon_Svg::svgActualSize()
{
    QIcon icon(prefix + "rect.svg");
    QCOMPARE(icon.actualSize(QSize(16, 2)), QSize(16, 2));
    QCOMPARE(icon.pixmap(QSize(16, 16)).size(), QSize(16, 2));

    QPixmap p(16, 16);
    p.fill(Qt::cyan);
    icon.addPixmap(p);

    QCOMPARE(icon.actualSize(QSize(16, 16)), QSize(16, 16));
    QCOMPARE(icon.pixmap(QSize(16, 16)).size(), QSize(16, 16));

    QCOMPARE(icon.actualSize(QSize(16, 14)), QSize(16, 2));
    QCOMPARE(icon.pixmap(QSize(16, 14)).size(), QSize(16, 2));
}

void tst_QIcon_Svg::svg()
{
    QIcon icon1(prefix + "heart.svg");
    QVERIFY(!icon1.pixmap(32).isNull());
    QImage img1 = icon1.pixmap(32).toImage();
    QVERIFY(!icon1.pixmap(32, QIcon::Disabled).isNull());
    QImage img2 = icon1.pixmap(32, QIcon::Disabled).toImage();

    icon1.addFile(prefix + "trash.svg", QSize(), QIcon::Disabled);
    QVERIFY(!icon1.pixmap(32, QIcon::Disabled).isNull());
    QImage img3 = icon1.pixmap(32, QIcon::Disabled).toImage();
    QVERIFY(img3 != img2);
    QVERIFY(img3 != img1);

    QPixmap pm(prefix + "image.png");
    icon1.addPixmap(pm, QIcon::Normal, QIcon::On);
    icon1.addPixmap(pm.scaled(QSize(32, 32)), QIcon::Normal, QIcon::On);
    QVERIFY(!icon1.pixmap(128, QIcon::Normal, QIcon::On).isNull());
    QImage img4 = icon1.pixmap(128, QIcon::Normal, QIcon::On).toImage();
    QCOMPARE_NE(img4, img3);
    QCOMPARE_NE(img4, img2);
    QCOMPARE_NE(img4, img1);
    QVERIFY(!icon1.pixmap(32, QIcon::Normal, QIcon::On).isNull());
    QImage img5 = icon1.pixmap(32, QIcon::Normal, QIcon::On).toImage();
    QCOMPARE_NE(img5, img4);
    QCOMPARE_NE(img5, img3);
    QCOMPARE_NE(img5, img2);
    QCOMPARE_NE(img5, img1);

    QIcon icon2;
    icon2.addFile(prefix + "heart.svg");
    QVERIFY(icon2.pixmap(57).toImage() == icon1.pixmap(57).toImage());

    QIcon icon3(prefix + "trash.svg");
    icon3.addFile(prefix + "heart.svg");
    QVERIFY(icon3.pixmap(57).toImage() == icon1.pixmap(57).toImage());

    QIcon icon4(prefix + "heart.svg");
    icon4.addFile(prefix + "image.png", QSize(), QIcon::Active);
    QVERIFY(!icon4.pixmap(32).isNull());
    QVERIFY(!icon4.pixmap(32, QIcon::Active).isNull());
    QVERIFY(icon4.pixmap(32).toImage() == img1);
    QIcon pmIcon(pm);
    QVERIFY(icon4.pixmap(pm.size(), QIcon::Active).toImage() == pmIcon.pixmap(pm.size(), QIcon::Active).toImage());

#ifndef QT_NO_COMPRESS
    QIcon icon5(prefix + "heart.svgz");
    QVERIFY(!icon5.pixmap(32).isNull());
#endif
}

void tst_QIcon_Svg::availableSizes()
{
    {
        // checks that there are no availableSizes for scalable images.
        QIcon icon(prefix + "heart.svg");
        QList<QSize> availableSizes = icon.availableSizes();
        qDebug() << availableSizes;
        QVERIFY(availableSizes.isEmpty());
    }

    {
        // even if an a scalable image contain added pixmaps,
        // availableSizes still should be empty.
        QIcon icon(prefix + "heart.svg");
        icon.addFile(prefix + "image.png", QSize(32,32));
        QList<QSize> availableSizes = icon.availableSizes();
        QVERIFY(availableSizes.isEmpty());
    }
}

void tst_QIcon_Svg::isNull()
{
    const QString nonExistentFilePath{ prefix + "nonExistentFile.svg" };
    const QRegularExpression cannotOpenMsg{ "Cannot open file '" + nonExistentFilePath + "'" };
    {
        //checks that an invalid file results in the icon being null
        QTest::ignoreMessage(QtWarningMsg, cannotOpenMsg);
        QIcon icon(nonExistentFilePath);
        QVERIFY(icon.isNull());
    }
    {
        //valid svg, we're not null
        QIcon icon(prefix + "heart.svg");
        QVERIFY(!icon.isNull());

        // check for non null of serialized/deserialized valid icon
        QByteArray buf;
        QDataStream out(&buf, QIODevice::WriteOnly);
        out << icon;

        QIcon icon2;
        QDataStream in(buf);
        in >> icon2;
        QVERIFY(!icon2.isNull());
    }
    {
        //invalid svg, but a pixmap added means we're not null
        QTest::ignoreMessage(QtWarningMsg, cannotOpenMsg);
        QIcon icon(nonExistentFilePath);
        icon.addFile(prefix + "image.png", QSize(32,32));
        QVERIFY(!icon.isNull());
    }

}

void tst_QIcon_Svg::sizeInPercent()
{
    QIcon icon(prefix + "rect_size_100percent.svg");
    QCOMPARE(icon.actualSize(QSize(16, 8)), QSize(16, 8));
    QCOMPARE(icon.pixmap(QSize(16, 8)).size(), QSize(16, 8));

    QCOMPARE(icon.actualSize(QSize(8, 8)), QSize(8, 4));
    QCOMPARE(icon.pixmap(QSize(8, 8)).size(), QSize(8, 4));
}

void tst_QIcon_Svg::fromTheme_data()
{
    QTest::addColumn<int>("requestedSize");
    QTest::addColumn<qreal>("requestedDpr");

    QTest::newRow("22x22,dpr=1") << 22 << 1.0;
    QTest::newRow("22x22,dpr=2") << 22 << 2.0;
    QTest::newRow("22x22,dpr=3") << 22 << 3.0;

    QTest::newRow("32x32,dpr=1") << 32 << 1.0;
    QTest::newRow("32x32,dpr=2") << 32 << 2.0;
    QTest::newRow("32x32,dpr=3") << 32 << 3.0;

    QTest::newRow("40x40,dpr=1") << 40 << 1.0;
    QTest::newRow("40x40,dpr=2") << 40 << 2.0;
    QTest::newRow("40x40,dpr=3") << 40 << 3.0;
}

void tst_QIcon_Svg::fromTheme()
{
    QFETCH(int, requestedSize);
    QFETCH(qreal, requestedDpr);

    QString searchPath = QLatin1String(":/icons");
    QIcon::setThemeSearchPaths(QStringList() << searchPath);
    QCOMPARE(QIcon::themeSearchPaths().size(), 1);
    QCOMPARE(searchPath, QIcon::themeSearchPaths()[0]);

    QString themeName("testtheme");
    QIcon::setThemeName(themeName);
    QCOMPARE(QIcon::themeName(), themeName);

    QIcon heartIcon = QIcon::fromTheme("heart");
    QVERIFY(!heartIcon.isNull());
    QCOMPARE(heartIcon.availableSizes().size(), 2); // 16x16, 22x22

    const QPixmap pixmap = heartIcon.pixmap(QSize(requestedSize, requestedSize), requestedDpr);
    const auto width = pixmap.size().width();
    const auto height = pixmap.size().height();
    requestedSize *= requestedDpr;
    QVERIFY(width == requestedSize || height == requestedSize);
    QCOMPARE(pixmap.devicePixelRatio(), requestedDpr);
}


QTEST_MAIN(tst_QIcon_Svg)
#include "tst_qicon_svg.moc"
