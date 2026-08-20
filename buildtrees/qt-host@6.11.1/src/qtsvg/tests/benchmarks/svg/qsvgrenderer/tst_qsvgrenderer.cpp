// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qtest.h>

#include <QFile>
#include <QPainter>
#include <QPicture>
#include <QSvgRenderer>

using namespace Qt::Literals::StringLiterals;

class tst_QSvgRenderer : public QObject
{
    Q_OBJECT

public:
    tst_QSvgRenderer();
    virtual ~tst_QSvgRenderer();

public slots:
    void init();
    void cleanup();

private slots:
    void construct();
    void load_data();
    void load();
    void render_picture_data();
    void render_picture();
    void render_image_data();
    void render_image();
};

tst_QSvgRenderer::tst_QSvgRenderer()
{
}

tst_QSvgRenderer::~tst_QSvgRenderer()
{
}

void tst_QSvgRenderer::init()
{
}

void tst_QSvgRenderer::cleanup()
{
}

void tst_QSvgRenderer::construct()
{
    QBENCHMARK {
        QSvgRenderer renderer;
    }
}

void load_sample_filenames()
{
    QTest::addColumn<QString>("fileName");

    QTest::newRow("tiger") << u":/data/tiger.svg"_s;
    QTest::newRow("filter") << u":/data/filter.svg"_s;
    QTest::newRow("mask") << u":/data/mask.svg"_s;
    QTest::newRow("opacity") << u":/data/opacity.svg"_s;
    QTest::newRow("text") << u":/data/text.svg"_s;
}

void tst_QSvgRenderer::load_data()
{
    load_sample_filenames();
}

void tst_QSvgRenderer::load()
{
    QFETCH(QString, fileName);
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        QFAIL(qPrintable(QStringLiteral("Cannot open file %1").arg(fileName)));
    QByteArray data = file.readAll();
    QSvgRenderer renderer;

    QBENCHMARK {
        renderer.load(data);
    }
}

void tst_QSvgRenderer::render_picture_data()
{
    load_sample_filenames();
}

void tst_QSvgRenderer::render_picture()
{
    QFETCH(QString, fileName);
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        QFAIL(qPrintable(QStringLiteral("Cannot open file %1").arg(fileName)));
    QByteArray data = file.readAll();

    QSvgRenderer renderer;
    renderer.load(data);

    QPicture picture;
    picture.setBoundingRect(QRect(QPoint(), renderer.defaultSize()));
    QPainter painter(&picture);

    QBENCHMARK {
        renderer.render(&painter);
    }
}

void tst_QSvgRenderer::render_image_data()
{
    load_sample_filenames();
}

void tst_QSvgRenderer::render_image()
{
    QFETCH(QString, fileName);
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        QFAIL(qPrintable(QStringLiteral("Cannot open file %1").arg(fileName)));
    QByteArray data = file.readAll();

    QSvgRenderer renderer;
    renderer.load(data);

    QImage image(renderer.defaultSize(), QImage::Format_RGBA8888);
    QPainter painter(&image);

    QBENCHMARK {
        renderer.render(&painter);
    }
}


QTEST_MAIN(tst_QSvgRenderer)
#include "tst_qsvgrenderer.moc"
