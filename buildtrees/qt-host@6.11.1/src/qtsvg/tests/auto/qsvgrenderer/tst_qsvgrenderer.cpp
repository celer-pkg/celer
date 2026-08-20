// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only


#include <QtTest/QTest>

#include <qguiapplication.h>
#include <qdebug.h>
#include <qsvgrenderer.h>
#include <qsvggenerator.h>
#include <QBuffer>
#include <QByteArray>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPen>
#include <QPicture>
#include <QRgb>
#include <QRectF>
#include <QTimer>
#include <QXmlStreamReader>

using namespace Qt::Literals::StringLiterals;

class tst_QSvgRenderer : public QObject
{
Q_OBJECT

public:
    tst_QSvgRenderer();
    virtual ~tst_QSvgRenderer();

private slots:
    void getSetCheck();
    void emptyRect_data();
    void emptyRect();
    void inexistentUrl();
    void emptyUrl();
    void invalidUrl_data();
    void invalidUrl();
    void testStrokeWidth();
#if QT_CONFIG(picture)
    void testMapViewBoxToTarget();
    void testRenderElement();
#endif
    void testRenderElementToBounds();
    void testRenderDocumentWithSizeToBounds();
#if QT_CONFIG(picture)
    void constructorQXmlStreamReader() const;
    void loadQXmlStreamReader() const;
    void nestedQXmlStreamReader() const;
#endif
    void stylePropagation() const;
    void transformForElement() const;
    void boundsOnElement() const;
    void gradientStops() const;
    void gradientRefs();
    void recursiveRefs_data();
    void recursiveRefs();
    void fillRule();
    void opacity();
    void paths();
    void paths2();
    void emptyPath_data();
    void emptyPath();
    void displayMode();
    void strokeInherit();
    void testFillInheritance();
    void testStopOffsetOpacity();
    void testUseElement();
    void smallFont();
    void glyphWarnings_data();
    void glyphWarnings();
    void styleSheet();
    void duplicateStyleId();
    void ossFuzzRender_data();
    void ossFuzzRender();
    void ossFuzzLoad_data();
    void ossFuzzLoad();
    void imageRendering();
    void imageMalformedDataUrl();
    void illegalAnimateTransform_data();
    void illegalAnimateTransform();
    void tSpanLineBreak();
    void animated();
    void notAnimated();
    void notAnimatedOption();
    void testMaskElement();
    void testSymbol();
    void testMarker();
    void testPatternElement();
    void testMisplacedElement();
    void testCycles_data();
    void testCycles();
    void testFeFlood();
    void testFeOffset();
    void testFeColorMatrix();
    void testFeMerge();
    void testFeComposite();
    void testFeGaussian();
    void testFeBlend();

    void testOption_data();
    void testOption();

#ifndef QT_NO_COMPRESS
    void testGzLoading();
#  ifdef QT_BUILD_INTERNAL
    void testGzHelper_data();
    void testGzHelper();
#  endif
#endif

private:
    static const char *const src;
};

const char *const tst_QSvgRenderer::src = "<svg><g><rect x='250' y='250' width='500' height='500'/>"
                                          "<rect id='foo' x='400' y='400' width='100' height='100'/></g></svg>";

tst_QSvgRenderer::tst_QSvgRenderer()
{
}

tst_QSvgRenderer::~tst_QSvgRenderer()
{
}

// Testing get/set functions
void tst_QSvgRenderer::getSetCheck()
{
    QSvgRenderer obj1;
    // int QSvgRenderer::framesPerSecond()
    // void QSvgRenderer::setFramesPerSecond(int)
    obj1.setFramesPerSecond(20);
    QCOMPARE(20, obj1.framesPerSecond());
    obj1.setFramesPerSecond(0);
    QCOMPARE(0, obj1.framesPerSecond());
    QTest::ignoreMessage(QtWarningMsg,
                         "QSvgRenderer::setFramesPerSecond: Cannot set negative value -2147483648");
    obj1.setFramesPerSecond(INT_MIN);
    QCOMPARE(0, obj1.framesPerSecond()); // Can't have a negative framerate
    obj1.setFramesPerSecond(INT_MAX);
    QCOMPARE(INT_MAX, obj1.framesPerSecond());

    // bool QSvgRenderer::isAnimationEnabled()
    // void QSvgRenderer::setAnimationEnabled()
    QVERIFY(obj1.isAnimationEnabled());
    obj1.setAnimationEnabled(false);
    QVERIFY(!obj1.isAnimationEnabled());
    obj1.setAnimationEnabled(true);
    QVERIFY(obj1.isAnimationEnabled());
}

void tst_QSvgRenderer::emptyRect_data()
{
    // Testing rects with zero width or height or no given value
    // Those caused divisions by zero, e.g. QTBUG-49160 and oss-fuzz issue 23588
    // UBSAN is required to see those divisions by zero
    QTest::addColumn<QByteArray>("svg");
    QTest::newRow("nothing") << R"(<svg><rect/></svg>)"_ba;
    QTest::newRow("no width zero height") << R"(<svg><rect height="0"/></svg>)"_ba;
    QTest::newRow("zero width no height") << R"(<svg><rect width="0"/></svg>)"_ba;
    QTest::newRow("no width") << R"(<svg><rect height="1"/></svg>)"_ba;
    QTest::newRow("no height") << R"(<svg><rect width="1"/></svg>)"_ba;
    QTest::newRow("both zero") << R"(<svg><rect width="0" height="0"/></svg>)"_ba;
    QTest::newRow("zero width") << R"(<svg><rect width="0" height="1"/></svg>)"_ba;
    QTest::newRow("zero heigth") << R"(<svg><rect width="1" height="0"/></svg>)"_ba;
}

void tst_QSvgRenderer::emptyRect()
{
    QFETCH(QByteArray, svg);
    QSvgRenderer renderer(svg);
    QVERIFY(renderer.isValid());
}

void tst_QSvgRenderer::inexistentUrl()
{
    const char *src = "<svg><g><path d=\"M0 0\" style=\"stroke:url(#inexistent)\"/></g></svg>";
    QTest::ignoreMessage(QtWarningMsg, "<input>:1:66: Could not resolve property: #inexistent");

    QByteArray data(src);
    QSvgRenderer renderer(data);

    QVERIFY(renderer.isValid());
}

void tst_QSvgRenderer::emptyUrl()
{
    const char *src = "<svg><text fill=\"url()\" /></svg>";
    QTest::ignoreMessage(QtWarningMsg, "<input>:1:32: Could not resolve property: ");

    QByteArray data(src);
    QSvgRenderer renderer(data);

    QVERIFY(renderer.isValid());
}

void tst_QSvgRenderer::invalidUrl_data()
{
    QTest::addColumn<QByteArray>("svg");

    QTest::newRow("url0")
            << R"(<svg><linearGradient id="0"/><circle fill="url0" /></svg>)"_ba;
    QTest::newRow("url(0")
            << R"(<svg><linearGradient id="0"/><circle fill="url(0" /></svg>)"_ba;
    QTest::newRow("url (0")
            << R"(<svg><linearGradient id="0"/><circle fill="url (0" /></svg>)"_ba;
    QTest::newRow("url ( 0")
            << R"(<svg><linearGradient id="0"/><circle fill="url ( 0" /></svg>)"_ba;
    QTest::newRow("url#")
            << R"(<svg><linearGradient id="0"/><circle fill="url#" /></svg>)"_ba;
    QTest::newRow("url#(")
            << R"(<svg><linearGradient id="0"/><circle fill="url#(" /></svg>)"_ba;
    QTest::newRow("url(#")
            << R"(<svg><linearGradient id="0"/><circle fill="url(#" /></svg>)"_ba;
    QTest::newRow("url(# ")
            << R"(<svg><linearGradient id="0"/><circle fill="url(# " /></svg>)"_ba;
    QTest::newRow("url(# 0")
            << R"(<svg><linearGradient id="0"/><circle fill="url(# 0" /></svg>)"_ba;
    QTest::newRow("urlblabla")
            << R"(<svg><linearGradient id="blabla"/><circle fill="urlblabla" /></svg>)"_ba;
    QTest::newRow("url(blabla")
            << R"(<svg><linearGradient id="blabla"/><circle fill="url(blabla" /></svg>)"_ba;
    QTest::newRow("url(blabla) ")
            << R"(<svg><linearGradient id="blabla"/><circle fill="url(blabla) "/></svg>)"_ba;
    QTest::newRow("url(#blabla")
            << R"(<svg><linearGradient id="blabla"/><circle fill="url(#blabla" /></svg>)"_ba;
}

void tst_QSvgRenderer::invalidUrl()
{
    QFETCH(QByteArray, svg);

#if QT_CONFIG(regularexpression)
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Could not resolve property"));
#endif
    QSvgRenderer renderer(svg);
    QVERIFY(renderer.isValid());
}

void tst_QSvgRenderer::testStrokeWidth()
{
    qreal squareSize = 30.0;
    qreal strokeWidth = 1.0;
    qreal topLeft = 100.0;

    QSvgGenerator generator;

    QBuffer buffer;
    QByteArray byteArray;
    buffer.setBuffer(&byteArray);
    generator.setOutputDevice(&buffer);

    QPainter painter(&generator);
    painter.setBrush(Qt::blue);

    // Draw a rect with stroke
    painter.setPen(QPen(Qt::black, strokeWidth));
    painter.drawRect(topLeft, topLeft, squareSize, squareSize);

    // Draw a rect without stroke
    painter.setPen(Qt::NoPen);
    painter.drawRect(topLeft, topLeft, squareSize, squareSize);
    painter.end();

    // Insert ID tags into the document
    byteArray.insert(byteArray.indexOf("stroke=\"#000000\""), "id=\"SquareStroke\" ");
    byteArray.insert(byteArray.indexOf("stroke=\"none\""), "id=\"SquareNoStroke\" ");

    QSvgRenderer renderer(byteArray);

    QRectF noStrokeRect = renderer.boundsOnElement("SquareNoStroke");
    QCOMPARE(noStrokeRect.width(), squareSize);
    QCOMPARE(noStrokeRect.height(), squareSize);
    QCOMPARE(noStrokeRect.x(), topLeft);
    QCOMPARE(noStrokeRect.y(), topLeft);

    QRectF strokeRect = renderer.boundsOnElement("SquareStroke");
    QCOMPARE(strokeRect.width(), squareSize + strokeWidth);
    QCOMPARE(strokeRect.height(), squareSize + strokeWidth);
    QCOMPARE(strokeRect.x(), topLeft - (strokeWidth / 2));
    QCOMPARE(strokeRect.y(), topLeft - (strokeWidth / 2));
}

#if QT_CONFIG(picture)
void tst_QSvgRenderer::testMapViewBoxToTarget()
{
    const char *src = R"(<svg><g><rect x="250" y="250" width="500" height="500" /></g></svg>)";
    QByteArray data(src);

    { // No viewport, viewBox, targetRect, or deviceRect -> boundingRect
        QPicture picture;
        QPainter painter(&picture);
        QSvgRenderer rend(data);
        rend.render(&painter);
        painter.end();
        QCOMPARE(picture.boundingRect(), QRect(0, 0, 500, 500));
    }

    { // No viewport, viewBox, targetRect -> deviceRect
        QPicture picture;
        picture.setBoundingRect(QRect(100, 100, 200, 200));
        QPainter painter(&picture);
        QSvgRenderer rend(data);
        rend.render(&painter);
        painter.end();
        QCOMPARE(picture.boundingRect(), QRect(100, 100, 200, 200));
    }

    { // No viewport, viewBox -> targetRect
        QPicture picture;
        QPainter painter(&picture);
        QSvgRenderer rend(data);
        rend.render(&painter, QRectF(50, 50, 250, 250));
        painter.end();
        QCOMPARE(picture.boundingRect(), QRect(50, 50, 250, 250));

    }

    data.replace("<svg>", "<svg viewBox=\"0 0 1000 1000\">");

    { // No viewport, no targetRect -> viewBox
        QPicture picture;
        QPainter painter(&picture);
        QSvgRenderer rend(data);
        rend.render(&painter);
        painter.end();
        QCOMPARE(picture.boundingRect(), QRect(250, 250, 500, 500));
    }

    data.replace("<svg", "<svg width=\"500\" height=\"500\"");

    { // Viewport
        QPicture picture;
        QPainter painter(&picture);
        QSvgRenderer rend(data);
        rend.render(&painter);
        painter.end();
        QCOMPARE(picture.boundingRect(), QRect(125, 125, 250, 250));
    }

    // Requires keep-aspectratio feature
    { // Viewport and viewBox specified -> scale 500x500 square to 1000x750 while preserving aspect ratio gives 750x750
        data = R"(<svg width="1000" height="750" viewBox="-250 -250 500 500"><g><rect x="0" y="0" width="500" height="500" /></g></svg>)";
        QPicture picture;
        QPainter painter(&picture);
        QSvgRenderer rend(data);
        rend.setAspectRatioMode(Qt::KeepAspectRatio);
        rend.render(&painter);
        painter.end();
        QCOMPARE(picture.boundingRect(), QRect(500, 375, 750, 750));
    }
}

void tst_QSvgRenderer::testRenderElement()
{
    QByteArray data(src);

    { // No viewport, viewBox, targetRect, or deviceRect -> boundingRect
        QPicture picture;
        QPainter painter(&picture);
        QSvgRenderer rend(data);
        rend.render(&painter, "foo"_L1);
        painter.end();
        QCOMPARE(picture.boundingRect(), QRect(0, 0, 100, 100));
    }

    { // No viewport, viewBox, targetRect -> deviceRect
        QPicture picture;
        picture.setBoundingRect(QRect(100, 100, 200, 200));
        QPainter painter(&picture);
        QSvgRenderer rend(data);
        rend.render(&painter, "foo"_L1);
        painter.end();
        QCOMPARE(picture.boundingRect(), QRect(100, 100, 200, 200));
    }

    { // No viewport, viewBox -> targetRect
        QPicture picture;
        QPainter painter(&picture);
        QSvgRenderer rend(data);
        rend.render(&painter, "foo"_L1, QRectF(50, 50, 250, 250));
        painter.end();
        QCOMPARE(picture.boundingRect(), QRect(50, 50, 250, 250));

    }

    data.replace("<svg>", "<svg viewBox=\"0 0 1000 1000\">");

    { // No viewport, no targetRect -> view box size
        QPicture picture;
        QPainter painter(&picture);
        QSvgRenderer rend(data);
        rend.render(&painter, "foo"_L1);
        painter.end();
        QCOMPARE(picture.boundingRect(), QRect(0, 0, 100, 100));
    }

    data.replace("<svg", "<svg width=\"500\" height=\"500\"");

    { // Viewport
        QPicture picture;
        QPainter painter(&picture);
        QSvgRenderer rend(data);
        rend.render(&painter, "foo"_L1);
        painter.end();
        QCOMPARE(picture.boundingRect(), QRect(0, 0, 100, 100));
    }

}
#endif

void tst_QSvgRenderer::testRenderElementToBounds()
{
    // QTBUG-79933
    QImage reference(400, 200, QImage::Format_ARGB32);
    {
        reference.fill(Qt::transparent);
        QPainter p(&reference);
        p.fillRect(  0,   0, 200, 100, Qt::blue);
        p.fillRect(200, 100, 200, 100, Qt::blue);
        p.fillRect(  0,   0, 100,  50, Qt::red);
        p.fillRect(100,  50, 100,  50, Qt::red);
        p.fillRect(200, 100, 100,  50, Qt::red);
        p.fillRect(300, 150, 100,  50, Qt::red);
    }

    QImage rendering(400, 200, QImage::Format_ARGB32);
    {
        const char *const src =
                "<svg viewBox=\"0 0 100 100\">" // Presence of a viewBox triggered QTBUG-79933
                "<path id=\"el1\" d=\"M 80,10 H 0 V 0 h 40 v 20 h 40 z\" fill=\"red\" />"
                "<path id=\"el2\" d=\"M 90,100 V 20 h 10 V 60 H 80 v 40 z\" fill=\"blue\" />"
                "</svg>";
        const QByteArray data(src);
        QSvgRenderer rend(data);
        rendering.fill(Qt::transparent);
        QPainter p(&rendering);
        rend.render(&p, "el1", QRectF(  0,   0, 200, 100));
        rend.render(&p, "el2", QRectF(  0,   0, 200, 100));
        rend.render(&p, "el1", QRectF(200, 100, 200, 100));
        rend.render(&p, "el2", QRectF(200, 100, 200, 100));
    }

    QCOMPARE(reference, rendering);
}

void tst_QSvgRenderer::testRenderDocumentWithSizeToBounds()
{
    // QTBUG-80888
    QImage reference(400, 200, QImage::Format_ARGB32);
    {
        reference.fill(Qt::transparent);
        QPainter p(&reference);
        p.fillRect(100, 100, 100,  50, Qt::blue);
        p.fillRect(200,  50, 100,  50, Qt::blue);
    }

    QImage rendering(400, 200, QImage::Format_ARGB32);
    {
        const char *const src = R"src(
        <svg width="20" height="80">
            <g transform="translate(-100,-100)">
                <path d="m 110,180 v -80 h 10 v 40 h -20 v 40 z" fill="blue" />
            </g>
        </svg>
        )src";
        const QByteArray data(src);
        QSvgRenderer rend(data);
        rendering.fill(Qt::transparent);
        QPainter p(&rendering);
        rend.render(&p, QRectF(100, 50, 200, 100));
    }

    QCOMPARE(reference, rendering);
}

#if QT_CONFIG(picture)
void tst_QSvgRenderer::constructorQXmlStreamReader() const
{
    const QByteArray data(src);

    QXmlStreamReader reader(data);

    QPicture picture;
    QPainter painter(&picture);
    QSvgRenderer rend(&reader);
    rend.render(&painter, "foo"_L1);
    painter.end();
    QCOMPARE(picture.boundingRect(), QRect(0, 0, 100, 100));
}

void tst_QSvgRenderer::loadQXmlStreamReader() const
{
    const QByteArray data(src);

    QXmlStreamReader reader(data);
    QPicture picture;
    QPainter painter(&picture);
    QSvgRenderer rend;
    rend.load(&reader);
    rend.render(&painter, "foo"_L1);
    painter.end();
    QCOMPARE(picture.boundingRect(), QRect(0, 0, 100, 100));
}

void tst_QSvgRenderer::nestedQXmlStreamReader() const
{
    const QByteArray data(QByteArray("<bar>") + QByteArray(src) + QByteArray("</bar>"));

    QXmlStreamReader reader(data);

    QCOMPARE(reader.readNext(), QXmlStreamReader::StartDocument);
    QCOMPARE(reader.readNext(), QXmlStreamReader::StartElement);
    QCOMPARE(reader.name().toString(), "bar"_L1);

    QPicture picture;
    QPainter painter(&picture);
    QSvgRenderer renderer(&reader);
    renderer.render(&painter, "foo"_L1);
    painter.end();
    QCOMPARE(picture.boundingRect(), QRect(0, 0, 100, 100));

    QCOMPARE(reader.readNext(), QXmlStreamReader::EndElement);
    QCOMPARE(reader.name().toString(), "bar"_L1);
    QCOMPARE(reader.readNext(), QXmlStreamReader::EndDocument);

    QVERIFY(reader.atEnd());
    QVERIFY(!reader.hasError());
}
#endif

void tst_QSvgRenderer::stylePropagation() const
{
#if !QT_CONFIG(cssparser)
    QSKIP("cssparser feature disabled, skipping test");
#else
    QByteArray data(R"(<svg>
                      <g id="foo" style="fill:#ffff00;">
                        <g id="bar" style="fill:#ff00ff;">
                          <g id="baz" style="fill:#00ffff;">
                            <rect id="alpha" x="0" y="0" width="100" height="100"/>
                          </g>
                          <rect id="beta" x="100" y="0" width="100" height="100"/>
                        </g>
                        <rect id="gamma" x="0" y="100" width="100" height="100"/>
                      </g>
                      <rect id="delta" x="100" y="100" width="100" height="100"/>
                    </svg>)"); // alpha=cyan, beta=magenta, gamma=yellow, delta=black

    QImage image1(200, 200, QImage::Format_RGB32);
    QImage image2(200, 200, QImage::Format_RGB32);
    QImage image3(200, 200, QImage::Format_RGB32);
    QPainter painter;
    QSvgRenderer renderer(data);
    QLatin1StringView parts[4] = {"alpha"_L1, "beta"_L1, "gamma"_L1, "delta"_L1};

    QVERIFY(painter.begin(&image1));
    for (int i = 0; i < 4; ++i)
        renderer.render(&painter, parts[i], QRectF(renderer.boundsOnElement(parts[i])));
    painter.end();

    QVERIFY(painter.begin(&image2));
    renderer.render(&painter, renderer.viewBoxF());
    painter.end();

    QVERIFY(painter.begin(&image3));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(Qt::cyan));
    painter.drawRect(0, 0, 100, 100);
    painter.setBrush(QBrush(Qt::magenta));
    painter.drawRect(100, 0, 100, 100);
    painter.setBrush(QBrush(Qt::yellow));
    painter.drawRect(0, 100, 100, 100);
    painter.setBrush(QBrush(Qt::black));
    painter.drawRect(100, 100, 100, 100);
    painter.end();

    QCOMPARE(image1, image2);
    QCOMPARE(image1, image3);
#endif // QT_CONFIG(cssparser)
}

static qreal transformNorm(const QTransform &m)
{
    return qSqrt(m.m11() * m.m11()
        + m.m12() * m.m12()
        + m.m13() * m.m13()
        + m.m21() * m.m21()
        + m.m22() * m.m22()
        + m.m23() * m.m23()
        + m.m31() * m.m31()
        + m.m32() * m.m32()
        + m.m33() * m.m33());
}

template<typename T>
static inline bool diffIsSmallEnough(T diff, T norm)
{
    static_assert(std::is_same_v<T, double> || std::is_same_v<T, float>);
    T sigma = []{
        if constexpr (std::is_same_v<T, double>)
            return 1e-12;
        else
            return 1e-5;
    }();
    return diff <= sigma * norm;
}

static void compareTransforms(const QTransform &m1, const QTransform &m2)
{
    qreal norm1 = transformNorm(m1);
    qreal norm2 = transformNorm(m2);
    qreal diffNorm = transformNorm(QTransform(m1.m11() - m2.m11(),
                                              m1.m12() - m2.m12(),
                                              m1.m13() - m2.m13(),
                                              m1.m21() - m2.m21(),
                                              m1.m22() - m2.m22(),
                                              m1.m23() - m2.m23(),
                                              m1.m31() - m2.m31(),
                                              m1.m32() - m2.m32(),
                                              m1.m33() - m2.m33()));
    QVERIFY(diffIsSmallEnough(diffNorm, qMin(norm1, norm2)));
}

void tst_QSvgRenderer::transformForElement() const
{
    QByteArray data(R"(<svg>
                      <g id="ichi" transform="translate(-3,1) ">
                        <g id="ni" transform="rotate(45) ">
                          <g id="san" transform="scale(4,2) ">
                            <g id="yon" transform="matrix(1,2,3,4,5,6) ">
                              <rect id="firkant" x="-1" y="-1" width="2" height="2"/>
                            </g>
                          </g>
                        </g>
                      </g>
                    </svg>)");

    QImage image(13, 37, QImage::Format_RGB32);
    QPainter painter(&image);
    QSvgRenderer renderer(data);

    compareTransforms(painter.worldTransform(), renderer.transformForElement("ichi"_L1));
    painter.translate(-3, 1);
    compareTransforms(painter.worldTransform(), renderer.transformForElement("ni"_L1));
    painter.rotate(45);
    compareTransforms(painter.worldTransform(), renderer.transformForElement("san"_L1));
    painter.scale(4, 2);
    compareTransforms(painter.worldTransform(), renderer.transformForElement("yon"_L1));
    painter.setWorldTransform(QTransform(1, 2, 3, 4, 5, 6), true);
    compareTransforms(painter.worldTransform(), renderer.transformForElement("firkant"_L1));
}

void tst_QSvgRenderer::boundsOnElement() const
{
    QByteArray data(R"(<svg>
                      <g id="prim" transform="translate(-3,1) ">
                        <g id="sjokade" stroke="none" stroke-width="10">
                          <rect id="kaviar" transform="rotate(45) " x="-10" y="-10" width="20" height="20"/>
                        </g>
                        <g id="nugatti" stroke="black" stroke-width="2">
                          <use x="0" y="0" transform="rotate(45) " xlink:href="#kaviar"/>
                        </g>
                        <g id="nutella" stroke="none" stroke-width="10">
                          <path id="baconost" transform="rotate(45) " d="M-10 -10 L10 -10 L10 10 L-10 10 Z"/>
                        </g>
                        <g id="hapaa" transform="translate(-2,2) " stroke="black" stroke-width="2">
                          <use x="0" y="0" transform="rotate(45) " xlink:href="#baconost"/>
                        </g>
                      </g>
                      <text id="textA" x="50" y="100">Lorem ipsum</text>
                      <text id="textB" transform="matrix(1 0 0 1 50 100) ">Lorem ipsum</text>
                      <g id="textGroup">
                        <text id="textC" transform="matrix(1 0 0 2 20 10) ">Lorem ipsum</text>
                        <text id="textD" transform="matrix(1 0 0 2 30 40) ">Lorem ipsum</text>
                      </g>
                    </svg>)");

    qreal sqrt2 = qSqrt(2);
    QSvgRenderer renderer(data);
    QCOMPARE(renderer.boundsOnElement("sjokade"_L1),
             QRectF(-10 * sqrt2, -10 * sqrt2, 20 * sqrt2, 20 * sqrt2));
    QCOMPARE(renderer.boundsOnElement("kaviar"_L1),
             QRectF(-10 * sqrt2, -10 * sqrt2, 20 * sqrt2, 20 * sqrt2));
    QCOMPARE(renderer.boundsOnElement("nugatti"_L1),
             QRectF(-11, -11, 22, 22));
    QCOMPARE(renderer.boundsOnElement("nutella"_L1),
             QRectF(-10 * sqrt2, -10 * sqrt2, 20 * sqrt2, 20 * sqrt2));
    QCOMPARE(renderer.boundsOnElement("baconost"_L1),
             QRectF(-10 * sqrt2, -10 * sqrt2, 20 * sqrt2, 20 * sqrt2));
    QCOMPARE(renderer.boundsOnElement("hapaa"_L1),
             QRectF(-13, -9, 22, 22));
    QCOMPARE(renderer.boundsOnElement("prim"_L1),
             QRectF(-10 * sqrt2 - 3, -10 * sqrt2 + 1, 20 * sqrt2, 20 * sqrt2));

    QRectF textBoundsA = renderer.boundsOnElement("textA"_L1);
    QVERIFY(!textBoundsA.isEmpty());
    QCOMPARE(renderer.boundsOnElement("textB"_L1), textBoundsA);

    QRectF cBounds = renderer.boundsOnElement("textC"_L1);
    QRectF dBounds = renderer.boundsOnElement("textD"_L1);
    QVERIFY(!cBounds.isEmpty());
    QCOMPARE(cBounds.size(), dBounds.size());
    QRectF groupBounds = renderer.boundsOnElement("textGroup"_L1);
    QCOMPARE(groupBounds, cBounds | dBounds);
}

void tst_QSvgRenderer::gradientStops() const
{
    {
        QByteArray data(R"(<svg>
                          <defs>
                            <linearGradient id="gradient">
                            </linearGradient>
                          </defs>
                          <rect fill="url(#gradient) " height="64" width="64" x="0" y="0"/>
                        </svg>)");
        QSvgRenderer renderer(data);

        QImage image(64, 64, QImage::Format_ARGB32_Premultiplied);
        QImage refImage(64, 64, QImage::Format_ARGB32_Premultiplied);
        image.fill(0x87654321);
        refImage.fill(0x87654321);

        QPainter painter(&image);
        renderer.render(&painter);
        QCOMPARE(image, refImage);
    }

    {
        QByteArray data(R"(<svg>
                          <defs>
                            <linearGradient id="gradient">
                              <stop offset="1" stop-color="cyan"/>
                            </linearGradient>
                          </defs>
                          <rect fill="url(#gradient) " height="64" width="64" x="0" y="0"/>
                        </svg>)");
        QSvgRenderer renderer(data);

        QImage image(64, 64, QImage::Format_ARGB32_Premultiplied);
        QImage refImage(64, 64, QImage::Format_ARGB32_Premultiplied);
        refImage.fill(0xff00ffff);

        QPainter painter(&image);
        renderer.render(&painter);
        QCOMPARE(image, refImage);
    }

    {
        QByteArray data(R"(<svg>
                          <defs>
                            <linearGradient id="gradient">
                              <stop offset="0" stop-color="red"/>
                              <stop offset="0" stop-color="cyan"/>
                              <stop offset="0.5" stop-color="cyan"/>
                              <stop offset="0.5" stop-color="magenta"/>
                              <stop offset="0.5" stop-color="yellow"/>
                              <stop offset="1" stop-color="yellow"/>
                              <stop offset="1" stop-color="blue"/>
                            </linearGradient>
                          </defs>
                          <rect fill="url(#gradient) " height="64" width="64" x="0" y="0"/>
                        </svg>)");
        QSvgRenderer renderer(data);

        QImage image(64, 64, QImage::Format_ARGB32_Premultiplied);
        QImage refImage(64, 64, QImage::Format_ARGB32_Premultiplied);

        QPainter painter;
        painter.begin(&refImage);
        painter.fillRect(QRectF(0, 0, 32, 64), Qt::cyan);
        painter.fillRect(QRectF(32, 0, 32, 64), Qt::yellow);
        painter.end();

        painter.begin(&image);
        renderer.render(&painter);
        painter.end();

        QCOMPARE(image, refImage);
    }
}

void tst_QSvgRenderer::gradientRefs()
{
    const char *svgs[] = {
        R"(<svg>
            <defs>
                <linearGradient id="gradient">
                    <stop offset="0" stop-color="red" stop-opacity="0"/>
                    <stop offset="1" stop-color="blue"/>
                </linearGradient>
            </defs>
            <rect fill="url(#gradient) " height="8" width="256" x="0" y="0"/>
        </svg>)",
        R"(<svg>
            <defs>
                <linearGradient id="gradient" xlink:href="#gradient0">
                </linearGradient>
                <linearGradient id="gradient0">
                    <stop offset="0" stop-color="red" stop-opacity="0"/>
                    <stop offset="1" stop-color="blue"/>
                </linearGradient>
            </defs>
            <rect fill="url(#gradient) " height="8" width="256" x="0" y="0"/>
        </svg>)",
        R"(<svg>
            <defs>
                <linearGradient id="gradient0">
                    <stop offset="0" stop-color="red" stop-opacity="0"/>
                    <stop offset="1" stop-color="blue"/>
                </linearGradient>
                <linearGradient id="gradient" xlink:href="#gradient0">
                </linearGradient>
            </defs>
            <rect fill="url(#gradient) " height="8" width="256" x="0" y="0"/>
        </svg>)",
        R"(<svg>
            <rect fill="url(#gradient) " height="8" width="256" x="0" y="0"/>
            <defs>
                <linearGradient id="gradient0">
                    <stop offset="0" stop-color="red" stop-opacity="0"/>
                    <stop offset="1" stop-color="blue"/>
                </linearGradient>
                <linearGradient id="gradient" xlink:href="#gradient0">
                </linearGradient>
            </defs>
        </svg>)",
        R"(<svg>
            <defs>
                <linearGradient xlink:href="#0" id="0">
                    <stop offset="0" stop-color="red" stop-opacity="0"/>
                    <stop offset="1" stop-color="blue"/>
                </linearGradient>
            </defs>
            <rect fill="url(#0) " height="8" width="256" x="0" y="0"/>
        </svg>)"
    };
    for (size_t i = 0 ; i < sizeof(svgs) / sizeof(svgs[0]) ; ++i)
    {
        QByteArray data = svgs[i];
        QSvgRenderer renderer(data);

        QImage image(256, 8, QImage::Format_ARGB32_Premultiplied);
        image.fill(0);

        QPainter painter(&image);
        renderer.render(&painter);

        const QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(3));
        QRgb left = line[0]; // transparent black
        QRgb mid = line[127]; // semi transparent magenta
        QRgb right = line[255]; // opaque blue

        QVERIFY(qAlpha(left) < 3 && qRed(left) < 3 && qGreen(left) == 0 && qBlue(left) < 3);
        QVERIFY(qAbs(qAlpha(mid) - 127) < 3 && qAbs(qRed(mid) - 63) < 4
                && qGreen(mid) == 0 && qAbs(qBlue(mid) - 63) < 4);
        QVERIFY(qAlpha(right) > 253 && qRed(right) < 3 && qGreen(right) == 0
                && qBlue(right) > 251);
    }
}

void tst_QSvgRenderer::recursiveRefs_data()
{
    QTest::addColumn<QByteArray>("svg");

    QTest::newRow("single") << QByteArray(R"(<svg>
                                          <linearGradient id="0" xlink:href="#0"/>
                                          <rect x="0" y="0" width="20" height="20" fill="url(#0) "/>
                                          </svg>)");

    QTest::newRow("double") << QByteArray(R"(<svg>
                                          <linearGradient id="0" xlink:href="#1"/>
                                          <linearGradient id="1" xlink:href="#0"/>
                                          <rect x="0" y="0" width="20" height="20" fill="url(#0) "/>
                                          </svg>)");

    QTest::newRow("triple") << QByteArray(R"(<svg>
                                          <linearGradient id="0" xlink:href="#1"/>
                                          <linearGradient id="1" xlink:href="#2"/>
                                          <linearGradient id="2" xlink:href="#0"/>
                                          <rect x="0" y="0" width="20" height="20" fill="url(#0) "/>
                                          </svg>)");

    QTest::newRow("pattern") << QByteArray(R"(<svg><pattern id="pattern" width="4" height="4"
                                              fill="url(#pattern) ">
                                              <rect width="2" height="2" fill=" "/></pattern>
                                              <rect width="2" height="2" fill="url(#pattern) "/>
                                              </svg>)");

    // lead to division by zero in QSvgPattern::patternImage while loading document
    QTest::newRow("pattern-no-elements") << QByteArray(R"(<svg>
                                                          <pattern id="pattern" width="4" height="4"
                                                           fill="url(#pattern) "/>
                                                          </svg>)");
}

void tst_QSvgRenderer::recursiveRefs()
{
    QFETCH(QByteArray, svg);

    QImage image(20, 20, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::green);
    QImage refImage = image.copy();

    QSvgRenderer renderer(svg);
    QPainter painter(&image);
    renderer.render(&painter);
    QCOMPARE(image, refImage);
}


#ifndef QT_NO_COMPRESS
void tst_QSvgRenderer::testGzLoading()
{
    QSvgRenderer renderer(QFINDTESTDATA("heart.svgz"));
    QVERIFY(renderer.isValid());

    QSvgRenderer resourceRenderer(":/heart.svgz"_L1);
    QVERIFY(resourceRenderer.isValid());

    QFile largeFileGz(QFINDTESTDATA("large.svgz"));
    QVERIFY(largeFileGz.open(QIODevice::ReadOnly));
    QByteArray data = largeFileGz.readAll();
    QSvgRenderer autoDetectGzData(data);
    QVERIFY(autoDetectGzData.isValid());
}

#  ifdef QT_BUILD_INTERNAL
QT_BEGIN_NAMESPACE
QByteArray qt_inflateGZipDataFrom(QIODevice *device);
QT_END_NAMESPACE

void tst_QSvgRenderer::testGzHelper_data()
{
    QTest::addColumn<QByteArray>("in");
    QTest::addColumn<QByteArray>("out");

    QTest::newRow("empty") << QByteArray() << QByteArray();
    QTest::newRow("small") << QByteArray::fromHex(QByteArray("1f8b08005819934800034b"
            "cbcfe70200a865327e04000000")) << QByteArray("foo\n");

    QFile largeFileGz(QFINDTESTDATA("large.svgz"));
    QVERIFY(largeFileGz.open(QIODevice::ReadOnly));
    QFile largeFile(QFINDTESTDATA("large.svg"));
    QVERIFY(largeFile.open(QIODevice::ReadOnly));
    QTest::newRow("large") << largeFileGz.readAll() << largeFile.readAll();

    QTest::newRow("zeroes") << QByteArray::fromHex(QByteArray("1f8b0800131f9348000333"
            "301805a360148c54c00500d266708601040000")) << QByteArray(1024, '0').append('\n');

    QTest::newRow("twoMembers") << QByteArray::fromHex(QByteArray("1f8b08001c2a934800034b"
            "cbcfe70200a865327e040000001f8b08001c2a934800034b4a2ce20200e9b3a20404000000"))
        << QByteArray("foo\nbar\n");

    QTest::newRow("corruptedSecondMember") << QByteArray::fromHex(QByteArray("1f8b08001c2a934800034b"
            "cbcfe70200a865327e040000001f8c08001c2a934800034b4a2ce20200e9b3a20404000000"))
        << QByteArray();

}

void tst_QSvgRenderer::testGzHelper()
{
    QFETCH(QByteArray, in);
    QFETCH(QByteArray, out);

    QBuffer buffer(&in);
    QVERIFY(buffer.open(QIODevice::ReadOnly));
    QVERIFY(buffer.isReadable());
    QByteArray result = qt_inflateGZipDataFrom(&buffer);
    QCOMPARE(result, out);
}
#  endif // #ifdef QT_BUILD_INTERNAL
#endif // #ifndef QT_NO_COMPRESS

void tst_QSvgRenderer::fillRule()
{
    static const char *svgs[] = {
        // Paths
        // Default fill-rule (nonzero)
        R"(<svg>
           <rect x="0" y="0" height="30" width="30" fill="blue" />
           <path d="M10 0 L30 0 L30 30 L0 30 L0 10 L20 10 L20 20 L10 20 Z" fill="red" />
        </svg>)",
        // nonzero
        R"(<svg>
           <rect x="0" y="0" height="30" width="30" fill="blue" />
           <path d="M10 0 L30 0 L30 30 L0 30 L0 10 L20 10 L20 20 L10 20 Z" fill="red" fill-rule="nonzero" />
        </svg>)",
        // evenodd
        R"(<svg>
           <rect x="0" y="0" height="30" width="30" fill="blue" />
           <path d="M10 0 L30 0 L30 30 L0 30 L0 10 L20 10 L20 20 L10 20 Z" fill="red" fill-rule="evenodd" />
        </svg>)",

        // Polygons
        // Default fill-rule (nonzero)
        R"(<svg>
           <rect x="0" y="0" height="30" width="30" fill="blue" />
           <polygon points="10 0 30 0 30 30 0 30 0 10 20 10 20 20 10 20" fill="red" />
        </svg>)",
        // nonzero
        R"(<svg>
           <rect x="0" y="0" height="30" width="30" fill="blue" />
           <polygon points="10 0 30 0 30 30 0 30 0 10 20 10 20 20 10 20" fill="red" fill-rule="nonzero" />
        </svg>)",
        // evenodd
        R"(<svg>
           <rect x="0" y="0" height="30" width="30" fill="blue" />
           <polygon points="10 0 30 0 30 30 0 30 0 10 20 10 20 20 10 20" fill="red" fill-rule="evenodd" />
        </svg>)"
    };

    const int COUNT = sizeof(svgs) / sizeof(svgs[0]);
    QImage refImageNonZero(30, 30, QImage::Format_ARGB32_Premultiplied);
    QImage refImageEvenOdd(30, 30, QImage::Format_ARGB32_Premultiplied);
    refImageNonZero.fill(0xffff0000);
    refImageEvenOdd.fill(0xffff0000);
    QPainter p;
    p.begin(&refImageNonZero);
    p.fillRect(QRectF(0, 0, 10, 10), Qt::blue);
    p.end();
    p.begin(&refImageEvenOdd);
    p.fillRect(QRectF(0, 0, 10, 10), Qt::blue);
    p.fillRect(QRectF(10, 10, 10, 10), Qt::blue);
    p.end();

    for (int i = 0; i < COUNT; ++i) {
        QByteArray data(svgs[i]);
        QSvgRenderer renderer(data);
        QImage image(30, 30, QImage::Format_ARGB32_Premultiplied);
        image.fill(0);
        p.begin(&image);
        renderer.render(&p);
        p.end();
        QCOMPARE(image, i % 3 == 2 ? refImageEvenOdd : refImageNonZero);
    }
}

static void opacity_drawSvgAndVerify(const QByteArray &data)
{
    QSvgRenderer renderer(data);
    QVERIFY(renderer.isValid());
    QImage image(10, 10, QImage::Format_ARGB32_Premultiplied);
    image.fill(0xffff00ff);
    QPainter painter(&image);
    renderer.render(&painter);
    painter.end();
    QCOMPARE(image.pixel(5, 5), 0xff7f7f7f);
}

void tst_QSvgRenderer::opacity()
{
    static const char *opacities[] = {"-1.4641", "0", "0.5", "1", "1.337"};
    static const char *firstColors[] = {"#7f7f7f", "#7f7f7f", "#402051", "blue", "#123456"};
    static const char *secondColors[] = {"red", "#bad", "#bedead", "#7f7f7f", "#7f7f7f"};

    // Fill-opacity
    for (int i = 0; i < 5; ++i) {
        QByteArray data("<svg><rect x=\"0\" y=\"0\" height=\"10\" width=\"10\" fill=\"");
        data.append(firstColors[i]);
        data.append("\"/><rect x=\"0\" y=\"0\" height=\"10\" width=\"10\" fill=\"");
        data.append(secondColors[i]);
        data.append("\" fill-opacity=\"");
        data.append(opacities[i]);
        data.append("\"/></svg>");
        opacity_drawSvgAndVerify(data);
    }
    // Stroke-opacity
    for (int i = 0; i < 5; ++i) {
        QByteArray data("<svg viewBox=\"0 0 10 10\"><polyline points=\"0 5 10 5\" fill=\"none\" stroke=\"");
        data.append(firstColors[i]);
        data.append("\" stroke-width=\"10\"/><line x1=\"5\" y1=\"0\" x2=\"5\" y2=\"10\" fill=\"none\" stroke=\"");
        data.append(secondColors[i]);
        data.append("\" stroke-width=\"10\" stroke-opacity=\"");
        data.append(opacities[i]);
        data.append("\"/></svg>");
        opacity_drawSvgAndVerify(data);
    }
    // As gradients:
    // Fill-opacity
    for (int i = 0; i < 5; ++i) {
        QByteArray data("<svg><defs><linearGradient id=\"gradient\"><stop offset=\"0\" stop-color=\"");
        data.append(secondColors[i]);
        data.append("\"/><stop offset=\"1\" stop-color=\"");
        data.append(secondColors[i]);
        data.append("\"/></linearGradient></defs><rect x=\"0\" y=\"0\" height=\"10\" width=\"10\" fill=\"");
        data.append(firstColors[i]);
        data.append("\"/><rect x=\"0\" y=\"0\" height=\"10\" width=\"10\" fill=\"url(#gradient)\" fill-opacity=\"");
        data.append(opacities[i]);
        data.append("\"/></svg>");
        opacity_drawSvgAndVerify(data);
    }
    // Stroke-opacity
    for (int i = 0; i < 5; ++i) {
        QByteArray data("<svg viewBox=\"0 0 10 10\"><defs><linearGradient id=\"grad\"><stop offset=\"0\" stop-color=\"");
        data.append(secondColors[i]);
        data.append("\"/><stop offset=\"1\" stop-color=\"");
        data.append(secondColors[i]);
        data.append("\"/></linearGradient></defs><polyline points=\"0 5 10 5\" fill=\"none\" stroke=\"");
        data.append(firstColors[i]);
        data.append("\" stroke-width=\"10\" /><line x1=\"5\" y1=\"0\" x2=\"5\" y2=\"10\" fill=\"none\" stroke=\"url(#grad)\" stroke-width=\"10\" stroke-opacity=\"");
        data.append(opacities[i]);
        data.append("\" /></svg>");
        opacity_drawSvgAndVerify(data);
    }

    // group opacity QTBUG-122310
    const char *svg = R"svg(
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="-1 -1 37 37">
    <g transform="translate(0, 0)">
        <rect fill="#808080" x="0" y="0" width="10" height="10" fill-opacity="0.5"/>
        <rect fill="#808080" x="5" y="5" width="10" height="10" fill-opacity="0.5"/>
    </g>
    <g transform="translate(20, 0)" fill-opacity="0.5">
        <rect fill="#808080" x="0" y="0" width="10" height="10"/>
        <rect fill="#808080" x="5" y="5" width="10" height="10"/>
    </g>
    <g transform="translate(0, 20)">
        <rect fill="#808080" x="0" y="0" width="10" height="10" opacity="0.5"/>
        <rect fill="#808080" x="5" y="5" width="10" height="10" opacity="0.5"/>
    </g>
    <g transform="translate(20, 20)" opacity="0.5">
        <rect fill="#808080" x="0" y="0" width="10" height="10"/>
        <rect fill="#808080" x="5" y="5" width="10" height="10"/>
    </g>
    </svg>
    )svg";

    QByteArray data(svg);
    QSvgRenderer renderer(data);
    QVERIFY(renderer.isValid());

    QImage image(140, 140, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QPainter p;
    p.begin(&image);
    renderer.render(&p);
    p.end();

    const QRgb lightGray(0xffc0c0c0);
    const QRgb gray(0xffa0a0a0);

    QCOMPARE(image.pixel(QPoint(10, 10)), lightGray);
    QCOMPARE(image.pixel(QPoint(30, 30)), gray);

    QCOMPARE(image.pixel(QPoint(90, 10)), lightGray);
    QCOMPARE(image.pixel(QPoint(110, 30)), gray);

    QCOMPARE(image.pixel(QPoint(10, 90)), lightGray);
    QCOMPARE(image.pixel(QPoint(30, 110)), gray);

    QCOMPARE(image.pixel(QPoint(90, 90)), lightGray);
    QCOMPARE(image.pixel(QPoint(110, 110)), lightGray);
}

void tst_QSvgRenderer::paths()
{
    static const char *svgs[] = {
        // Absolute coordinates, explicit commands.
        "<svg>"
        "   <rect x=\"0\" y=\"0\" height=\"50\" width=\"50\" fill=\"blue\" />"
        "   <path d=\"M50 0 V50 H0 Q0 25 25 25 T50 0 C25 0 50 50 25 50 S25 0 0 0 Z\" fill=\"red\" fill-rule=\"evenodd\"/>"
        "</svg>",
        // Absolute coordinates, implicit commands.
        "<svg>"
        "   <rect x=\"0\" y=\"0\" height=\"50\" width=\"50\" fill=\"blue\" />"
        "   <path d=\"M50 0 50 50 0 50 Q0 25 25 25 Q50 25 50 0 C25 0 50 50 25 50 C0 50 25 0 0 0 Z\" fill=\"red\" fill-rule=\"evenodd\" />"
        "</svg>",
        // Relative coordinates, explicit commands.
        "<svg>"
        "   <rect x=\"0\" y=\"0\" height=\"50\" width=\"50\" fill=\"blue\" />"
        "   <path d=\"m50 0 v50 h-50 q0 -25 25 -25 t25 -25 c-25 0 0 50 -25 50 s0 -50 -25 -50 z\" fill=\"red\" fill-rule=\"evenodd\" />"
        "</svg>",
        // Relative coordinates, implicit commands.
        "<svg>"
        "   <rect x=\"0\" y=\"0\" height=\"50\" width=\"50\" fill=\"blue\" />"
        "   <path d=\"m50 0 0 50 -50 0 q0 -25 25 -25 25 0 25 -25 c-25 0 0 50 -25 50 -25 0 0 -50 -25 -50 z\" fill=\"red\" fill-rule=\"evenodd\" />"
        "</svg>",
        // Absolute coordinates, explicit commands, minimal whitespace.
        "<svg>"
        "   <rect x=\"0\" y=\"0\" height=\"50\" width=\"50\" fill=\"blue\" />"
        "   <path d=\"m50 0v50h-50q0-25 25-25t25-25c-25 0 0 50-25 50s0-50-25-50z\" fill=\"red\" fill-rule=\"evenodd\" />"
        "</svg>",
        // Absolute coordinates, explicit commands, extra whitespace.
        "<svg>"
        "   <rect x=\"0\" y=\"0\" height=\"50\" width=\"50\" fill=\"blue\" />"
        "   <path d=\" M  50  0  V  50  H  0  Q 0  25   25 25 T  50 0 C 25   0 50  50 25 50 S  25 0 0  0 Z  \" fill=\"red\" fill-rule=\"evenodd\" />"
        "</svg>"
    };

    const int COUNT = sizeof(svgs) / sizeof(svgs[0]);
    QImage images[COUNT];
    QPainter p;

    for (int i = 0; i < COUNT; ++i) {
        QByteArray data(svgs[i]);
        QSvgRenderer renderer(data);
        QVERIFY(renderer.isValid());
        images[i] = QImage(50, 50, QImage::Format_ARGB32_Premultiplied);
        images[i].fill(0);
        p.begin(&images[i]);
        renderer.render(&p);
        p.end();
        if (i != 0) {
            QCOMPARE(images[i], images[0]);
        }
    }
}

void tst_QSvgRenderer::paths2()
{
    const char *svg =
        R"(<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16">
            <path d="M 3 8 A 5 5 0 1013 8" id="path1"/>
        </svg>)";

    QByteArray data(svg);
    QSvgRenderer renderer(data);
    QVERIFY(renderer.isValid());
    QCOMPARE(renderer.boundsOnElement("path1"_L1).toRect(), QRect(3, 8, 10, 5));
}

void tst_QSvgRenderer::emptyPath_data()
{
    QTest::addColumn<QByteArray>("svg");

    // This caused a division by zero in in QPainterPath::angleAtPercent, reported by UBSAN.
    QTest::newRow("empty")
            << (R"(<svg><path marker-end="url(#w) "/></svg>)"_ba);
    QTest::newRow("zero-movements")
            << (R"(<svg><path d="M1 2L1 2l0 0H1h0V2v0Z" marker-end="url(#w) "/></svg>)"_ba);
}

void tst_QSvgRenderer::emptyPath()
{
    QFETCH(QByteArray, svg);
    QImage image(377, 233, QImage::Format_RGB32);
    QPainter painter(&image);
    QSvgRenderer renderer(svg);
    renderer.render(&painter);
}

void tst_QSvgRenderer::displayMode()
{
    static const char *svgs[] = {
        // All visible.
        R"(<svg>
           <g>
               <rect x="0" y="0" height="10" width="10" fill="red" />
               <rect x="0" y="0" height="10" width="10" fill="blue" />
           </g>
        </svg>)",
        // Don't display svg element.
        R"(<svg display="none">
           <g>
               <rect x="0" y="0" height="10" width="10" fill="red" />
               <rect x="0" y="0" height="10" width="10" fill="blue" />
           </g>
        </svg>)",
        // Don't display g element.
        R"(<svg>
           <g display="none">
               <rect x="0" y="0" height="10" width="10" fill="red" />
               <rect x="0" y="0" height="10" width="10" fill="blue" />
           </g>
        </svg>)",
        // Don't display first rect element.
        R"(<svg>
           <g>
               <rect x="0" y="0" height="10" width="10" fill="red" display="none" />
               <rect x="0" y="0" height="10" width="10" fill="blue" />
           </g>
        </svg>)",
        // Don't display second rect element.
        R"(<svg>
           <g>
               <rect x="0" y="0" height="10" width="10" fill="red" />
               <rect x="0" y="0" height="10" width="10" fill="blue" display="none" />
           </g>
        </svg>)",
        // Don't display svg element, but set display mode to "inline" for other elements.
        R"(<svg display="none">
           <g display="inline">
               <rect x="0" y="0" height="10" width="10" fill="red" display="inline" />
               <rect x="0" y="0" height="10" width="10" fill="blue" display="inline" />
           </g>
        </svg>)"
    };

    QRgb expectedColors[] = {0xff0000ff, 0xff00ff00, 0xff00ff00, 0xff0000ff, 0xffff0000, 0xff00ff00};

    const int COUNT = sizeof(svgs) / sizeof(svgs[0]);
    QPainter p;

    for (int i = 0; i < COUNT; ++i) {
        QByteArray data(svgs[i]);
        QSvgRenderer renderer(data);
        QVERIFY(renderer.isValid());
        QImage image(10, 10, QImage::Format_ARGB32_Premultiplied);
        image.fill(0xff00ff00);
        p.begin(&image);
        renderer.render(&p);
        p.end();
        QCOMPARE(image.pixel(5, 5), expectedColors[i]);
    }
}

void tst_QSvgRenderer::strokeInherit()
{
    static const char *svgs[] = {
        // Reference.
        R"(<svg viewBox="0 0 200 30">
           <g stroke="blue" stroke-width="20" stroke-linecap="butt"
               stroke-linejoin="miter" stroke-miterlimit="1" stroke-dasharray="20,10"
               stroke-dashoffset="10" stroke-opacity="0.5">
               <polyline fill="none" points="10 10 100 10 100 20 190 20"/>
           </g>
           <g stroke="green" stroke-width="0" stroke-dasharray="3,3,1" stroke-dashoffset="4.5">
               <polyline fill="none" points="10 25 80 25"/>
           </g>
        </svg>)",
        // stroke
        R"(<svg viewBox="0 0 200 30">
           <g stroke="none" stroke-width="20" stroke-linecap="butt"
               stroke-linejoin="miter" stroke-miterlimit="1" stroke-dasharray="20,10"
               stroke-dashoffset="10" stroke-opacity="0.5">
               <polyline fill="none" points="10 10 100 10 100 20 190 20" stroke="blue"/>
           </g>
           <g stroke="yellow" stroke-width="0" stroke-dasharray="3,3,1" stroke-dashoffset="4.5">
               <polyline fill="none" points="10 25 80 25" stroke="green"/>
           </g>
        </svg>)",
        // stroke-width
        R"(<svg viewBox="0 0 200 30">
           <g stroke="blue" stroke-width="0" stroke-linecap="butt"
               stroke-linejoin="miter" stroke-miterlimit="1" stroke-dasharray="20,10"
               stroke-dashoffset="10" stroke-opacity="0.5">
               <polyline fill="none" points="10 10 100 10 100 20 190 20" stroke-width="20"/>
           </g>
           <g stroke="green" stroke-width="10" stroke-dasharray="3,3,1" stroke-dashoffset="4.5">
               <polyline fill="none" points="10 25 80 25" stroke-width="0"/>
           </g>
        </svg>)",
        // stroke-linecap
        R"(<svg viewBox="0 0 200 30">
           <g stroke="blue" stroke-width="20" stroke-linecap="round"
               stroke-linejoin="miter" stroke-miterlimit="1" stroke-dasharray="20,10"
               stroke-dashoffset="10" stroke-opacity="0.5">
               <polyline fill="none" points="10 10 100 10 100 20 190 20" stroke-linecap="butt"/>
           </g>
           <g stroke="green" stroke-width="0" stroke-dasharray="3,3,1" stroke-dashoffset="4.5">
               <polyline fill="none" points="10 25 80 25"/>
           </g>
        </svg>)",
        // stroke-linejoin
        R"(<svg viewBox="0 0 200 30">
          <g stroke="blue" stroke-width="20" stroke-linecap="butt"
               stroke-linejoin="round" stroke-miterlimit="1" stroke-dasharray="20,10"
               stroke-dashoffset="10" stroke-opacity="0.5">
               <polyline fill="none" points="10 10 100 10 100 20 190 20" stroke-linejoin="miter"/>
           </g>
           <g stroke="green" stroke-width="0" stroke-dasharray="3,3,1" stroke-dashoffset="4.5">
               <polyline fill="none" points="10 25 80 25"/>
           </g>
        </svg>)",
        // stroke-miterlimit
        R"(<svg viewBox="0 0 200 30">
           <g stroke="blue" stroke-width="20" stroke-linecap="butt"
               stroke-linejoin="miter" stroke-miterlimit="2" stroke-dasharray="20,10"
               stroke-dashoffset="10" stroke-opacity="0.5">
               <polyline fill="none" points="10 10 100 10 100 20 190 20" stroke-miterlimit="1"/>
           </g>
           <g stroke="green" stroke-width="0" stroke-dasharray="3,3,1" stroke-dashoffset="4.5">
               <polyline fill="none" points="10 25 80 25"/>
           </g>
        </svg>)",
        // stroke-dasharray
        R"(<svg viewBox="0 0 200 30">
           <g stroke="blue" stroke-width="20" stroke-linecap="butt"
               stroke-linejoin="miter" stroke-miterlimit="1" stroke-dasharray="1,1,1,1,1,1,3,1,3,1,3,1,1,1,1,1,1,3"
               stroke-dashoffset="10" stroke-opacity="0.5">
               <polyline fill="none" points="10 10 100 10 100 20 190 20" stroke-dasharray="20,10"/>
           </g>
           <g stroke="green" stroke-width="0" stroke-dasharray="none" stroke-dashoffset="4.5">
               <polyline fill="none" points="10 25 80 25" stroke-dasharray="3,3,1"/>
           </g>
        </svg>)",
        // stroke-dashoffset
        R"(<svg viewBox="0 0 200 30">
           <g stroke="blue" stroke-width="20" stroke-linecap="butt"
               stroke-linejoin="miter" stroke-miterlimit="1" stroke-dasharray="20,10"
               stroke-dashoffset="0" stroke-opacity="0.5">
               <polyline fill="none" points="10 10 100 10 100 20 190 20" stroke-dashoffset="10"/>
           </g>
           <g stroke="green" stroke-width="0" stroke-dasharray="3,3,1" stroke-dashoffset="0">
               <polyline fill="none" points="10 25 80 25" stroke-dashoffset="4.5"/>
           </g>
        </svg>)",
        // stroke-opacity
        R"(<svg viewBox="0 0 200 30">
           <g stroke="blue" stroke-width="20" stroke-linecap="butt"
               stroke-linejoin="miter" stroke-miterlimit="1" stroke-dasharray="20,10"
               stroke-dashoffset="10" stroke-opacity="0">
               <polyline fill="none" points="10 10 100 10 100 20 190 20" stroke-opacity="0.5"/>
           </g>
           <g stroke="green" stroke-width="0" stroke-dasharray="3,3,1" stroke-dashoffset="4.5">
               <polyline fill="none" points="10 25 80 25"/>
           </g>
        </svg>)"
    };

    const int COUNT = sizeof(svgs) / sizeof(svgs[0]);
    QImage images[COUNT];
    QPainter p;

    for (int i = 0; i < COUNT; ++i) {
        QByteArray data(svgs[i]);
        QSvgRenderer renderer(data);
        QVERIFY(renderer.isValid());
        images[i] = QImage(200, 30, QImage::Format_ARGB32_Premultiplied);
        images[i].fill(-1);
        p.begin(&images[i]);
        renderer.render(&p);
        p.end();
        if (i != 0) {
            QCOMPARE(images[0], images[i]);
        }
    }
}

void tst_QSvgRenderer::testFillInheritance()
{
    static const char *svgs[] = {
        //reference
        R"(<svg viewBox = "0 0 200 200">
            <polygon points="20,20 50,120 100,10 40,80 50,80" fill= "red" stroke = "blue" fill-opacity = "0.5" fill-rule = "evenodd"/>
        </svg>)",
        R"(<svg viewBox = "0 0 200 200">
            <polygon points="20,20 50,120 100,10 40,80 50,80" fill= "red" stroke = "blue" fill-opacity = "0.5" fill-rule = "evenodd"/>
            <rect x = "40" y = "40" width = "70" height ="20" fill = "green" fill-opacity = "0"/>
        </svg>)",
        R"(<svg viewBox = "0 0 200 200">
           <g fill = "red" fill-opacity = "0.5" fill-rule = "evenodd">
               <polygon points="20,20 50,120 100,10 40,80 50,80" stroke = "blue"/>
           </g>
            <rect x = "40" y = "40" width = "70" height ="20" fill = "green" fill-opacity = "0"/>
        </svg>)",
        R"(<svg viewBox = "0 0 200 200">
           <g  fill = "green" fill-rule = "nonzero">
               <polygon points="20,20 50,120 100,10 40,80 50,80" stroke = "blue" fill = "red" fill-opacity = "0.5" fill-rule = "evenodd"/>
           </g>
           <g fill-opacity = "0.8" fill = "red">
               <rect x = "40" y = "40" width = "70" height ="20" fill = "green" fill-opacity = "0"/>
           </g>
        </svg>)",
        R"(<svg viewBox = "0 0 200 200">
           <g  fill = "red" >
              <g fill-opacity = "0.5">
                 <g fill-rule = "evenodd">
                    <g>
                        <polygon points="20,20 50,120 100,10 40,80 50,80" stroke = "blue"/>
                    </g>
                 </g>
              </g>
           </g>
           <g fill-opacity = "0.8" >
               <rect x = "40" y = "40" width = "70" height ="20" fill = "none"/>
           </g>
        </svg>)",
        R"(<svg viewBox = "0 0 200 200">
           <g fill = "none" fill-opacity = "0">
               <polygon points="20,20 50,120 100,10 40,80 50,80" stroke = "blue" fill = "red" fill-opacity = "0.5" fill-rule = "evenodd"/>
           </g>
           <g fill-opacity = "0" >
               <rect x = "40" y = "40" width = "70" height ="20" fill = "green"/>
           </g>
        </svg>)"
    };

    const int COUNT = sizeof(svgs) / sizeof(svgs[0]);
    QImage images[COUNT];
    QPainter p;

    for (int i = 0; i < COUNT; ++i) {
        QByteArray data(svgs[i]);
        QSvgRenderer renderer(data);
        QVERIFY(renderer.isValid());
        images[i] = QImage(200, 200, QImage::Format_ARGB32_Premultiplied);
        images[i].fill(-1);
        p.begin(&images[i]);
        renderer.render(&p);
        p.end();
        if (i != 0) {
            QCOMPARE(images[0], images[i]);
        }
    }
}
void tst_QSvgRenderer::testStopOffsetOpacity()
{
    static const char *svgs[] = {
        //reference
        R"(<svg  viewBox="0 0 64 64">
         <radialGradient id="MyGradient1" gradientUnits="userSpaceOnUse" cx="50" cy="50" r="30" fx="20" fy="20">
          <stop offset="0.0" style="stop-color:red"  stop-opacity="0.3"/>
          <stop offset="0.5" style="stop-color:green"  stop-opacity="1"/>
          <stop offset="1" style="stop-color:yellow"  stop-opacity="1"/>
         </radialGradient>
         <radialGradient id="MyGradient2" gradientUnits="userSpaceOnUse" cx="50" cy="70" r="70" fx="20" fy="20">
          <stop offset="0.0" style="stop-color:blue"  stop-opacity="0.3"/>
          <stop offset="0.5" style="stop-color:violet"  stop-opacity="1"/>
          <stop offset="1" style="stop-color:orange"  stop-opacity="1"/>
         </radialGradient>
         <rect  x="5" y="5" width="55" height="55" fill="url(#MyGradient1) " stroke="black" />
         <rect  x="20" y="20" width="35" height="35" fill="url(#MyGradient2) "/>
        </svg>)",
        //Stop Offset
        R"(<svg  viewBox="0 0 64 64">
         <radialGradient id="MyGradient1" gradientUnits="userSpaceOnUse" cx="50" cy="50" r="30" fx="20" fy="20">
          <stop offset="abc" style="stop-color:red"  stop-opacity="0.3"/>
          <stop offset="0.5" style="stop-color:green"  stop-opacity="1"/>
          <stop offset="1" style="stop-color:yellow"  stop-opacity="1"/>
         </radialGradient>
         <radialGradient id="MyGradient2" gradientUnits="userSpaceOnUse" cx="50" cy="70" r="70" fx="20" fy="20">
          <stop offset="-3.bc" style="stop-color:blue"  stop-opacity="0.3"/>
          <stop offset="0.5" style="stop-color:violet"  stop-opacity="1"/>
          <stop offset="1" style="stop-color:orange"  stop-opacity="1"/>
         </radialGradient>
         <rect  x="5" y="5" width="55" height="55" fill="url(#MyGradient1) " stroke="black" />
         <rect  x="20" y="20" width="35" height="35" fill="url(#MyGradient2) "/>
        </svg>)",
        //Stop Opacity
        R"(<svg  viewBox="0 0 64 64">
         <radialGradient id="MyGradient1" gradientUnits="userSpaceOnUse" cx="50" cy="50" r="30" fx="20" fy="20">
          <stop offset="0.0" style="stop-color:red"  stop-opacity="0.3"/>
          <stop offset="0.5" style="stop-color:green"  stop-opacity="x.45"/>
          <stop offset="1" style="stop-color:yellow"  stop-opacity="-3.abc"/>
         </radialGradient>
         <radialGradient id="MyGradient2" gradientUnits="userSpaceOnUse" cx="50" cy="70" r="70" fx="20" fy="20">
          <stop offset="0.0" style="stop-color:blue"  stop-opacity="0.3"/>
          <stop offset="0.5" style="stop-color:violet"  stop-opacity="-0.xy"/>
          <stop offset="1" style="stop-color:orange"  stop-opacity="z.5"/>
         </radialGradient>
         <rect  x="5" y="5" width="55" height="55" fill="url(#MyGradient1) " stroke="black" />
         <rect  x="20" y="20" width="35" height="35" fill="url(#MyGradient2) "/>
        </svg>)",
        //Stop offset and Stop opacity
        R"(<svg  viewBox="0 0 64 64">
         <radialGradient id="MyGradient1" gradientUnits="userSpaceOnUse" cx="50" cy="50" r="30" fx="20" fy="20">
          <stop offset="abc" style="stop-color:red"  stop-opacity="0.3"/>
          <stop offset="0.5" style="stop-color:green"  stop-opacity="x.45"/>
          <stop offset="1" style="stop-color:yellow"  stop-opacity="-3.abc"/>
         </radialGradient>
         <radialGradient id="MyGradient2" gradientUnits="userSpaceOnUse" cx="50" cy="70" r="70" fx="20" fy="20">
          <stop offset="-3.bc" style="stop-color:blue"  stop-opacity="0.3"/>
          <stop offset="0.5" style="stop-color:violet"  stop-opacity="-0.xy"/>
          <stop offset="1" style="stop-color:orange"  stop-opacity="z.5"/>
         </radialGradient>
         <rect  x="5" y="5" width="55" height="55" fill="url(#MyGradient1) " stroke="black" />
         <rect  x="20" y="20" width="35" height="35" fill="url(#MyGradient2) "/>
        </svg>)"
    };

    QImage images[4];
    QPainter p;

    for (int i = 0; i < 4; ++i) {
        QByteArray data(svgs[i]);
        QSvgRenderer renderer(data);
        QVERIFY(renderer.isValid());
        images[i] = QImage(64, 64, QImage::Format_ARGB32_Premultiplied);
        images[i].fill(-1);
        p.begin(&images[i]);
        renderer.render(&p);
        p.end();
    }
    QCOMPARE(images[0], images[1]);
    QCOMPARE(images[0], images[2]);
    QCOMPARE(images[0], images[3]);
}

void tst_QSvgRenderer::testUseElement()
{
    static const char *svgs[] = {
        // 0 - Use referring to non group node (1)
        R"(<svg viewBox = "0 0 200 200">"
         <polygon points="20,20 50,120 100,10 40,80 50,80"/>"
         <polygon points="20,80 50,180 100,70 40,140 50,140" fill= "red" stroke = "blue" fill-opacity = "0.7" fill-rule = "evenodd" stroke-width = "3"/>"
        </svg>)",
        // 1
        R"(<svg viewBox = "0 0 200 200">"
         <polygon id = "usedPolyline" points="20,20 50,120 100,10 40,80 50,80"/>"
         <use y = "60" xlink:href = "#usedPolyline" fill= "red" stroke = "blue" fill-opacity = "0.7" fill-rule = "evenodd" stroke-width = "3"/>"
        </svg>)",
        // 2
        R"(<svg viewBox = "0 0 200 200">"
         <polygon id = "usedPolyline" points="20,20 50,120 100,10 40,80 50,80"/>"
         <g fill = " red" fill-opacity ="0.2">"
        <use y = "60" xlink:href = "#usedPolyline" stroke = "blue" fill-opacity = "0.7" fill-rule = "evenodd" stroke-width = "3"/>"
        </g>"
        </svg>)",
        // 3
        R"(<svg viewBox = "0 0 200 200">
         <polygon id = "usedPolyline" points="20,20 50,120 100,10 40,80 50,80"/>
         <g stroke-width = "3" stroke = "yellow">
          <use y = "60" xlink:href = "#usedPolyline" fill = " red" stroke = "blue" fill-opacity = "0.7" fill-rule = "evenodd"/>
         </g>
        </svg>)",
        // 4 - Use referring to non group node (2)
        R"(<svg viewBox = "0 0 200 200">
         <polygon points="20,20 50,120 100,10 40,80 50,80" fill = "green" fill-rule = "nonzero" stroke = "purple" stroke-width = "4" stroke-dasharray = "1,1,3,1" stroke-offset = "3" stroke-miterlimit = "6" stroke-linecap = "butt" stroke-linejoin = "round"/>
         <polygon points="20,80 50,180 100,70 40,140 50,140" fill= "red" stroke = "blue" fill-opacity = "0.7" fill-rule = "evenodd" stroke-width = "3" stroke-dasharray = "1,1,1,1" stroke-offset = "5" stroke-miterlimit = "3" stroke-linecap = "butt" stroke-linejoin = "square"/>
        </svg>)",
        // 5
        R"(<svg viewBox = "0 0 200 200">
         <g fill = "green" fill-rule = "nonzero" stroke = "purple" stroke-width = "4" stroke-dasharray = "1,1,3,1" stroke-offset = "3" stroke-miterlimit = "6" stroke-linecap = "butt" stroke-linejoin = "round">
          <polygon id = "usedPolyline" points="20,20 50,120 100,10 40,80 50,80" />
         </g>
         <g stroke = "blue" stroke-width = "3" stroke-dasharray = "1,1,1,1" stroke-offset = "5" stroke-miterlimit = "3" stroke-linecap = "butt" stroke-linejoin = "square">
          <use y = "60" xlink:href = "#usedPolyline"  fill-opacity = "0.7" fill= "red" stroke = "blue" fill-rule = "evenodd"/>
         </g>
        </svg>)",
        // 6
        R"(<svg viewBox = "0 0 200 200">
         <g fill = "green" fill-rule = "nonzero" stroke = "purple" stroke-width = "4" stroke-dasharray = "1,1,3,1" stroke-offset = "3" stroke-miterlimit = "6" stroke-linecap = "butt" stroke-linejoin = "round">
          <polygon id = "usedPolyline" points="20,20 50,120 100,10 40,80 50,80" />
         </g>
         <g stroke-width = "3" stroke-dasharray = "1,1,1,1" stroke-offset = "5" stroke-miterlimit = "3" stroke-linecap = "butt" stroke-linejoin = "square" >
          <use y = "60" xlink:href = "#usedPolyline" fill= "red" stroke = "blue" fill-opacity = "0.7" fill-rule = "evenodd" />
         </g>
        </svg>)",
        // 7 - Use referring to group node
        R"(<svg viewBox = "0 0 200 200">
         <g>
          <circle cx="0" cy="0" r="100" fill = "red" fill-opacity = "0.6"/>
          <rect x = "10" y = "10" width = "30" height = "30" fill = "red" fill-opacity = "0.5"/>
          <circle fill="#a6ce39" cx="0" cy="0" r="33" fill-opacity = "0.5"/>
         </g>
        </svg>)",
        // 8
        R"(<svg viewBox = "0 0 200 200">
         <defs>
          <g id="usedG">
           <circle cx="0" cy="0" r="100" fill-opacity = "0.6"/>
           <rect x = "10" y = "10" width = "30" height = "30"/>
           <circle fill="#a6ce39" cx="0" cy="0" r="33" />
          </g>
         </defs>
         <use xlink:href ="#usedG" fill = "red" fill-opacity ="0.5"/>
        </svg>)",
        // 9
        R"(<svg viewBox = "0 0 200 200">
         <defs>
          <g fill = "blue" fill-opacity = "0.3">
           <g id="usedG">
            <circle cx="0" cy="0" r="100" fill-opacity = "0.6"/>
            <rect x = "10" y = "10" width = "30" height = "30"/>
            <circle fill="#a6ce39" cx="0" cy="0" r="33" />
           </g>
          </g>
         </defs>
         <g fill = "red" fill-opacity ="0.5">
          <use xlink:href ="#usedG" />
         </g>
        </svg>)",
        // 10 - Self referral, should be ignored
        R"(<svg><g id="0"><use xlink:href="#0" /></g></svg>)",
        // 11
        R"(<svg width="200" height="200">
          <rect width="100" height="50"/>
        </svg>)",
        // 12
        R"(<svg width="200" height="200">
          <g id="0"><use xlink:href="#0" /><rect width="100" height="50"/></g>
        </svg>)",
        // 13
        R"(<svg width="200" height="200">
          <g id="0"><g><use xlink:href="#0" /><rect width="100" height="50"/></g></g>
        </svg>)",
        // 14 (undefined)
        R"(<svg width="200" height="200">
          <rect width="100" height="50"/>
          <use x="100" y="100" opacity="0.5" xlink:href="#nosuch" />
        </svg>)",
        // 15 - Forward references
        R"(<svg viewBox = "0 0 200 200">
         <use y = "60" xlink:href = "#usedPolyline" fill= "red" stroke = "blue" fill-opacity = "0.7" fill-rule = "evenodd" stroke-width = "3"/>
         <polygon id = "usedPolyline" points="20,20 50,120 100,10 40,80 50,80"/>
        </svg>)",
        // 16
        R"(<svg viewBox = "0 0 200 200">
         <use xlink:href ="#usedG" fill = "red" fill-opacity ="0.5"/>
         <defs>
          <g id="usedG">
           <circle cx="0" cy="0" r="100" fill-opacity = "0.6"/>
           <rect x = "10" y = "10" width = "30" height = "30"/>
           <circle fill="#a6ce39" cx="0" cy="0" r="33" />
          </g>
         </defs>
        </svg>)",
        // 17 - Indirect self referral
        R"(<svg>
         <defs>
           <g id="g0">
             <g id="g1"><use href="#g2"/></g>
             <g id="g2"><use href="#g1"/></g>
           </g>
         </defs>
         <use xlink:href="#g0" fill="black"/>
        </svg>)"
    };

    const int COUNT = sizeof(svgs) / sizeof(svgs[0]);
    QImage images[COUNT];
    QPainter p;

    for (int i = 0; i < COUNT; ++i) {
        QByteArray data(svgs[i]);
        QSvgRenderer renderer(data);
        images[i] = QImage(200, 200, QImage::Format_ARGB32_Premultiplied);
        images[i].fill(-1);
        p.begin(&images[i]);
        renderer.render(&p);
        p.end();

        if (i < 4 && i != 0) {
            QCOMPARE(images[0], images[i]);
        } else if (i > 4 && i < 7) {
            if (sizeof(qreal) != sizeof(float))
            {
                // These images use blending functions which due to numerical
                // issues on Windows CE and likes differ in very few pixels.
                // For this reason an exact comparison will fail.
                QCOMPARE(images[4], images[i]);
            }
        } else if (i > 7 && i < 10) {
            QCOMPARE(images[8], images[i]);
        } else if (i == 12 || i == 13 || i == 17) {
            QCOMPARE(images[10], images[i]);
        } else if (i > 11 && i < 15) {
            QCOMPARE(images[11], images[i]);
        } else if (i == 15) {
            QCOMPARE(images[0], images[i]);
        } else if (i == 16) {
            QCOMPARE(images[8], images[i]);
        }
    }
}

void tst_QSvgRenderer::smallFont()
{
    static const char *svgs[] = { R"(<svg width="50px" height="50px"><text x="10" y="10" font-size="0">Hello world</text></svg>)",
                                  R"(<svg width="50px" height="50px"><text x="10" y="10" font-size="0.5">Hello world</text></svg>)"
    };
    const int COUNT = sizeof(svgs) / sizeof(svgs[0]);
    QImage images[COUNT];
    QPainter p;

    for (int i = 0; i < COUNT; ++i) {
        QByteArray data(svgs[i]);
        if (i == 0)
            QTest::ignoreMessage(QtWarningMsg, "QFont::setPointSizeF: Point size <= 0 (0.000000), must be greater than 0");
        QTest::ignoreMessage(QtWarningMsg, "QFont::setPixelSize: Pixel size <= 0 (0)");
        QSvgRenderer renderer(data);
        images[i] = QImage(50, 50, QImage::Format_ARGB32_Premultiplied);
        images[i].fill(-1);
        p.begin(&images[i]);
        renderer.render(&p);
        p.end();
    }
    QVERIFY(images[0] != images[1]);
}

void tst_QSvgRenderer::glyphWarnings_data()
{
    QTest::addColumn<QString>("missingGlyphAttributes");
    QTest::addColumn<QString>("secondElement");
    QTest::addColumn<QByteArrayList>("warnings");

    const QString glyph = R"(<glyph %1 d="M0 0L9 9z"/>)";
    QTest::newRow("glyph-without-unicode")
        << "" << glyph.arg("")
        << QByteArrayList{"glyph does not define a non-empty 'unicode' attribute and will be ignored",
                          "<input>:1:120: Problem parsing glyph"};
    QTest::newRow("glyph-empty-unicode")
        << "" << glyph.arg(R"(unicode="")")
        << QByteArrayList{"glyph does not define a non-empty 'unicode' attribute and will be ignored",
                          "<input>:1:130: Problem parsing glyph"};
    QTest::newRow("missing-glyph-with-unicode")
        << R"(unicode="a")" << glyph.arg(R"(unicode="a")")
        << QByteArrayList{"Ignoring missing-glyph's 'unicode' attribute"};
    QTest::newRow("double-missing-glyph")
        << "" << R"(<missing-glyph d="M0 0L9 9z"/>)"
        << QByteArrayList{"The font already has a 'missing-glyph' element.",
                          "<input>:1:127: Problem parsing missing-glyph"};
}

void tst_QSvgRenderer::glyphWarnings()
{
    // This test validates that questionable attributes in a font's glyph and missing-glyph elements
    // will trigger suitable warnings while parsing of the entire document finishes sucessfully.

    const QString svgTemplate = R"(<svg><defs>)"
                                R"(<font xml:id="tstFnt"><font-face font-family="tstFnt"/>)"
                                R"(<missing-glyph %1 d="M0 0L9 9z"/>)"
                                R"(%2)"
                                R"(</font></defs>)"
                                R"(<text font-family="tstFnt">a</text></svg>)";

    QFETCH(QString, missingGlyphAttributes);
    QFETCH(QString, secondElement);
    QFETCH(const QByteArrayList, warnings);

    for (const auto &w: warnings)
        QTest::ignoreMessage(QtWarningMsg, w.constData());
    QSvgRenderer renderer(svgTemplate.arg(missingGlyphAttributes).arg(secondElement).toUtf8());
    QVERIFY(renderer.isValid());
}

void tst_QSvgRenderer::styleSheet()
{
    static const char *svgs[] = { R"(<svg><style type="text/css">.cls {fill:#ff0000;}</style><rect class="cls" x = "10" y = "10" width = "30" height = "30"/></svg>)",
                                  R"(<svg><style>.cls {fill:#ff0000;}</style><rect class="cls" x = "10" y = "10" width = "30" height = "30"/></svg>)",
    };
    const int COUNT = sizeof(svgs) / sizeof(svgs[0]);
    QImage images[COUNT];
    QPainter p;

    for (int i = 0; i < COUNT; ++i) {
        QByteArray data(svgs[i]);
        QSvgRenderer renderer(data);
        images[i] = QImage(50, 50, QImage::Format_ARGB32_Premultiplied);
        images[i].fill(-1);
        p.begin(&images[i]);
        renderer.render(&p);
        p.end();
    }
    QCOMPARE(images[0], images[1]);
}

void tst_QSvgRenderer::duplicateStyleId()
{
    QByteArray svg = QByteArrayLiteral(R"(<svg><linearGradient id="a"/>
                                       <rect style="fill:url(#a) "/>
                                       <linearGradient id="a"/></svg>)");
    QTest::ignoreMessage(QtWarningMsg, R"(Duplicate unique style id: "a")");
    QImage image(200, 200, QImage::Format_RGB32);
    QPainter painter(&image);
    QSvgRenderer renderer(svg);
    renderer.render(&painter);
}

void tst_QSvgRenderer::ossFuzzRender_data()
{
    QTest::addColumn<QByteArray>("svg");

    // oss-fuzz's findings can be looked up by their respective id:
    // https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=xxxxx
    // or, respectively:
    // https://issues.oss-fuzz.com/issues/xxxxxxxx
    // when configured with "-sanitize undefined", this resulted in:
    // "runtime error: -nan is outside the range of representable values of type 'int'"
    // runtime error: signed integer overflow: -2147483648 + -2147483648 cannot be represented in type 'int'
    QTest::newRow("NaN-in-path") // id=24131
            << R"(<svg><path d="M- 4 44044404444E-334-"/></svg>)"_ba;
    QTest::newRow("empty polygon") // id=399769595
            << R"-(<svg><linearGradient id="c"/><polygon stroke="url(#c)"/><polygon points="-- 7-" stroke="url(#c)"/></svg>)-"_ba;
    // runtime error: signed integer overflow: -2147483648 + -1 cannot be represented in type 'int'
    QTest::newRow("excessive moveto in path") // id=406541912
            << R"(<svg><path stroke="#000" d="M- 7e8t9 ."/><marker id="c"/><use href=" c"/></svg>)"_ba;
    // Bad-cast to QSvgMarker from QSvgLine -> Heap-buffer-overflow
    QTest::newRow("line-as-marker") // id=496327371
            << R"-(<svg><line x1="4" id="lledr" marker-end="url(#lledr)" stroke="#00f"/></svg>)-"_ba;
    QTest::newRow("line-as-mask") // modeled after 496327371 to test similar problem, needs UBSAN
            << R"-(<svg>
                     <defs>
                      <line x1="4" id="line"/>
                      <mask id="mask" width="2" height="2" mask="url(#line)"/>
                     </defs>
                     <rect width="2" height="2" mask="url(#mask)"/>
                   </svg>)-"_ba;
}

void tst_QSvgRenderer::ossFuzzRender()
{
    QFETCH(QByteArray, svg);
    QImage image(377, 233, QImage::Format_RGB32);
    QPainter painter(&image);
    QSvgRenderer renderer(svg);
    renderer.render(&painter);
}

void tst_QSvgRenderer::ossFuzzLoad_data()
{
    QTest::addColumn<QByteArray>("svg");
    // oss-fuzz's findings can be looked up by their respective id:
    // https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=xxxxx
    // or, respectively:
    // https://issues.oss-fuzz.com/issues/xxxxxxxx
    // when configured with "-sanitize undefined", these resulted in:
    // "runtime error: division by zero"
    QTest::newRow("missing-arc-data") // id=23731
            << R"(<svg><path d="A4------">)"_ba;
    QTest::newRow("subnormal-radius") // id=24738
            << R"(<svg><path d="a 2 1e-212.....">)"_ba;
    // resulted in null pointer deref
    QTest::newRow("badly-nested") // id=61586
            << R"(<svg><style>*{font-family:q}<linearGradient><stop>)"_ba;
    // resulted in stack overflow
    QTest::newRow("cyclic-reference") // id=42532991
            << R"-(<svg><pattern height="3" width="9" id="c"><path d="v4T1-" stroke="url(#c)"><symbol>)-"_ba;
    // resulted in stack overflow
    QTest::newRow("cyclic-reference-from-parent") // id=390467765
            << R"-(<svg stroke="url(#c)"><pattern height="2" width="4" id="c"/><path stroke="#F00" d="v2"/></svg>)-"_ba;
    // resulted in memory leak, reported when configured with "-sanitize address"
    QTest::newRow("animation-without-target") // id=456050169
            << R"-(<svg><animateTransform type="rotate" from=" " to=" " href="#X">)-"_ba;
}

void tst_QSvgRenderer::ossFuzzLoad()
{
    QFETCH(QByteArray, svg);
    QSvgRenderer().load(svg);
}

QByteArray image_data_url(QImage &image) {
    QByteArray data;
    QBuffer buffer(&data);
    QTEST_ASSERT(buffer.open(QBuffer::ReadWrite));
    image.save(&buffer, "PNG");
    buffer.close();
    QByteArray url("data:image/png;base64,");
    url.append(data.toBase64());
    return url;
}

void tst_QSvgRenderer::imageRendering() {
    QImage img(2, 2, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::green);
    img.setPixel(0, 0, qRgb(255, 0, 0));
    img.setPixel(1, 1, qRgb(255, 0, 0));
    QByteArray imgurl(image_data_url(img));
    QString svgtemplate(
        "<svg><g transform='scale(2, 2)'>"
            "<image image-rendering='%1' xlink:href='%2' width='2' height='2' />"
        "</g></svg>"
    );
    const char *cases[] = {"optimizeQuality", "optimizeSpeed"};
    for (auto ir: cases) {
        QString svg = svgtemplate.arg(QLatin1String(ir)).arg(QLatin1String(imgurl));
        QImage img1(4, 4, QImage::Format_ARGB32);
        QPainter p1;
        p1.begin(&img1);
        QSvgRenderer renderer(svg.toLatin1());
        Q_ASSERT(renderer.isValid());
        renderer.render(&p1);
        p1.end();

        QImage img2(4, 4, QImage::Format_ARGB32);
        QPainter p2(&img2);
        p2.scale(2, 2);
        if (QLatin1String(ir) == "optimizeSpeed"_L1)
            p2.setRenderHint(QPainter::SmoothPixmapTransform, false);
        else if (QLatin1String(ir) == "optimizeQuality"_L1)
            p2.setRenderHint(QPainter::SmoothPixmapTransform, true);
        p2.drawImage(0, 0, img);
        p2.end();
        QCOMPARE(img1, img2);
    }
}

void tst_QSvgRenderer::imageMalformedDataUrl()
{
    // The input below triggered an assert in qDecodeDataUrl() which is used when creating svg
    // nodes. That assert is fixed and tested in qtbase. Still, the input is invalid and should be
    // treated as such. The test makes sure that QSvgRenderer properly warns about it.
    QTest::ignoreMessage(QtWarningMsg, R"(Could not create image from "data:charset,")");
    QVERIFY(QSvgRenderer().load(
            R"(<svg><image width="1" height="1" xlink:href="data:charset,"/></svg>)"_ba));
}

void tst_QSvgRenderer::illegalAnimateTransform_data()
{
    QTest::addColumn<QByteArray>("svg");

    QTest::newRow("surplus-values")
            << R"(<svg><animateTransform type="rotate" begin="1" dur="2" values="8,0,5,0">)"_ba;
    QTest::newRow("missing-value")
            << R"(<svg><animateTransform type="rotate" begin="1" dur="2" values="1,2">)"_ba;
    QTest::newRow("from:..+to:f")
            << R"(<svg><animateTransform type="rotate" begin="1" dur="2" from=".. 5 2" to="f">)"_ba;
    QTest::newRow("by:--,..")
            << R"(<svg><animateTransform type="scale" begin="1" dur="2" by="--,..">)"_ba;
}

void tst_QSvgRenderer::illegalAnimateTransform()
{
    QFETCH(QByteArray, svg);
    QSvgRenderer renderer;
    QVERIFY(!renderer.load(svg)); // also shouldn't assert
}

void tst_QSvgRenderer::testMaskElement()
{
    QByteArray svgDoc(R"(<svg width="240" height="240">
                        <defs>
                            <radialGradient id="myGradient">
                                <stop offset="0" stop-color="black"/>
                                <stop offset="1" stop-color="white"/>
                            </radialGradient>
                            <mask id="mask" width="240" height="240">
                                <rect width="240" height="240" fill="white"/>
                                <circle cx="120" cy="120" r="120" fill="url(#myGradient) "/>
                            </mask>
                        </defs>
                        <rect width="240" height="240" fill="red" mask="url(#mask) "/>
                      </svg>)");

    QSvgRenderer renderer(svgDoc);
    QVERIFY(renderer.isValid());

    QImage image(240, 240, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QImage refImage(240, 240, QImage::Format_ARGB32_Premultiplied);
    refImage.fill(Qt::transparent);
    QImage refMask(240, 240, QImage::QImage::Format_RGBA8888);

    QPainter p;
    p.begin(&image);
    renderer.render(&p);
    p.end();

    p.begin(&refMask);
    p.fillRect(0, 0, 240, 240, QColorConstants::Svg::white);
    QRadialGradient radialGradient(0.5, 0.5, 0.5, 0.5, 0.5, 0);
    radialGradient.setCoordinateMode(QGradient::ObjectMode);
    radialGradient.setInterpolationMode(QGradient::ComponentInterpolation);
    QBrush gradientBrush(radialGradient);
    p.setBrush(gradientBrush);
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(120, 120), 120, 120);
    p.end();

    for (int i=0; i < refMask.height(); i++) {
        QRgb *line = reinterpret_cast<QRgb *>(refMask.scanLine(i));
        for (int j=0; j < refMask.width(); j++) {
            const qreal rC = 0.2125, gC = 0.7154, bC = 0.0721; //luminanceToAlpha following SVG 1.1
            int alpha = 255 - (qRed(line[j]) * rC + qGreen(line[j]) * gC + qBlue(line[j]) * bC) * qAlpha(line[j])/255.;
            line[j] = qRgba(0, 0, 0, alpha);
        }
    }

    p.begin(&refImage);
    p.fillRect(0, 0, 240, 240, QColorConstants::Svg::red);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.drawImage(QRect(0, 0, 240, 240), refMask);
    p.end();

    QCOMPARE(refImage, image);
}

void tst_QSvgRenderer::testSymbol()
{
    QByteArray svgDoc(R"(<svg width="100" height="100">
                      <symbol id="dot" width="100" height="100" viewBox="0 0 1 1">
                      <rect x="0.25" y="0.25" width="0.5" height="0.5" fill="red"/>
                      </symbol>
                      <use href="#dot" x="0" y="0"/>
                      </svg>)");

    QSvgRenderer renderer(svgDoc);
    QVERIFY(renderer.isValid());

    QImage image(100, 100, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QImage refImage(100, 100, QImage::Format_ARGB32_Premultiplied);
    refImage.fill(Qt::white);

    QPainter p;
    p.begin(&image);
    renderer.render(&p);
    p.end();

    p.begin(&refImage);
    p.setBrush(Qt::red);
    p.setPen(Qt::NoPen);
    p.drawRect(25, 25, 50, 50);
    p.end();

    QCOMPARE(refImage, image);
}

void tst_QSvgRenderer::testMarker()
{
    QByteArray svgDoc(R"(<svg width="100" height="100">
                      <marker id="mark" markerWidth="10" markerHeight="10" viewBox="0 0 1 1" refX="0" refY="0.5">
                      <rect x="0" y="0" width="1" height="1" fill="red"/>
                      </marker>
                      <line x1="10" y1="50" x2="90" y2="50" stroke="white" marker-end="url(#mark) "/>
                      </svg>)");

    QSvgRenderer renderer(svgDoc);
    QVERIFY(renderer.isValid());

    QImage image(100, 100, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QImage refImage(100, 100, QImage::Format_ARGB32_Premultiplied);
    refImage.fill(Qt::white);

    QPainter p;
    p.begin(&image);
    renderer.render(&p);
    p.end();

    p.begin(&refImage);
    p.setPen(Qt::white);
    p.drawLine(10, 50, 90, 50);
    p.setBrush(Qt::red);
    p.setPen(Qt::NoPen);
    p.drawRect(90, 45, 10, 10);
    p.end();

    QCOMPARE(refImage, image);
}

void tst_QSvgRenderer::tSpanLineBreak()
{
    QSvgRenderer renderer;
    QVERIFY(renderer.load("<svg><textArea>Foo<tbreak/>Bar</textArea></svg>"_ba));

    QImage img(50, 50, QImage::Format_ARGB32);
    {
        QPainter p(&img);
        renderer.render(&p); // Don't crash
    }
}

static const char *const animatedSvgContents = R"(<svg>
            <path d="M36 18c0-9.94-8.06-18-18-18">
                <animateTransform attributeName="transform" type="rotate" from="0 18 18" to="360 18 18" dur="1s" repeatCount="indefinite"/>
            </path></svg>)";

void tst_QSvgRenderer::animated()
{
    QSvgRenderer renderer;
    QVERIFY(renderer.load(QByteArray(animatedSvgContents)));
    QVERIFY(renderer.isAnimationEnabled());
    QCOMPARE(renderer.framesPerSecond(), 30);
    QTimer *timer = renderer.findChild<QTimer *>();
    QVERIFY(timer);
    QVERIFY(timer->isActive());

    // Toggling animationEnabled
    renderer.setAnimationEnabled(false);
    QVERIFY(!renderer.isAnimationEnabled());
    QVERIFY(!timer->isActive());
    renderer.setAnimationEnabled(true);
    QVERIFY(renderer.isAnimationEnabled());
    QVERIFY(timer->isActive());

    // Adjusting the FPS
    renderer.setFramesPerSecond(0);
    QVERIFY(renderer.isAnimationEnabled());
    QVERIFY(!timer->isActive());
    renderer.setFramesPerSecond(30);
    QVERIFY(renderer.isAnimationEnabled());
    QVERIFY(timer->isActive());

    // Mixing both
    renderer.setFramesPerSecond(0);
    QVERIFY(!timer->isActive());
    renderer.setAnimationEnabled(true); // this isn't enough to restart the animation, we are still at FPS 0
    QVERIFY(!timer->isActive());
    renderer.setFramesPerSecond(30);
    QVERIFY(timer->isActive());

    // Load non-animated SVG
    QVERIFY(renderer.load(QByteArray(src)));
    QVERIFY(renderer.isAnimationEnabled()); // property didn't change
    QVERIFY(!timer->isActive()); // but timer stopped
}

void tst_QSvgRenderer::notAnimated()
{
    // Start with animations disabled
    QSvgRenderer renderer;
    renderer.setAnimationEnabled(false);
    QVERIFY(renderer.load(QByteArray(animatedSvgContents)));
    QVERIFY(!renderer.isAnimationEnabled());
}

void tst_QSvgRenderer::notAnimatedOption()
{
    QSvgRenderer renderer;
    renderer.setOptions(QtSvg::Option::DisableAnimations);
    // Shouldn't crash accessing the animator when it's disabled.
    QVERIFY(renderer.load(QByteArray(animatedSvgContents)));
}

void tst_QSvgRenderer::testPatternElement()
{
    QByteArray svgDoc(R"(<svg viewBox="0 0 200 200">
                        <pattern id="pattern" patternUnits="userSpaceOnUse" width="20" height="20">
                            <rect x="0" y="0" width="10" height="10" fill="red"/>
                            <rect x="10" y="0" width="10" height="10" fill="green"/>
                            <rect x="0" y="10" width="10" height="10" fill="blue"/>
                            <rect x="10" y="10" width="10" height="10" fill="yellow"/>
                        </pattern>
                        <rect width="200" height="200" fill="url(#pattern) "/>
                    </svg>)");

    QSvgRenderer renderer(svgDoc);
    QVERIFY(renderer.isValid());

    QImage image(200, 200, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QImage refImage(200, 200, QImage::Format_ARGB32_Premultiplied);
    refImage.fill(Qt::white);
    QImage refPattern(20, 20, QImage::Format_ARGB32);
    refPattern.fill(Qt::transparent);

    QPainter p;
    p.begin(&image);
    renderer.render(&p);
    p.end();

    p.begin(&refPattern);
    p.fillRect(0, 0, 10, 10, QColorConstants::Svg::red);
    p.fillRect(10, 0, 10, 10, QColorConstants::Svg::green);
    p.fillRect(0, 10, 10, 10, QColorConstants::Svg::blue);
    p.fillRect(10, 10, 10, 10, QColorConstants::Svg::yellow);
    p.end();

    p.begin(&refImage);
    p.fillRect(0, 0, 200, 200, QBrush(refPattern));
    p.end();

    QCOMPARE(refImage, image);
}

void tst_QSvgRenderer::testMisplacedElement()
{
    // This input caused a QSvgPattern node to be created with a QSvgPatternStyle referencing to it.
    // The code then detected that the <pattern> element is misplaced in the <text> element and
    // deleted it. That left behind the QSvgPatternStyle pointing to the deleted QSvgPattern. That
    // was reported when running the test with ASAN or UBSAN.
    QByteArray svg(R"(<svg>
                      <text><pattern id="ptn" width="4" height="4"/></text>
                      <g fill="url(#ptn) "/>
                      </svg>)");

    QImage image(20, 20, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::green);
    QImage refImage = image.copy();

    QTest::ignoreMessage(QtWarningMsg, "<input>:2:68: Could not add child element to parent "
                                       "element because the types are incorrect.");
    QTest::ignoreMessage(QtWarningMsg, "<input>:4:28: Could not resolve property: #ptn");

    QSvgRenderer renderer(svg);
    QPainter painter(&image);
    renderer.render(&painter);
    QCOMPARE(image, refImage);
}

void tst_QSvgRenderer::testCycles_data()
{
    QTest::addColumn<QByteArray>("svgDoc");

    QTest::newRow("fill-pattern") << R"(<svg viewBox="0 0 200 200">
                         <pattern id="pattern" patternUnits="userSpaceOnUse" width="20" height="20">
                           <rect x="0" y="0" width="10" height="10" fill="url(#pattern) "/>
                         </pattern>
                       </svg>)"_ba;

    QTest::newRow("use") << R"(<svg viewBox="0 0 200 200">
                                 <g xml:id="group-1"><use xml:id="use-1" xlink:href="#group-2"/></g>
                                 <g xml:id="group-2"><use xml:id="use-2" xlink:href="#group-1"/></g>
                               </svg>)"_ba;
}

void tst_QSvgRenderer::testCycles()
{
    QFETCH(QByteArray, svgDoc);
#if QT_CONFIG(regularexpression)
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Cycles detected in SVG"));
#endif
    QSvgRenderer renderer(svgDoc);
    QVERIFY(!renderer.isValid());
}

void tst_QSvgRenderer::testFeFlood()
{
    QByteArray svgDoc(R"(<svg width="50" height="50">
                      <filter id="f1">
                      <feFlood flood-color="red"/>
                      </filter>
                      <rect x="10" y="10" width="30" height="30" fill="blue" filter="url(#f1) "/>
                      <rect x="10" y="10" width="30" height="30" fill="blue"/>
                      </svg>)");

    QSvgRenderer renderer(svgDoc);
    QVERIFY(renderer.isValid());

    QImage image(100, 100, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QImage refImage(100, 100, QImage::Format_ARGB32_Premultiplied);
    refImage.fill(Qt::white);

    QPainter p;
    p.begin(&image);
    renderer.render(&p);
    p.end();

    p.begin(&refImage);
    p.fillRect(14, 14, 72, 72, Qt::red);
    p.fillRect(20, 20, 60, 60, Qt::blue);
    p.end();

    QCOMPARE(refImage, image);
}

void tst_QSvgRenderer::testFeOffset()
{
    QByteArray svgDoc(R"(<svg width="50" height="50">
                      <defs>
                      <filter id="f1">
                      <feOffset in="SourceGraphic" dx="5" dy="5"/>
                      </filter>
                      </defs>
                      <rect x="10" y="10" width="30" height="30" stroke="none" fill="blue"/>
                      </svg>)");

    QSvgRenderer renderer(svgDoc);
    QVERIFY(renderer.isValid());

    QImage image(50, 50, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QImage refImage(50, 50, QImage::Format_ARGB32_Premultiplied);
    refImage.fill(Qt::white);

    QPainter p;
    p.begin(&image);
    renderer.render(&p);
    p.end();

    p.begin(&refImage);
    p.fillRect(10, 10, 30, 30, Qt::blue);
    p.end();

    QCOMPARE(refImage, image);
}

void tst_QSvgRenderer::testFeColorMatrix()
{
    QByteArray svgDoc(R"(<svg width="50" height="50">
                      <defs>
                      <filter id="f1">
                      <feColorMatrix in="SourceGraphic" type="saturate" values="0"/>
                      </filter>
                      </defs>
                      <rect x="0" y="0" width="50" height="50" stroke="none" fill="red" filter="url(#f1) "/>
                      </svg>)");

    QSvgRenderer renderer(svgDoc);
    QVERIFY(renderer.isValid());

    QImage image(50, 50, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QImage refImage(50, 50, QImage::Format_ARGB32_Premultiplied);
    refImage.fill(Qt::white);

    QPainter p;
    p.begin(&image);
    renderer.render(&p);
    p.end();

    QVERIFY(image.allGray());
}

void tst_QSvgRenderer::testFeMerge()
{
    QByteArray svgDoc(R"(<svg width="50" height="50">
                      <filter id="f1">
                      <feOffset in="SourceAlpha" dx="2" dy="2"/>
                      <feMerge>
                      <feMergeNode/>
                      <feMergeNode in="SourceGraphic"/>
                      </feMerge>
                      </filter>
                      <rect x="10" y="10" width="30" height="30" fill="blue" filter="url(#f1) "/>
                      </svg>)");

    QSvgRenderer renderer(svgDoc);
    QVERIFY(renderer.isValid());

    QImage image(50, 50, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QImage refImage(50, 50, QImage::Format_ARGB32_Premultiplied);
    refImage.fill(Qt::white);

    QPainter p;
    p.begin(&image);
    renderer.render(&p);
    p.end();

    p.begin(&refImage);
    p.fillRect(12, 12, 30, 30, Qt::black);
    p.fillRect(10, 10, 30, 30, Qt::blue);
    p.end();

    QCOMPARE(refImage, image);
}


void tst_QSvgRenderer::testFeComposite()
{
    QByteArray svgDoc(R"(<svg width="50" height="50">
                      <filter id="f1">
                      <feOffset in="SourceAlpha" dx="2" dy="2"/>
                      <feComposite in2="SourceGraphic" operator="over"/>
                      </filter>
                      <rect x="10" y="10" width="30" height="30" fill="blue" filter="url(#f1) "/>
                      </svg>)");

    QSvgRenderer renderer(svgDoc);
    QVERIFY(renderer.isValid());

    QImage image(50, 50, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QImage refImage(50, 50, QImage::Format_ARGB32_Premultiplied);
    refImage.fill(Qt::white);

    QPainter p;
    p.begin(&image);
    renderer.render(&p);
    p.end();

    p.begin(&refImage);
    p.fillRect(10, 10, 30, 30, Qt::blue);
    p.fillRect(12, 12, 30, 30, Qt::black);
    p.end();

    QCOMPARE(refImage, image);
}

void tst_QSvgRenderer::testFeGaussian()
{
    QByteArray svgDoc(R"(<svg width="50" height="50">
                      <filter id="f1">
                      <feGaussianBlur in="SourceGraphic" stdDeviation="5"/>
                      </filter>
                      <rect x="10" y="10" width="30" height="30" fill="black" filter="url(#f1) "/>
                      </svg>)");

    QSvgRenderer renderer(svgDoc);
    QVERIFY(renderer.isValid());

    QImage image(50, 50, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);

    QPainter p;
    p.begin(&image);
    renderer.render(&p);
    p.end();

    QVERIFY(image.allGray());

    QCOMPARE(qGray(image.pixel(QPoint(0, 25))), 255);
    QCOMPARE(qGray(image.pixel(QPoint(5, 25))), 255);
    QCOMPARE_LE(qGray(image.pixel(QPoint(10, 25))), 150);
    QCOMPARE_GE(qGray(image.pixel(QPoint(10, 25))), 100);
    QCOMPARE_LE(qGray(image.pixel(QPoint(25, 25))), 10);

}

void tst_QSvgRenderer::testFeBlend()
{
    QByteArray svgDoc(R"(<svg width="50" height="50">
                      <filter id="f1">
                      <feOffset in="SourceAlpha" dx="2" dy="2"/>
                      <feBlend in2="SourceGraphic" mode="normal"/>
                      </filter>
                      <rect x="10" y="10" width="30" height="30" fill="blue" filter="url(#f1) "/>
                      </svg>)");

    QSvgRenderer renderer(svgDoc);
    QVERIFY(renderer.isValid());

    QImage image(50, 50, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QImage refImage(50, 50, QImage::Format_ARGB32_Premultiplied);
    refImage.fill(Qt::transparent);

    QPainter p;
    p.begin(&image);
    renderer.render(&p);
    p.end();

    p.begin(&refImage);
    p.fillRect(10, 10, 30, 30, Qt::blue);
    p.fillRect(12, 12, 30, 30, Qt::black);
    p.end();

    QCOMPARE(refImage, image);
}

void tst_QSvgRenderer::testOption_data()
{
    QTest::addColumn<QtSvg::Option>("option");

    QTest::newRow("No Option") << QtSvg::Option::NoOption;
    QTest::newRow("Tiny 1.2 Features") << QtSvg::Option::Tiny12FeaturesOnly;
    QTest::newRow("Assume Trusted Source") << QtSvg::Option::AssumeTrustedSource;
    QTest::newRow("Disable SMIL") << QtSvg::Option::DisableSMILAnimations;
    QTest::newRow("Disable Animations") << QtSvg::Option::DisableAnimations;
}

void tst_QSvgRenderer::testOption()
{
    QFETCH(QtSvg::Option, option);

    QSvgRenderer renderer;
    renderer.setOptions(option);
    QVERIFY(renderer.options().testFlag(option));
}

QTEST_MAIN(tst_QSvgRenderer)
#include "tst_qsvgrenderer.moc"
