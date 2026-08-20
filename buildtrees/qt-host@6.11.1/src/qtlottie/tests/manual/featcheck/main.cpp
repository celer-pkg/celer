// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

#include <QtLottie/private/qlottiebase_p.h>
#include <QtLottie/private/qlottielayer_p.h>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc < 2) {
        printf("Filename missing\n");
        return -1;
    }

    QFile sourceFile(argv[1]);

    if (!sourceFile.open(QIODevice::ReadOnly)) {
        fprintf(stderr, "Cannot open file\n");
        return -1;
    }

    QByteArray json = sourceFile.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(json);
    QJsonObject rootObj = doc.object();

    if (rootObj.empty()) {
        fprintf(stderr, "Json document not found\n");
        return -1;
    }

    if (rootObj.value(QLatin1String("assets")).toArray().count())
        qWarning() << "assets not supported";

    if (rootObj.value(QLatin1String("chars")).toArray().count())
        qWarning() << "chars not supported";

    if (rootObj.value(QLatin1String("markers")).toArray().count())
        qWarning() << "markers not supported";

    QMap<QString, QJsonObject> assets;
    QJsonArray jsonLayers = rootObj.value(QLatin1String("layers")).toArray();
    QJsonArray jsonAssets = rootObj.value(QLatin1String("assets")).toArray();
    QJsonArray::const_iterator jsonAssetsIt = jsonAssets.constBegin();
    while (jsonAssetsIt != jsonAssets.constEnd()) {
        QJsonObject jsonAsset = (*jsonAssetsIt).toObject();

        jsonAsset.insert(QLatin1String("fileSource"), QJsonValue::fromVariant(sourceFile.fileName()));
        QString id = jsonAsset.value(QLatin1String("id")).toString();
        assets.insert(id, jsonAsset);
        jsonAssetsIt++;
    }
    QJsonArray::const_iterator jsonLayerIt = jsonLayers.constBegin();
    while (jsonLayerIt != jsonLayers.constEnd()) {
        QJsonObject jsonLayer = (*jsonLayerIt).toObject();
        QLottieLayer *layer = QLottieLayer::construct(jsonLayer, assets);
        if (layer)
            delete layer;
        jsonLayerIt++;
    }
}
