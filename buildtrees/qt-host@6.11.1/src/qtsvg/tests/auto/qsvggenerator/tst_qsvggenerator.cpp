// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#include <QtTest/QTest>

#include <QBuffer>
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QFont>
#include <QLinearGradient>
#include <QPainterPath>
#include <QRadialGradient>

#include <qguiapplication.h>
#include <qdebug.h>
#include <qpainter.h>
#include <qsvggenerator.h>
#include <qsvgrenderer.h>

class tst_QSvgGenerator : public QObject
{
Q_OBJECT

public:
    tst_QSvgGenerator();
    virtual ~tst_QSvgGenerator();

private slots:
    void construction();
    void fileName();
    void escapesTitle();
    void escapesDescription();
    void outputDevice();
    void sizeAndViewBox();
    void metric();
    void radialGradient();
    void fileEncoding();
    void fractionalFontSize();
    void titleAndDescription();
    void gradientInterpolation();
    void patternBrush();
    void assertOnCurveToElement();
};

tst_QSvgGenerator::tst_QSvgGenerator()
{
}

tst_QSvgGenerator::~tst_QSvgGenerator()
{
    QFile::remove(QLatin1String("fileName_output.svg"));
    QFile::remove(QLatin1String("outputDevice_output.svg"));
    QFile::remove(QLatin1String("radial_gradient.svg"));
}

void tst_QSvgGenerator::construction()
{
    QSvgGenerator generator;
    QCOMPARE(generator.fileName(), QString());
    QCOMPARE(generator.outputDevice(), (QIODevice *)0);
    QCOMPARE(generator.resolution(), 72);
    QCOMPARE(generator.size(), QSize());
}

static void removeAttribute(const QDomNode &node, const QString &attribute)
{
    if (node.isNull())
        return;

    node.toElement().removeAttribute(attribute);

    removeAttribute(node.firstChild(), attribute);
    removeAttribute(node.nextSibling(), attribute);
}

static void compareWithoutFontInfo(const QByteArray &source, const QByteArray &reference)
{
    QDomDocument sourceDoc;
    sourceDoc.setContent(source);

    QDomDocument referenceDoc;
    referenceDoc.setContent(reference);

    const QString fontAttributes[] = {
        "font-family",
        "font-size",
        "font-weight",
        "font-style",
    };

    for (const QString &attribute : fontAttributes) {
        removeAttribute(sourceDoc, attribute);
        removeAttribute(referenceDoc, attribute);
    }

    QCOMPARE(sourceDoc.toByteArray(), referenceDoc.toByteArray());
}

static void checkFile(const QString &fileName)
{
    QVERIFY(QFile::exists(fileName));;

    QFile file(fileName);
    QVERIFY(file.open(QIODevice::ReadOnly));

    QFile referenceFile(QFINDTESTDATA("referenceSvgs/" + fileName));
    QVERIFY(referenceFile.open(QIODevice::ReadOnly));

    compareWithoutFontInfo(file.readAll(), referenceFile.readAll());
}

void tst_QSvgGenerator::fileName()
{
    QString fileName = "fileName_output.svg";
    QFile::remove(fileName);

    QSvgGenerator generator;
    generator.setFileName(fileName);
    QCOMPARE(generator.fileName(), fileName);

    QPainter painter(&generator);
    painter.fillRect(0, 0, 100, 100, Qt::red);
    painter.end();

    checkFile(fileName);
}

void tst_QSvgGenerator::escapesTitle()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    const QString titleThatNeedsToBeEscaped("<malicious>\"title\" 'oh no'");

    {
        QSvgGenerator generator;

        generator.setOutputDevice(&buffer);
        generator.setTitle(titleThatNeedsToBeEscaped);

        QPainter painter(&generator);
        painter.end();
    }

    QDomDocument generated;
    generated.setContent(byteArray);

    const auto titleElements = generated.documentElement().elementsByTagName("title");

    QCOMPARE(1, titleElements.size());

    const auto theOnlyTitleElement = titleElements.at(0);

    QCOMPARE(1, theOnlyTitleElement.childNodes().size());
    QCOMPARE(titleThatNeedsToBeEscaped, theOnlyTitleElement.firstChild().nodeValue());
}

void tst_QSvgGenerator::escapesDescription()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    const QString descriptionThatNeedsToBeEscaped("<evil>\"description\" 'whoopsie!'");

    {
        QSvgGenerator generator;

        generator.setOutputDevice(&buffer);
        generator.setDescription(descriptionThatNeedsToBeEscaped);

        QPainter painter(&generator);
        painter.end();
    }

    QDomDocument generated;
    generated.setContent(byteArray);

    const auto descriptionElements = generated.documentElement().elementsByTagName("desc");

    QCOMPARE(1, descriptionElements.size());

    const auto theOnlyDescriptionElement = descriptionElements.at(0);

    QCOMPARE(1, theOnlyDescriptionElement.childNodes().size());
    QCOMPARE(descriptionThatNeedsToBeEscaped, theOnlyDescriptionElement.firstChild().nodeValue());
}

void tst_QSvgGenerator::outputDevice()
{
    const QString fileName = "outputDevice_output.svg";
    const QByteArray notOpenWarning =
            QString("QIODevice::write (QFile, \"%1\"): device not open").arg(fileName).toLatin1();
    QFile::remove(fileName);

    QFile file(fileName);

    {
        // Device is not open
        QSvgGenerator generator;
        generator.setOutputDevice(&file);
        QCOMPARE(generator.outputDevice(), (QIODevice *)&file);

        QPainter painter;
        QTest::ignoreMessage(QtWarningMsg, notOpenWarning.constData());
        QVERIFY(painter.begin(&generator));
        QCOMPARE(file.openMode(), QIODevice::OpenMode(QIODevice::Text | QIODevice::WriteOnly));
        file.close();
    }
    {
        // Device is not open, WriteOnly
        QVERIFY(file.open(QIODevice::WriteOnly));

        QSvgGenerator generator;
        generator.setOutputDevice(&file);
        QCOMPARE(generator.outputDevice(), (QIODevice *)&file);

        QPainter painter;
        QTest::ignoreMessage(QtWarningMsg, notOpenWarning.constData());
        QVERIFY(painter.begin(&generator));
        QCOMPARE(file.openMode(), QIODevice::OpenMode(QIODevice::WriteOnly));
        file.close();
    }
    {
        // Device is not open, ReadOnly
        QVERIFY(file.open(QIODevice::ReadOnly));

        QSvgGenerator generator;
        generator.setOutputDevice(&file);
        QCOMPARE(generator.outputDevice(), (QIODevice *)&file);

        QPainter painter;
        QTest::ignoreMessage(QtWarningMsg, "QSvgPaintEngine::begin(), could not write to read-only output device: 'Unknown error'");
        QTest::ignoreMessage(QtWarningMsg, "QPainter::begin(): Returned false");
        QVERIFY(!painter.begin(&generator));
        QCOMPARE(file.openMode(), QIODevice::OpenMode(QIODevice::ReadOnly));
        file.close();
    }
}

void tst_QSvgGenerator::sizeAndViewBox()
{
    { // Setting neither properties should result in
      // none of the attributes written to the SVG
        QSvgGenerator generator;
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        generator.setOutputDevice(&buffer);
        QPainter painter(&generator);
        painter.end();

        QVERIFY(!byteArray.contains("<svg width=\""));
        QVERIFY(!byteArray.contains("viewBox=\""));
    }

    { // Setting size only should write size only
        QSvgGenerator generator;
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        generator.setOutputDevice(&buffer);
        generator.setResolution(254);
        generator.setSize(QSize(100, 100));
        QPainter painter(&generator);
        painter.end();

        QVERIFY(byteArray.contains("<svg width=\"10mm\" height=\"10mm\""));
        QVERIFY(!byteArray.contains("viewBox=\""));
    }

    { // Setting viewBox only should write viewBox only
        QSvgGenerator generator;
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        generator.setOutputDevice(&buffer);
        generator.setViewBox(QRectF(20, 20, 50.666, 50.666));
        QPainter painter(&generator);
        painter.end();

        QVERIFY(!byteArray.contains("<svg width=\""));
        QVERIFY(byteArray.contains("<svg viewBox=\"20 20 50.666 50.666\""));
    }

    { // Setting both properties should result in
      // both of the attributes written to the SVG
        QSvgGenerator generator;
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        generator.setOutputDevice(&buffer);
        generator.setResolution(254);
        generator.setSize(QSize(500, 500));
        generator.setViewBox(QRectF(20.666, 20.666, 50, 50));
        QPainter painter(&generator);
        painter.end();

        QVERIFY(byteArray.contains("<svg width=\"50mm\" height=\"50mm\""));
        QVERIFY(byteArray.contains("viewBox=\"20.666 20.666 50 50\""));
    }
}

void tst_QSvgGenerator::metric()
{
    QSvgGenerator generator;
    generator.setSize(QSize(100, 100));
    generator.setResolution(254); // 254 dots per inch == 10 dots per mm

    QCOMPARE(generator.widthMM(), 10);
    QCOMPARE(generator.heightMM(), 10);
}

void tst_QSvgGenerator::radialGradient()
{
    QString fileName = "radial_gradient.svg";
    QFile::remove(fileName);

    QSvgGenerator generator;
    generator.setSize(QSize(200, 100));
    generator.setFileName(fileName);
    QCOMPARE(generator.fileName(), fileName);

    QRadialGradient gradient(QPointF(0.5, 0.5), 0.5, QPointF(0.5, 0.5), 0);
    gradient.setInterpolationMode(QGradient::ComponentInterpolation);
    gradient.setColorAt(0, Qt::red);
    gradient.setColorAt(1, Qt::blue);
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);

    QPainter painter(&generator);
    painter.fillRect(0, 0, 100, 100, gradient);

    gradient = QRadialGradient(QPointF(150, 50), 50, QPointF(150, 50), 0);
    gradient.setInterpolationMode(QGradient::ComponentInterpolation);
    gradient.setColorAt(0, Qt::red);
    gradient.setColorAt(1, Qt::blue);
    painter.fillRect(100, 0, 100, 100, gradient);
    painter.end();

    checkFile(fileName);
}

void tst_QSvgGenerator::fileEncoding()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    QSvgGenerator generator;
    generator.setOutputDevice(&buffer);

    static const QChar unicode[] = { 'f', 'o', 'o',
            u'\u00F8', 'b', 'a', 'r'};

    int size = sizeof(unicode) / sizeof(QChar);
    QString unicodeString = QString::fromRawData(unicode, size);

    QPainter painter(&generator);
    painter.drawText(100, 100, unicodeString);
    painter.end();

    QVERIFY(byteArray.contains(unicodeString.toUtf8()));
}

void tst_QSvgGenerator::fractionalFontSize()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    QSvgGenerator generator;
    generator.setResolution(72);
    generator.setOutputDevice(&buffer);

    QPainter painter(&generator);
    QFont fractionalFont = painter.font();
    fractionalFont.setPointSizeF(7.5);
    painter.setFont(fractionalFont);
    painter.drawText(100, 100, "foo");
    painter.end();

    QVERIFY(byteArray.contains("7.5"));
}

void tst_QSvgGenerator::titleAndDescription()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    QSvgGenerator generator;
    generator.setTitle("foo");
    QCOMPARE(generator.title(), QString("foo"));
    generator.setDescription("bar");
    QCOMPARE(generator.description(), QString("bar"));
    generator.setOutputDevice(&buffer);

    QPainter painter(&generator);
    painter.end();

    QVERIFY(byteArray.contains("<title>foo</title>"));
    QVERIFY(byteArray.contains("<desc>bar</desc>"));
}

static void drawTestGradients(QPainter &painter)
{
    int w = painter.device()->width();
    int h = painter.device()->height();
    if (w <= 0 || h <= 0)
        h = w = 72;

    QLinearGradient gradient(QPoint(0, 0), QPoint(1, 1));
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    gradient.setColorAt(0, QColor(255, 0, 0, 0));
    gradient.setColorAt(1, QColor(0, 0, 255, 255));
    painter.fillRect(QRectF(0, 0, w/2, h/2), gradient);

    gradient.setInterpolationMode(QGradient::ComponentInterpolation);
    painter.fillRect(QRectF(0, h/2, w/2, h - h/2), gradient);

    gradient.setInterpolationMode(QGradient::ColorInterpolation);
    gradient.setColorAt(0, QColor(255, 0, 0, 123));
    gradient.setColorAt(1, QColor(0, 0, 255, 123));
    painter.fillRect(QRectF(w/2, 0, w - w/2, h/2), gradient);

    gradient.setInterpolationMode(QGradient::ComponentInterpolation);
    painter.fillRect(QRectF(w/2, h/2, w - w/2, h - h/2), gradient);
}

static qreal sqrImageDiff(const QImage &image1, const QImage &image2)
{
    if (image1.size() != image2.size())
        return 1e30;
    quint64 sum = 0;
    for (int y = 0; y < image1.height(); ++y) {
        const quint8 *line1 = reinterpret_cast<const quint8 *>(image1.scanLine(y));
        const quint8 *line2 = reinterpret_cast<const quint8 *>(image2.scanLine(y));
        for (int x = 0; x < image1.width() * 4; ++x)
            sum += quint64((int(line1[x]) - int(line2[x])) * (int(line1[x]) - int(line2[x])));
    }
    return qreal(sum) / qreal(image1.width() * image1.height());
}

void tst_QSvgGenerator::gradientInterpolation()
{
    QByteArray byteArray;
    QPainter painter;
    QImage image(576, 576, QImage::Format_ARGB32_Premultiplied);
    QImage refImage(576, 576, QImage::Format_ARGB32_Premultiplied);
    image.fill(0x80208050);
    refImage.fill(0x80208050);

    {
        QSvgGenerator generator;
        QBuffer buffer(&byteArray);
        generator.setOutputDevice(&buffer);

        QVERIFY(painter.begin(&generator));
        drawTestGradients(painter);
        painter.end();
    }

    {
        QVERIFY(painter.begin(&image));
        QSvgRenderer renderer(byteArray);
        renderer.render(&painter, image.rect());
        painter.end();
    }

    {
        QVERIFY(painter.begin(&refImage));
        drawTestGradients(painter);
        painter.end();
    }

    QVERIFY(sqrImageDiff(image, refImage) < 2); // pixel error < 1.41 (L2-norm)
}

void tst_QSvgGenerator::patternBrush()
{
    { // Pattern brush should create mask and pattern used as fill
        QSvgGenerator generator;
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        generator.setOutputDevice(&buffer);
        QPainter painter(&generator);
        painter.setBrush(Qt::CrossPattern);
        painter.drawRect(0, 0, 100, 100);
        painter.end();

        QVERIFY(byteArray.contains("<mask id=\"patternmask"));
        QVERIFY(byteArray.contains("<pattern id=\"fillpattern"));
        QVERIFY(byteArray.contains("<g fill=\"url(#fillpattern"));
    }

    { // Masks and patterns should be reused, not regenerated
        QSvgGenerator generator;
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        generator.setOutputDevice(&buffer);
        QPainter painter(&generator);
        painter.setBrush(QBrush(Qt::red, Qt::Dense3Pattern));
        painter.drawRect(0, 0, 100, 100);
        painter.drawEllipse(200, 50, 50, 50);
        painter.setBrush(QBrush(Qt::green, Qt::Dense3Pattern));
        painter.drawRoundedRect(0, 200, 100, 100, 10, 10);
        painter.setBrush(QBrush(Qt::blue, Qt::Dense4Pattern));
        painter.drawRect(200, 200, 100, 100);
        painter.setBrush(QBrush(Qt::red, Qt::Dense3Pattern));
        painter.drawRoundedRect(120, 120, 60, 60, 5, 5);
        painter.end();

        QCOMPARE(byteArray.count("<mask id=\"patternmask"), 2);
        QCOMPARE(byteArray.count("<pattern id=\"fillpattern"), 3);
        QVERIFY(byteArray.count("<g fill=\"url(#fillpattern") >= 4);
    }

}

void tst_QSvgGenerator::assertOnCurveToElement()
{
    // QTBUG-145372
    // Tried to access QPainterPath beyond its size when closing path, needs enabled asserts
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    QSvgGenerator svg;
    svg.setOutputDevice(&buffer);
    QPainter painter(&svg);
    QPainterPath path;
    path.moveTo(100, 0);
    path.cubicTo(100, 55, 55, 100, 0, 100);
    path.cubicTo(55, -100, 100, -55, 100, 0);
    painter.drawPath(path);
    painter.end();
    // Validate that path is still closed correctly
    QVERIFY(byteArray.contains("M100,0 C100,55 55,100 0,100 C55,-100 100,-55 100,0 Z"));
}

QTEST_MAIN(tst_QSvgGenerator)
#include "tst_qsvggenerator.moc"
