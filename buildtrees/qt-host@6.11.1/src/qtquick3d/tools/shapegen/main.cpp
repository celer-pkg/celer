// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QCoreApplication>

#include <qlist.h>

#include <QCommandLineOption>
#include <QCommandLineParser>

#include <QtCore/qfile.h>
#include <QtCore/qdir.h>

#include "shapemanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser cmdLineparser;
    cmdLineparser.setApplicationDescription(QLatin1String("Tool to generate Qt Quick 3D Particles Custom Shapes"));
    cmdLineparser.addHelpOption();

    QCommandLineOption imageOption({QChar(u'i'), QLatin1String("image")}, QLatin1String("Input image for the data."), QLatin1String("file"));
    cmdLineparser.addOption(imageOption);

    QCommandLineOption outputOption({QChar(u'o'), QLatin1String("output")}, QLatin1String("Output CBOR file for the shape data."), QLatin1String("file"));
    cmdLineparser.addOption(outputOption);

    QCommandLineOption depthOption({QChar(u'd'), QLatin1String("depth")}, QLatin1String("Depth (z scale) for the data."), QLatin1String("number"));
    cmdLineparser.addOption(depthOption);

    QCommandLineOption scaleOption({QChar(u's'), QLatin1String("scale")}, QLatin1String("Scale used for the image data. Default 1.0"), QLatin1String("number"));
    cmdLineparser.addOption(scaleOption);

    QCommandLineOption amountOption({QChar(u'a'), QLatin1String("amount")}, QLatin1String("Amount of position data to generate."), QLatin1String("number"));
    cmdLineparser.addOption(amountOption);

    QCommandLineOption sortPositionOption({QChar(u'p'), QLatin1String("sorting-position")}, QLatin1String("Position to use for sorting. Format \"x, y, z\""), QLatin1String("qvector3d"));
    cmdLineparser.addOption(sortPositionOption);

    QCommandLineOption dumpCborFileOption({QChar(u'l'), QLatin1String("list-cbor")}, QLatin1String("Lists CBOR file content."));
    cmdLineparser.addOption(dumpCborFileOption);

    cmdLineparser.process(app);

    ShapeManager manager;

    if (!cmdLineparser.isSet(imageOption)) {
        qWarning("Please provide the input image");
        app.exit(-1);
        return -1;
    } else {
        const auto imageFile = cmdLineparser.value(imageOption);
        manager.setImage(imageFile);
    }

    if (cmdLineparser.isSet(depthOption))
        manager.setDepth(cmdLineparser.value(depthOption).toFloat());

    if (cmdLineparser.isSet(scaleOption))
        manager.setScale(cmdLineparser.value(scaleOption).toFloat());

    if (cmdLineparser.isSet(amountOption))
        manager.setAmount(cmdLineparser.value(amountOption).toInt());

    if (cmdLineparser.isSet(sortPositionOption)) {
        QString posString = cmdLineparser.value(sortPositionOption);
        QStringList list = posString.split(QLatin1Char(','));
        QVector3D posVector;
        if (list.size() > 0)
            posVector.setX(list.at(0).toFloat());
        if (list.size() > 1)
            posVector.setY(list.at(1).toFloat());
        if (list.size() > 2)
            posVector.setZ(list.at(2).toFloat());
        manager.setSortingPosition(posVector);
        // TODO: Add command-line option
        manager.setSortingMode(ShapeManager::SortingMode::DistanceClosestFirst);
    }

    QString outputFile = "out.cbor";
    if (cmdLineparser.isSet(outputOption))
        outputFile = cmdLineparser.value(outputOption);

    // Now generate data and store it in CBOR format
    manager.generateData();
    manager.saveShapeData(outputFile);

    if (cmdLineparser.isSet(dumpCborFileOption))
        manager.dumpOutput();

    app.exit(0);
    return 0;
}
