// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeotiledmap_nokia.h"
#include "qgeotiledmappingmanagerengine_nokia.h"

#include <QDebug>
#include <QObject>
#include <QColor>
#include <QFont>
#include <QPainter>
#include <QImage>
#include <QRect>

#include <QStaticText>

#include <QtLocation/private/qgeocameradata_p.h>

QT_BEGIN_NAMESPACE

/*
 Constructs a new tiled map data object, which stores the map data required by
 \a geoMap and makes use of the functionality provided by \a engine.
 */
QGeoTiledMapNokia::QGeoTiledMapNokia(QGeoTiledMappingManagerEngineNokia *engine, QObject *parent /*= 0*/) :
    Map(engine, parent),
    m_logo(":/nokia/logo.png"), // HERE logo image
    m_engine(engine)
{}

QGeoTiledMapNokia::~QGeoTiledMapNokia() {}

void QGeoTiledMapNokia::evaluateCopyrights(const QSet<QGeoTileSpec> &visibleTiles)
{
    const int spaceToLogo = 4;
    const int blurRate = 1;
    const int fontSize = 10;

    if (m_engine.isNull())
        return;

    const QString copyrightsString = m_engine->evaluateCopyrightsText(activeMapType(), cameraData().zoomLevel(), visibleTiles);

    if (viewportWidth() > 0 && viewportHeight() > 0 && ((copyrightsString.isNull() && m_copyrightsSlab.isNull()) || copyrightsString != m_lastCopyrightsString)) {
        QFont font("Sans Serif");
        font.setPixelSize(fontSize);
        font.setStyleHint(QFont::SansSerif);
        font.setWeight(QFont::Bold);

        QRect textBounds = QFontMetrics(font).boundingRect(0, 0, viewportWidth(), viewportHeight(), Qt::AlignBottom | Qt::AlignLeft | Qt::TextWordWrap, copyrightsString);

        m_copyrightsSlab = QImage(m_logo.width() + textBounds.width() + spaceToLogo + blurRate * 2,
                                qMax(m_logo.height(), textBounds.height() + blurRate * 2),
                                QImage::Format_ARGB32_Premultiplied);
        m_copyrightsSlab.fill(Qt::transparent);

        QPainter painter(&m_copyrightsSlab);
        painter.drawImage(QPoint(0, m_copyrightsSlab.height() - m_logo.height()), m_logo);
        painter.setFont(font);
        painter.setPen(QColor(0, 0, 0, 64));
        painter.translate(spaceToLogo + m_logo.width(), -blurRate);
        for (int x=-blurRate; x<=blurRate; ++x) {
            for (int y=-blurRate; y<=blurRate; ++y) {
                painter.drawText(x, y, textBounds.width(), m_copyrightsSlab.height(),
                                 Qt::AlignBottom | Qt::AlignLeft | Qt::TextWordWrap,
                                 copyrightsString);
            }
        }
        painter.setPen(Qt::white);
        painter.drawText(0, 0, textBounds.width(), m_copyrightsSlab.height(),
                         Qt::AlignBottom | Qt::AlignLeft | Qt::TextWordWrap,
                         copyrightsString);
        painter.end();

        m_lastCopyrightsString = copyrightsString;
    }

    emit copyrightsImageChanged(m_copyrightsSlab);
}

QT_END_NAMESPACE
