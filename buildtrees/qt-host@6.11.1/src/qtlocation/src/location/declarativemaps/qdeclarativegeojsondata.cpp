// Copyright (C) 2019 Julian Sherollari <jdotsh@gmail.com>
// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativegeojsondata_p.h"

#include <QtLocation/private/qgeojson_p.h>

QT_BEGIN_NAMESPACE

namespace extractor
{
    static bool hasProperties(QQuickItem *item)
    {
        QVariant props = item->property("props");
        return !props.isNull();
    }

    static QVariant toVariant(QObject *mapItem)
    {
        if (QDeclarativeGeoMapItemView *miv = qobject_cast<QDeclarativeGeoMapItemView *>(mapItem)) {
            return toVariant(miv);
        } else if (QDeclarativePolylineMapItem *polyline = qobject_cast<QDeclarativePolylineMapItem *>(mapItem)) {
            return toVariant(polyline);
        } else if (QDeclarativePolygonMapItem *polygon = qobject_cast<QDeclarativePolygonMapItem *>(mapItem)) {
            return toVariant(polygon);
        } else if (QDeclarativeCircleMapItem *circle = qobject_cast<QDeclarativeCircleMapItem *>(mapItem)) {
            return toVariant(circle); // If GeoJSON Point type is visualized in other ways, handle those types here instead.
        } else if (QDeclarativeRectangleMapItem *rectangle = qobject_cast<QDeclarativeRectangleMapItem *>(mapItem)) {
            return toVariant(rectangle); // For the self-drawn rectangles. Will be exported as Polygons
        }
        return QVariant();
    }

    static QVariantMap toVariant(QDeclarativePolygonMapItem *mapPolygon)
    {
        QVariantMap ls;
        ls[QStringLiteral("type")] = QStringLiteral("Polygon");
        ls[QStringLiteral("data")] = QVariant::fromValue(mapPolygon->geoShape());
        if (hasProperties(mapPolygon))
            ls[QStringLiteral("properties")] = mapPolygon->property("props").toMap();
        return ls;
    }

    static QVariantMap toVariant(QDeclarativePolylineMapItem *mapPolyline)
    {
        QVariantMap ls;
        ls[QStringLiteral("type")] = QStringLiteral("LineString");
        ls[QStringLiteral("data")] = QVariant::fromValue(mapPolyline->geoShape());
        if (hasProperties(mapPolyline))
            ls[QStringLiteral("properties")] = mapPolyline->property("props").toMap();
        return ls;
    }

    static QVariantMap toVariant(QDeclarativeCircleMapItem *mapCircle)
    {
        QVariantMap pt;
        pt[QStringLiteral("type")] = QStringLiteral("Point");
        pt[QStringLiteral("data")] = QVariant::fromValue(mapCircle->geoShape());
        QVariantMap propMap = mapCircle->property("props").toMap();
        propMap[QStringLiteral("radius")] = mapCircle->radius();
        pt[QStringLiteral("properties")] = propMap;
        return pt;
    }

    static QVariantMap toVariant(QDeclarativeRectangleMapItem *mapRectangle)
    {
        QVariantMap pt;
        pt[QStringLiteral("type")] = QStringLiteral("Polygon");
        QGeoRectangle rectanlge = mapRectangle->geoShape();
        QGeoPolygon poly;
        poly.addCoordinate(rectanlge.topLeft());
        poly.addCoordinate(rectanlge.topRight());
        poly.addCoordinate(rectanlge.bottomRight());
        poly.addCoordinate(rectanlge.bottomLeft());
        pt[QStringLiteral("data")] = QVariant::fromValue(poly);
        if (hasProperties(mapRectangle))
            pt[QStringLiteral("properties")] = mapRectangle->property("props").toMap();
        return pt;
    }
};


/*!
    \qmltype GeoJsonData
    \nativetype QDeclarativeGeoJsonData
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-maps
    \since QtLocation 6.7

    \brief A model to represent, load and save GeoJSON documents.


    The GeoJsonData type reads and writes GeoJson formatted documents.
    GeoJsonData has functions to open and save the model at the URL set
    to the \l sourceUrl property. The loaded model is internally represented by
    QVariant and binds to the \l model property. Use
    \l{Models and Views in Qt Quick#View Delegates}{Delegates} to visualize the items
    in a view.

    For information about GeoJSON, visit the \l{https://geojson.org/}{GeoJSON} website.

    \section1 GeoJSON Object

    The GeoJSON object is a valid JSON object that represents geometry, a feature, or a
    collection of geometries or features.

    A GeoJSON object must be one of these types:
    \list
        \li \c Point
        \li \c MultiPoint
        \li \c LineString
        \li \c MultiLineString
        \li \c Polygon
        \li \c MultiPolygon
        \li \c GeometryCollection
        \li \c Feature
        \li \c FeatureCollection
    \endlist

    To set the type, bind the \c type member to a GeoJSON type. The \c coordinates member can
    be a type of QGeoShape or a list, depending on the GeoJSON type. The \c Feature type has an additional
    \c geometry and \c properties member.

    A list of the geometric types and their equivalent QVariant representations:

    \list
    \li For a \c Point object, \c coordinates pairs with QGeoCircle.
        For example:
        \code
        {
            "type": "Point",
            "coordinates": [11, 60]
        }
        \endcode
        This GeoJSON object has a corresponding QVariantMap representation:
        \code
        {
            type: "Point"
            coordinates: QGeoCircle({11.000, 60.000}, -1)
        }
        \endcode

    \li For a \c LineString object, \c coordinates pairs with QGeoPath.
        For example:
        \code
        {
            "type" : "LineString",
            "coordinates" : [
            [13.5, 43],
            [10.73, 59.92]
            ]
        }
        \endcode
        This GeoJSON object has a corresponding QVariantMap representation:
        \code
        {
            type : "LineString"
            data : QGeoPath([{43.000, 13.500}, {59.920, 10.730}])
        }
        \endcode

    \li For a \c{Polygon} object, \c coordinates member pairs with QGeoPolygon (holes are possible). The polygon is a
        \e{linear ring}, whose final coordinate is the same as the first coordinate, thereby opening and closing
        the ring. The \c bbox member is an optional member and is for setting the area's range, useful for concave boundaries.
        For more information about the accepted polygon coordinates, read about the
        \l{https://datatracker.ietf.org/doc/html/rfc7946#section-3.1.6Polygon}{Polygon} type in the GeoJson specification.
        For example:
        \code
        {
            "type": "Polygon",
            "coordinates": [
                [
                    [17.13, 51.11],
                    [30.54, 50.42],
                    [26.70, 58.36],
                    [17.13, 51.11]
                ]
            ],
            "bbox": [50, -50, 10, -10]
        }
        \endcode
        This GeoJSON object has a corresponding QVariantMap representation:
        \code
        {
            type : "Polygon"
            coordinates : QGeoPolygon([{51.110, 17.130}, {50.420,30.540}, {58.360, 26.700}, {51.110, 17.130}])
        }
        \endcode

    \endlist

    For the \c MultiPoint, \c MultiLineString, and \c MultiPolygon types, \c coordinates pairs with a QVariantList.
    The list element is a QVariantMap containing geometry.

    \list
    \li For a \c MultiPoint object, \c coordinates pairs with a list of Point coordinates.
        For example:
        \code
        {
            "type": "MultiPoint",
            "coordinates": [
                [11, 60],
                [5.5, 60.3],
                [5.7, 58.90]
            ]
        }
        \endcode
        This GeoJSON object has a corresponding QVariantMap representation:
        \code
        {
            type : "MultiPoint"
            coordinates : [
                {
                    type : "Point"
                    coordinates : QGeoCircle({60.000, 11.000}, -1)
                },
                {
                    type : "Point"
                    coordinates : QGeoCircle({60.300, 5.500}, -1)
                },
                {
                    type : "Point"
                    coordinates : QGeoCircle({58.900, 5.700}, -1)
                }
                ]
        }
        \endcode

    \li For a \c MultiLineString object, \c coordinates pairs with a list of LineString coordinates.
        The following GeoJSON object constructs two non-parallel lines:
        \code
        {
            "type" : "MultiLineString",
            "coordinates" : [
            [[13.5, 43], [10.73, 59.92]],
            [[9.15, 45], [-3.15, 58.90]]
            ]
        }
        \endcode
        This GeoJSON object has a corresponding QVariantMap representation:
        \code
        {
            type : "MultiLineString"
            coordinates : [
                {
                    type : "LineString"
                    coordinates : QGeoPath([{45.000, 9.150}, {58.900, -3.150}])
                },
                {
                    type : "LineString"
                    coordinates : QGeoPath([{43.000, 13.500}, {59.920, 10.730}])
                }
            ]
        }
        \endcode

    \li For a \c MultiPolygon type, \c coordinates pairs with a list of Polygon coordinates.
        The polygon is a \e{linear ring} and the \c Polygon type has more information about accepted formats.
        The following GeoJSON object contains a list of two triangles:
        \code
        {
            "type" : "MultiPolygon",
            "coordinates" : [
            [
                [[17.13, 51.11],
                [30.54, 50.42],
                [26.74, 58.36],
                [17.13, 51.11]
                ]],
            [
                [[19.84, 41.33],
                [30.45, 49.26],
                [17.07, 50.10],
                [19.84, 41.33]
                ]]
            ]
        }
        \endcode
        This GeoJSON object has a corresponding QVariantMap representation:
        \code
        {
            type : "MultiPolygon"
            coordinates : [
                {
                    type : "Polygon"
                    coordinates : QGeoPolygon([{51.110, 17.130}, {50.420,30.540}, {58.360, 26.740}])
                },
                {
                    type : "Polygon"
                    coordinates : QGeoPolygon([{41.330, 19.840}, {49.260,30.450}, {50.100, 17.070}])
                }
                ]
        }
        \endcode

    \endlist

    The \c GeometryCollection type is a composition of other geometry types. The value of the
    \c geometries member is a QVariantList containing QVariantMaps of various types,
    including other GeometryCollection types.

    For example, the following \c GeometryCollection type contains several other geometries:
    \code
    {
        "type" : "GeometryCollection",
        "geometries" : [
            {
                "type" : "MultiPoint",
                "coordinates" : [
                    [11,60], [5.5,60.3], [5.7,58.90]
                ]
            },
            {
                "type" : "MultiLineString",
                "coordinates": [
                    [[13.5, 43], [10.73, 59.92]],
                    [[9.15, 45], [-3.15, 58.90]]
                ]
            },
            {
                "type" : "MultiPolygon",
                "coordinates" : [
                    [
                        [
                            [17.13, 51.11],
                            [30.54, 50.42],
                            [26.74, 58.36],
                            [17.13, 51.11]
                        ]
                    ],
                    [
                        [
                            [19.84, 41.33],
                            [30.45, 49.26],
                            [17.07, 50.10],
                            [19.84, 41.33]
                        ]
                    ]
                ]
            }
        ]
    }
    \endcode
    This GeoJSON object has a corresponding QVariantMap representation:
    \code
    {
      type : "GeometryCollection"
      coordinates : [
        {
          type : "MultiPolygon"
          coordinates : [
            {
              type : "Polygon"
              coordinates : QGeoPolygon([{41.330, 19.840}, {49.260, 30.450}, {50.100, 17.070}])
            },
            {
              type : "Polygon"
              coordinates : QGeoPolygon([{51.110, 17.130}, {50.420, 30.540}, {58.360, 26.740}])
            }
          ]
        }
        {
          type : "MultiLineString"
          coordinates : [
            {
              type : "LineString"
              coordinates : QGeoPath([{45.000, 9.150}, {58.900, -3.150}])
            },
            {
              type : "LineString"
              coordinates : QGeoPath([{43.000, 13.500}, {59.920, 10.730}])
            }
          ]
        }
        {
          type : "MultiPoint"
          coordinates : [
            {
              type : Point
              coordinates : QGeoCircle({58.900, 5.700}, -1)
            },
            {
              type : Point
              coordinates : QGeoCircle({60.300, 5.500}, -1)
            },
            {
              type : Point
              coordinates : QGeoCircle({60.000, 11.000}, -1)
            }
          ]
        }
      ]
    }
    \endcode

    The \c Feature type contains an additional \c geometry and \c properties
    member. The only way to distinguish a Feature object from other geometrical objects
    is to check for the existence of a \c properties node in the QVariantMap object.

    For example, the following \c Feature has a geometry and properties members:
    \code
    {
        "type": "Feature",
        "id": "Poly",
        "properties": {
            "name": "Poly",
            "text": "This is a Feature with a Polygon",
            "color": "limegreen"
        },
        "geometry": {
            "type": "Polygon",
            "coordinates": [
                [
                    [17.13, 51.11],
                    [30.54, 50.42],
                    [26.70, 58.36],
                    [17.13, 51.11]
                ],
                [
                    [23.46, 54.36],
                    [20.52, 51.91],
                    [28.25, 51.50],
                    [26.80, 54.36],
                    [23.46, 54.36]
                ]
            ]
        }
    }
    \endcode
    This GeoJSON object has a corresponding QVariantMap representation:
    \code
    {
      type : "Polygon"
      data : QGeoPolygon([{51.110, 17.130}, {50.420,30.540}, {58.360, 26.700}, {51.110, 17.130}])
      properties : {text : "This is a Feature with a Polygon"}
    }
    \endcode

    The \c FeatureCollection is a composition of Feature objects. THe \c features
    member binds to a QVariantList object containing other Feature objects.

    For example, the following \c FeatureCollection has several Features and geometries:
    \code
    {
        "type" : "FeatureCollection",
        "properties" : {
            "color" : "crimson"
        },
        "features" : [
            {
                "type" : "Feature",
                "id" : "Poly",
                "properties" : {
                    "text" : "This is a Feature with a Polygon"
                },
                "geometry" : {
                "type" : "Polygon",
                "coordinates" : [
                    [
                        [17.13, 51.11],
                        [30.54, 50.42],
                        [26.70, 58.36],
                        [17.13, 51.11]
                    ],
                    [
                        [23.46, 54.36],
                        [20.52, 51.91],
                        [28.25, 51.50],
                        [26.80, 54.36],
                        [23.46, 54.36]
                    ]
                ]
            }
        },
        {
            "type" : "Feature",
            "id" : "MultiLine",
            "properties" : {
                "text" : "This is a Feature with a MultiLineString",
                "color" : "deepskyblue"
            },
            "geometry" : {
                "type" : "MultiLineString",
                "coordinates" : [
                    [[13.5, 43], [10.73, 59.92]],
                    [[9.15, 45], [-3.15, 58.90]]
                ]
            }
        }
        ]
    }
    \endcode
    This GeoJSON object has a corresponding QVariantMap representation:
    \code
    {
        type: "FeatureCollection"
        data: [{
                type: "MultiLineString"
                data: [{
                    type: "LineString"
                    data: QGeoPath( [{45.000, 9.150}, {58.900, -3.150}] )
                } {
                    type: "LineString"
                    data: QGeoPath( [{43.000, 13.500}, {59.920, 10.730}] )
                }]
                properties: { text: "This is a Feature with a MultiLineString" }
            },
            {
                type: "Polygon"
                data: QGeoPolygon(  {51.110, 17.130},
                                    {50.420, 30.540},
                                    {58.360, 26.700},
                                    {51.110, 17.130}
                                )
                properties: { text: "This is a Feature with a Polygon" }
            }
        ]
    }
    \endcode

    \section1 GeoJson Example

    The \l{GeoJson Viewer (QML)}{GeoJson Viewer} example demonstrates the use of the GeoJsonData QML type to
    load and visualize coordinates on a map.
*/


QDeclarativeGeoJsonData::QDeclarativeGeoJsonData(QObject *parent)
    : QObject(parent)
{

}

QDeclarativeGeoJsonData::~QDeclarativeGeoJsonData()
{

}

/*!
    \qmlproperty QVariant QtLocation::GeoJsonData::model

    A QVariant representation of the GeoJSON document. QML
    delegates can display the contents in views.

    \since 6.7
*/
QVariant QDeclarativeGeoJsonData::model() const
{
    return m_content;
}

void QDeclarativeGeoJsonData::setModel(const QVariant &model)
{
    m_content = model;
    emit modelChanged();
}


/*!
    \qmlproperty url QtLocation::GeoJsonData::sourceUrl

    The URL of a GeoJSON document. Setting this property loads the
    document and binds the object to the \l model member.

    \since 6.7
*/


QUrl QDeclarativeGeoJsonData::sourceUrl() const
{
    return m_url;
}

/*!
    \qmlmethod void QtLocation::GeoJsonData::clear()

    Deletes all items bound to the \l model.
*/
void QDeclarativeGeoJsonData::clear()
{
    m_content = QVariantList();
    emit modelChanged();
}

/*!
    \qmlmethod bool QtLocation::GeoJsonData::addItem(Item item)

    Adds the \a item to the \l model object.

    Returns \c true if adding is successful, \c false otherwise.
*/
void QDeclarativeGeoJsonData::addItem(QQuickItem *item)
{
    QVariant entry;
    if (auto *polyline = qobject_cast<QDeclarativePolylineMapItem *>(item))
        entry = extractor::toVariant(polyline);
    else if (auto *polygon = qobject_cast<QDeclarativePolygonMapItem *>(item))
        entry = extractor::toVariant(polygon);
    else if (auto *circle = qobject_cast<QDeclarativeCircleMapItem *>(item))
        entry = extractor::toVariant(circle);
    else if (auto *rectangle = qobject_cast<QDeclarativeRectangleMapItem *>(item))
        entry = extractor::toVariant(rectangle);
    else
        return;

    QVariantList geoJson = m_content.toList();
    if (!geoJson.isEmpty()){
        QVariantList geoData = (geoJson[0].toMap()[QStringLiteral("type")] == QStringLiteral("FeatureCollection")) ? geoJson[0].toMap()[QStringLiteral("data")].toList() : geoJson;
        geoData.append(entry);
        geoJson[0] = QVariantMap{{QStringLiteral("type"), QStringLiteral("FeatureCollection")}, {QStringLiteral("data"), geoData}};
    }
    else {
        geoJson.append(entry);
    }
    m_content = geoJson;
    emit modelChanged();
}

/*!
    \qmlmethod bool QtLocation::GeoJsonData::open()

    Loads the content of the file at \l sourceUrl.

    Returns \c true if opening is successful, \c false otherwise.
*/
bool QDeclarativeGeoJsonData::open()
{
    return openUrl(m_url);
}

/*!
    \qmlmethod bool QtLocation::GeoJsonData::openUrl(Url url)

    Loads the GeoJson document at \a url and binds it to the \l model. The property
    \l sourceUrl is set to \a url if opening the file is successful.

    Returns \c true if opening is successful, \c false otherwise.
*/
bool QDeclarativeGeoJsonData::openUrl(const QUrl &url)
{
    // Reading GeoJSON file
    QFile loadFile(url.toLocalFile());
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Error while opening the file: " << url;
        qWarning() << loadFile.errorString();
        return false;
    }

    // Load the GeoJSON file using Qt's API
    QJsonParseError err;
    QJsonDocument loadDoc(QJsonDocument::fromJson(loadFile.readAll(), &err));
    if (err.error) {
         qWarning() << "Parsing while importing the JSON document:\n" << err.errorString();
         return false;
    }

    // Import geographic data to a QVariantList
    m_content = QGeoJson::importGeoJson(loadDoc);
    if (m_url != url) {
        m_url = url;
        emit sourceUrlChanged();
    }
    emit modelChanged();
    return true;
}

/*!
    \qmlmethod bool QtLocation::GeoJsonData::saveAs(Url url)

    Saves the \l model at \a url.
    The \l sourceUrl property is set to \a url if successful.

    Returns \c true if saving is successful, \c false otherwise.
*/
bool QDeclarativeGeoJsonData::saveAs(const QUrl &url)
{
    if (m_url != url){
        m_url = url;
        emit sourceUrlChanged();
    }
    return save();
}

/*!
    \qmlmethod bool QtLocation::GeoJsonData::save()

    Saves the model at \l{sourceUrl}.

    Returns \c true if saving is successful, \c false otherwise.
*/
bool QDeclarativeGeoJsonData::save()
{
    return dumpGeoJSON(m_content.toList(), m_url);
}

/*!
    \qmlmethod void QtLocation::GeoJsonData::setModelToMapContents(MapView mapItemView)

    Adds all map items of \a mapItemView to the \l model of the GeoJsonData
    object. Deletes previously stored map items from the \l model.

    Returns \c true if setting is successful, \c false otherwise.

    \sa addItem
*/
void QDeclarativeGeoJsonData::setModelToMapContents(QDeclarativeGeoMap *map)
{
    m_content = toVariant(map);
    emit modelChanged();
}

/*! \internal
    A helper function to convert the children of a map to a \l QVariantList that
    represents the items and that can be exported to a Json file.

    Returns a \l QVariantList containing all data of the mapItems.
*/
QVariantList QDeclarativeGeoJsonData::toVariant(QDeclarativeGeoMap *map)
{
    QVariantList res;
    const QList<QObject*> mapChildren = map->children();
    for (QObject *child : mapChildren) {
        QVariant mapItemAsVariant = extractor::toVariant(child);
        if (mapItemAsVariant.isValid())
            res.append(mapItemAsVariant);
    }
    return res;
}

/*! \internal
    A helper function to dump the VariantList \a geoJson into the file at \a url.

    Returns \c true if the file was saved successfully, \c false otherwise.
*/
bool QDeclarativeGeoJsonData::dumpGeoJSON(const QVariantList &geoJson, const QUrl &url)
{
    QJsonDocument json = QGeoJson::exportGeoJson(geoJson);
    QFile jsonFile(url.toLocalFile());
    if (!jsonFile.open(QIODevice::WriteOnly))
        return false;
    jsonFile.write(json.toJson());
    jsonFile.close();
    return true;
}

/*! \internal
    A helper function to write a human interpretable representation of
    \a geoJson into the file at \a url.

    Returns \c true if the file was saved successfully, \c false otherwise.
*/
bool QDeclarativeGeoJsonData::writeDebug(const QVariantList &geoJson, const QUrl &url)
{
    QString prettyPrint = QGeoJson::toString(geoJson);
    QFile debugFile(url.toLocalFile());
    if (!debugFile.open(QIODevice::WriteOnly))
        return false;
    debugFile.write(prettyPrint.toUtf8());
    debugFile.close();
    return true;
}

/*! \internal
    A helper function to generate a human interpretable representation of
    \a geoJson.

    Returns a string with the human interpretable representation of \a geoJson.
*/
QString QDeclarativeGeoJsonData::toString(const QVariantList &geoJson)
{
    return QGeoJson::toString(geoJson);
}

QT_END_NAMESPACE
