// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore>
#include <QtSvg>
#include <QtGui>
#include <stdlib.h>

static QImage render(const QString &filePath)
{
    fprintf(stdout, "Rendering %s\n", qPrintable(filePath));
    QSvgRenderer renderer(filePath);
    if (!renderer.isValid()) {
        fprintf(stderr, "Could not load SVG file %s\n", qPrintable(filePath));
        return QImage();
    }
    QImage image(480, 360, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    {
        QPainter p(&image);
        renderer.render(&p);
    }
    return image;
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument(QLatin1String("command"), QLatin1String("[create-baseline,diff]"));
    parser.addPositionalArgument(QLatin1String("path-to-svg-testsuite"), QLatin1String("Path to the svg/ sub-directory of the test suite"));

    parser.process(app);

    const auto args = parser.positionalArguments();

    if (args.count() != 2) {
        parser.showHelp(EXIT_FAILURE);
        return EXIT_FAILURE; // never reached
    }

    const QString commandAsString = args.at(0);
    const QString sourcePath = args.at(1);

    QDirIterator sourceFileIterator(sourcePath, QStringList(QLatin1String("*.svg")), QDir::Files);
    const QString baselinePath = "baseline";
    const QString diffPath = "difference";

    const auto referenceFilePath = [baselinePath](const QFileInfo &testCaseFileInfo) -> QString {
        return baselinePath + QLatin1Char('/') + testCaseFileInfo.baseName() + QLatin1String(".png");
    };

    if (commandAsString == "create-baseline") {
        while (sourceFileIterator.hasNext()) {
            sourceFileIterator.next();

            QImage image = render(sourceFileIterator.filePath());
            if (image.isNull())
                return EXIT_FAILURE;
            QString outputFileName = referenceFilePath(sourceFileIterator.fileInfo());
            if (!QDir().mkpath(baselinePath) || !image.save(outputFileName)) {
                fprintf(stderr, "Could not save PNG file %s\n", qPrintable(outputFileName));
                return EXIT_FAILURE;
            }
        }
    } else if (commandAsString == "diff") {
        while (sourceFileIterator.hasNext()) {
            sourceFileIterator.next();

            QImage actual = render(sourceFileIterator.filePath());
            const QString referencePath = referenceFilePath(sourceFileIterator.fileInfo());
            QImage reference;
            if (!reference.load(referencePath)) {
                fprintf(stderr, "Could not load reference file %s\n", qPrintable(referencePath));
                return EXIT_FAILURE;
            }

            if (actual == reference)
                continue;

            QImage sideBySideImage(actual.width() * 2, actual.height(), QImage::Format_ARGB32);
            sideBySideImage.fill(Qt::transparent);
            {
                QPainter p(&sideBySideImage);
                p.drawImage(0, 0, actual);
                p.drawImage(actual.width(), 0, reference);
            }

            const QString sideBySideFileName = diffPath + QLatin1Char('/') + sourceFileIterator.fileInfo().baseName() + QLatin1String(".png");
            if (!QDir().mkpath(diffPath) || !sideBySideImage.save(sideBySideFileName)) {
                fprintf(stderr, "Could not save side-by-side image at %s\n", qPrintable(sideBySideFileName));
                return EXIT_FAILURE;
            }
        }
    } else {
        fprintf(stderr, "Unknown command %s\n", qPrintable(commandAsString));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

