// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "satellitemodel.h"
#include "sortfiltermodel.h"

#include <QtGui/qfontdatabase.h>
#include <QtGui/qguiapplication.h>
#include <QtQml/qqmlapplicationengine.h>

#include <array>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    static const std::array<QString, 2> fonts {
        u":/fonts/TitilliumWeb-Regular.ttf"_s,
        u":/fonts/TitilliumWeb-SemiBold.ttf"_s
    };
    int fontId{-1};
    for (const auto &font : fonts) {
        fontId = QFontDatabase::addApplicationFont(font);
        if (fontId == -1) {
            qWarning("Failed to load application font. Default system font will be used");
            break;
        }
    }
    if (fontId != -1) {
        QFont appFont = app.font();
        appFont.setFamilies(QFontDatabase::applicationFontFamilies(fontId));
        app.setFont(appFont);
    }

    SatelliteModel satelliteModel;
    SortFilterModel sortFilterModel;
    sortFilterModel.setSourceModel(&satelliteModel);
    sortFilterModel.sort(0);

    QQmlApplicationEngine engine;
    engine.setInitialProperties({
        {u"satellitesModel"_s, QVariant::fromValue(&satelliteModel)},
        {u"sortFilterModel"_s, QVariant::fromValue(&sortFilterModel)}
    });
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     []() { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("SatelliteInformation", "Main");

    return app.exec();
}
