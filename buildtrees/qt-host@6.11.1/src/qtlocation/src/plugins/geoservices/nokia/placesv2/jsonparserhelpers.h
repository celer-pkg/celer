// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef JSONPARSERHELPERS_H
#define JSONPARSERHELPERS_H

#include <QtLocation/QPlaceContent>

QT_BEGIN_NAMESPACE

class QJsonArray;
class QJsonObject;
class QGeoCoordinate;
class QPlaceContactDetail;
class QPlaceImage;
class QPlaceReview;
class QPlaceEditorial;
class QPlaceCategory;
class QPlaceContentRequest;
class QPlaceManagerEngineNokiaV2;

QGeoCoordinate parseCoordinate(const QJsonArray &coordinateArray);
QPlaceSupplier parseSupplier(const QJsonObject &supplierObject,
                             const QPlaceManagerEngineNokiaV2 *engine);
QPlaceCategory parseCategory(const QJsonObject &categoryObject,
                             const QPlaceManagerEngineNokiaV2 *engine);
QList<QPlaceCategory> parseCategories(const QJsonArray &categoryArray,
                             const QPlaceManagerEngineNokiaV2 *engine);
QList<QPlaceContactDetail> parseContactDetails(const QJsonArray &contacts);

QPlaceContent parseImage(const QJsonObject &imageObject,
                         const QPlaceManagerEngineNokiaV2 *engine);
QPlaceContent parseReview(const QJsonObject &reviewObject,
                          const QPlaceManagerEngineNokiaV2 *engine);
QPlaceContent parseEditorial(const QJsonObject &editorialObject,
                             const QPlaceManagerEngineNokiaV2 *engine);

void parseCollection(QPlaceContent::Type type, const QJsonObject &object,
                     QPlaceContent::Collection *collection, int *totalCount,
                     QPlaceContentRequest *previous, QPlaceContentRequest *next,
                     const QPlaceManagerEngineNokiaV2 *engine);

QT_END_NAMESPACE

#endif // JSONPARSERHELPERS_H
