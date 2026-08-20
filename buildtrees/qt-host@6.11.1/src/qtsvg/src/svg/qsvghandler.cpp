// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qplatformdefs.h"

#include "qsvghandler_p.h"

#include "qsvgdocument_p.h"
#include "qsvgstructure_p.h"
#include "qsvggraphics_p.h"
#include "qsvgfilter_p.h"
#include "qsvgnode_p.h"
#include "qsvgfont_p.h"
#include "qsvganimate_p.h"

#include "qpen.h"
#include "qpainterpath.h"
#include "qbrush.h"
#include "qcolor.h"
#include "qtextformat.h"

#include <QtCore/private/qdataurl_p.h>
#include "qlist.h"
#include "qfileinfo.h"
#include "qfile.h"
#include "qdir.h"
#include "qdebug.h"
#include "qmath.h"
#include "qnumeric.h"
#include <qregularexpression.h>
#include "qtransform.h"
#include "qvarlengtharray.h"
#include "qimagereader.h"

#include "float.h"

#include <algorithm>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcSvgHandler, "qt.svg")

static const char *qt_inherit_text = "inherit";
#define QT_INHERIT QLatin1String(qt_inherit_text)

static QByteArray prefixMessage(const QByteArray &msg, const QXmlStreamReader *r)
{
    QByteArray result;
    if (r) {
        if (const QFile *file = qobject_cast<const QFile *>(r->device()))
            result.append(QFile::encodeName(QDir::toNativeSeparators(file->fileName())));
        else
            result.append(QByteArrayLiteral("<input>"));
        result.append(':');
        result.append(QByteArray::number(r->lineNumber()));
        if (const qint64 column = r->columnNumber()) {
            result.append(':');
            result.append(QByteArray::number(column));
        }
        result.append(QByteArrayLiteral(": "));
    }
    result.append(msg);
    return result;
}

static inline QByteArray msgProblemParsing(QStringView localName, const QXmlStreamReader *r)
{
    return prefixMessage("Problem parsing " + localName.toLocal8Bit(), r);
}

static inline QByteArray msgCouldNotResolveProperty(QStringView id, const QXmlStreamReader *r)
{
    return prefixMessage("Could not resolve property: " + id.toLocal8Bit(), r);
}

static QList<QStringView> splitWithDelimiter(QStringView delimitedList)
{
    static const QRegularExpression delimiterRE(QStringLiteral("[,\\s]+"));
    return delimitedList.split(delimiterRE, Qt::SkipEmptyParts);
}

// ======== duplicated from qcolor_p

static inline int qsvg_h2i(char hex, bool *ok = nullptr)
{
    if (hex >= '0' && hex <= '9')
        return hex - '0';
    if (hex >= 'a' && hex <= 'f')
        return hex - 'a' + 10;
    if (hex >= 'A' && hex <= 'F')
        return hex - 'A' + 10;
    if (ok)
        *ok = false;
    return -1;
}

static inline int qsvg_hex2int(const char *s, bool *ok = nullptr)
{
    return (qsvg_h2i(s[0], ok) * 16) | qsvg_h2i(s[1], ok);
}

static inline int qsvg_hex2int(char s, bool *ok = nullptr)
{
    int h = qsvg_h2i(s, ok);
    return (h * 16) | h;
}

bool qsvg_get_hex_rgb(const char *name, QRgb *rgb)
{
    if(name[0] != '#')
        return false;
    name++;
    const size_t len = qstrlen(name);
    int r, g, b;
    bool ok = true;
    if (len == 12) {
        r = qsvg_hex2int(name, &ok);
        g = qsvg_hex2int(name + 4, &ok);
        b = qsvg_hex2int(name + 8, &ok);
    } else if (len == 9) {
        r = qsvg_hex2int(name, &ok);
        g = qsvg_hex2int(name + 3, &ok);
        b = qsvg_hex2int(name + 6, &ok);
    } else if (len == 6) {
        r = qsvg_hex2int(name, &ok);
        g = qsvg_hex2int(name + 2, &ok);
        b = qsvg_hex2int(name + 4, &ok);
    } else if (len == 3) {
        r = qsvg_hex2int(name[0], &ok);
        g = qsvg_hex2int(name[1], &ok);
        b = qsvg_hex2int(name[2], &ok);
    } else {
        r = g = b = -1;
    }
    if ((uint)r > 255 || (uint)g > 255 || (uint)b > 255 || !ok) {
        *rgb = 0;
        return false;
    }
    *rgb = qRgb(r, g ,b);
    return true;
}

bool qsvg_get_hex_rgb(const QChar *str, int len, QRgb *rgb)
{
    if (len > 13)
        return false;
    char tmp[16];
    for(int i = 0; i < len; ++i)
        tmp[i] = str[i].toLatin1();
    tmp[len] = 0;
    return qsvg_get_hex_rgb(tmp, rgb);
}

// ======== end of qcolor_p duplicate

static inline QString someId(const QXmlStreamAttributes &attributes)
{
    QStringView id = attributes.value(QLatin1String("id"));
    if (id.isEmpty())
        id = attributes.value(QLatin1String("xml:id"));
    return id.toString();
}

struct QSvgAttributes
{
    QSvgAttributes(const QXmlStreamAttributes &xmlAttributes, QSvgHandler *handler);
    void setAttributes(const QXmlStreamAttributes &attributes, QSvgHandler *handler);

    QString id;

    QStringView color;
    QStringView colorOpacity;
    QStringView fill;
    QStringView fillRule;
    QStringView fillOpacity;
    QStringView stroke;
    QStringView strokeDashArray;
    QStringView strokeDashOffset;
    QStringView strokeLineCap;
    QStringView strokeLineJoin;
    QStringView strokeMiterLimit;
    QStringView strokeOpacity;
    QStringView strokeWidth;
    QStringView vectorEffect;
    QStringView fontFamily;
    QStringView fontSize;
    QStringView fontStyle;
    QStringView fontWeight;
    QStringView fontVariant;
    QStringView textAnchor;
    QStringView transform;
    QStringView visibility;
    QStringView opacity;
    QStringView compOp;
    QStringView display;
    QStringView offset;
    QStringView stopColor;
    QStringView stopOpacity;
    QStringView imageRendering;
    QStringView mask;
    QStringView markerStart;
    QStringView markerMid;
    QStringView markerEnd;
    QStringView filter;
};

QSvgAttributes::QSvgAttributes(const QXmlStreamAttributes &xmlAttributes, QSvgHandler *handler)
{
    setAttributes(xmlAttributes, handler);
}

void QSvgAttributes::setAttributes(const QXmlStreamAttributes &attributes, QSvgHandler *handler)
{
    for (const QXmlStreamAttribute &attribute : attributes) {
        QStringView name = attribute.qualifiedName();
        if (name.isEmpty())
            continue;
        QStringView value = attribute.value();

        switch (name.at(0).unicode()) {

        case 'c':
            if (name == QLatin1String("color"))
                color = value;
            else if (name == QLatin1String("color-opacity"))
                colorOpacity = value;
            else if (name == QLatin1String("comp-op"))
                compOp = value;
            break;

        case 'd':
            if (name == QLatin1String("display"))
                display = value;
            break;

        case 'f':
            if (name == QLatin1String("fill"))
                fill = value;
            else if (name == QLatin1String("fill-rule"))
                fillRule = value;
            else if (name == QLatin1String("fill-opacity"))
                fillOpacity = value;
            else if (name == QLatin1String("font-family"))
                fontFamily = value;
            else if (name == QLatin1String("font-size"))
                fontSize = value;
            else if (name == QLatin1String("font-style"))
                fontStyle = value;
            else if (name == QLatin1String("font-weight"))
                fontWeight = value;
            else if (name == QLatin1String("font-variant"))
                fontVariant = value;
            else if (name == QLatin1String("filter") &&
                     !handler->options().testFlag(QtSvg::Tiny12FeaturesOnly))
                filter = value;
            break;

        case 'i':
            if (name == QLatin1String("id"))
                id = value.toString();
            else if (name == QLatin1String("image-rendering"))
                imageRendering = value;
            break;

        case 'm':
            if (name == QLatin1String("mask") &&
                !handler->options().testFlag(QtSvg::Tiny12FeaturesOnly))
                mask = value;
            if (name == QLatin1String("marker-start") &&
                !handler->options().testFlag(QtSvg::Tiny12FeaturesOnly))
                markerStart = value;
            if (name == QLatin1String("marker-mid") &&
                !handler->options().testFlag(QtSvg::Tiny12FeaturesOnly))
                markerMid = value;
            if (name == QLatin1String("marker-end") &&
                !handler->options().testFlag(QtSvg::Tiny12FeaturesOnly))
                markerEnd = value;
            break;

        case 'o':
            if (name == QLatin1String("opacity"))
                opacity = value;
            if (name == QLatin1String("offset"))
                offset = value;
            break;

        case 's':
            if (name.size() > 5 && name.mid(1, 5) == QLatin1String("troke")) {
                QStringView strokeRef = name.mid(6, name.size() - 6);
                if (strokeRef.isEmpty())
                    stroke = value;
                else if (strokeRef == QLatin1String("-dasharray"))
                    strokeDashArray = value;
                else if (strokeRef == QLatin1String("-dashoffset"))
                    strokeDashOffset = value;
                else if (strokeRef == QLatin1String("-linecap"))
                    strokeLineCap = value;
                else if (strokeRef == QLatin1String("-linejoin"))
                    strokeLineJoin = value;
                else if (strokeRef == QLatin1String("-miterlimit"))
                    strokeMiterLimit = value;
                else if (strokeRef == QLatin1String("-opacity"))
                    strokeOpacity = value;
                else if (strokeRef == QLatin1String("-width"))
                    strokeWidth = value;
            } else if (name == QLatin1String("stop-color"))
                stopColor = value;
            else if (name == QLatin1String("stop-opacity"))
                stopOpacity = value;
            break;

        case 't':
            if (name == QLatin1String("text-anchor"))
                textAnchor = value;
            else if (name == QLatin1String("transform"))
                transform = value;
            break;

        case 'v':
            if (name == QLatin1String("vector-effect"))
                vectorEffect = value;
            else if (name == QLatin1String("visibility"))
                visibility = value;
            break;

        case 'x':
            if (name == QLatin1String("xml:id") && id.isEmpty())
                id = value.toString();
            break;

        default:
            break;
        }
    }
}

static QList<qreal> parseNumbersList(QStringView *str)
{
    QList<qreal> points;
    if (!str)
        return points;
    points.reserve(32);

    while (!str->isEmpty() && str->first().isSpace())
        str->slice(1);
    while (!str->isEmpty()
           && (QSvgUtils::isDigit(str->first().unicode()) || str->startsWith(QLatin1Char('-'))
               || str->startsWith(QLatin1Char('+')) || str->startsWith(QLatin1Char('.')))) {

        points.append(QSvgUtils::toDouble(str));

        while (!str->isEmpty() && str->first().isSpace())
            str->slice(1);
        if (str->startsWith(QLatin1Char(',')))
            str->slice(1);

        //eat the rest of space
        while (!str->isEmpty() && str->first().isSpace())
            str->slice(1);
    }

    return points;
}

static QList<qreal> parsePercentageList(QStringView str)
{
    QList<qreal> points;

    while (!str.isEmpty() && str.first().isSpace())
        str.slice(1);
    while ((!str.isEmpty() && str.first() >= QLatin1Char('0') && str.first() <= QLatin1Char('9'))
           || str.startsWith(QLatin1Char('-')) || str.startsWith(QLatin1Char('+'))
           || str.startsWith(QLatin1Char('.'))) {

        points.append(QSvgUtils::toDouble(&str));

        while (!str.isEmpty() && str.first().isSpace())
            str.slice(1);
        if (str.startsWith(QLatin1Char('%')))
            str.slice(1);
        while (!str.isEmpty() && str.first().isSpace())
            str.slice(1);
        if (str.startsWith(QLatin1Char(',')))
            str.slice(1);

        //eat the rest of space
        while (!str.isEmpty() && str.first().isSpace())
            str.slice(1);
    }

    return points;
}

static QStringView idFromUrl(QStringView url)
{
    // The form is url(<IRI>), where IRI can be
    // just an ID on #<id> form.
    url = url.trimmed();
    if (url.startsWith(QLatin1Char('(')))
        url.slice(1);
    else
        return QStringView();
    url = url.trimmed();
    if (!url.startsWith(QLatin1Char('#')))
        return QStringView();
    const qsizetype closingBracePos = url.indexOf(QLatin1Char(')'));
    if (closingBracePos == -1)
        return QStringView();
    return url.first(closingBracePos).trimmed();
}

/**
 * returns true when successfully set the color. false signifies
 * that the color should be inherited
 */
static bool resolveColor(QStringView colorStr, QColor &color, QSvgHandler *handler)
{
    QStringView colorStrTr = colorStr.trimmed();
    if (colorStrTr.isEmpty())
        return false;

    switch(colorStrTr.at(0).unicode()) {

        case '#':
            {
                // #rrggbb is very very common, so let's tackle it here
                // rather than falling back to QColor
                QRgb rgb;
                bool ok = qsvg_get_hex_rgb(colorStrTr.constData(), colorStrTr.size(), &rgb);
                if (ok)
                    color.setRgb(rgb);
                return ok;
            }
            break;

        case 'r':
            {
                // starts with "rgb(", ends with ")" and consists of at least 7 characters "rgb(,,)"
                if (colorStrTr.size() >= 7 && colorStrTr.at(colorStrTr.size() - 1) == QLatin1Char(')')
                    && colorStrTr.mid(0, 4) == QLatin1String("rgb(")) {
                    QStringView sv{ colorStrTr.sliced(4) };
                    QList<qreal> compo = parseNumbersList(&sv);
                    //1 means that it failed after reaching non-parsable
                    //character which is going to be "%"
                    if (compo.size() == 1) {
                        compo = parsePercentageList(colorStrTr.sliced(4));
                        for (int i = 0; i < compo.size(); ++i)
                            compo[i] *= (qreal)2.55;
                    }

                    if (compo.size() == 3) {
                        color = QColor(int(compo[0]),
                                       int(compo[1]),
                                       int(compo[2]));
                        return true;
                    }
                    return false;
                }
            }
            break;

        case 'c':
            if (colorStrTr == QLatin1String("currentColor")) {
                color = handler->currentColor();
                return true;
            }
            break;
        case 'i':
            if (colorStrTr == QT_INHERIT)
                return false;
            break;
        default:
            break;
    }

    color = QColor::fromString(colorStrTr);
    return color.isValid();
}

void setAlpha(QStringView opacity, QColor *color)
{
    bool ok = true;
    qreal op = qBound(qreal(0.0), QSvgUtils::toDouble(opacity, &ok), qreal(1.0));
    if (!ok)
        op = 1.0;
    color->setAlphaF(op);
}

static bool constructColor(QStringView colorStr, QStringView opacity,
                           QColor &color, QSvgHandler *handler)
{
    if (!resolveColor(colorStr, color, handler))
        return false;
    if (!opacity.isEmpty())
        setAlpha(opacity, &color);
    return true;
}

static inline qreal convertToNumber(QStringView str, bool *ok = NULL)
{
    QSvgUtils::LengthType type;
    qreal num = QSvgUtils::parseLength(str.toString(), &type, ok);
    if (type == QSvgUtils::LengthType::LT_PERCENT) {
        num = num/100.0;
    }
    return num;
}

static bool createSvgGlyph(QSvgFont *font, const QXmlStreamAttributes &attributes,
                           bool isMissingGlyph)
{
    QStringView uncStr = attributes.value(QLatin1String("unicode"));
    QStringView havStr = attributes.value(QLatin1String("horiz-adv-x"));
    QStringView pathStr = attributes.value(QLatin1String("d"));

    qreal havx = (havStr.isEmpty()) ? -1 : QSvgUtils::toDouble(havStr);
    QPainterPath path = QSvgUtils::parsePathDataFast(pathStr).value_or(QPainterPath());

    path.setFillRule(Qt::WindingFill);

    if (isMissingGlyph) {
        if (!uncStr.isEmpty())
            qWarning("Ignoring missing-glyph's 'unicode' attribute");
        return font->addMissingGlyph(path, havx);
    }

    if (uncStr.isEmpty()) {
        qWarning("glyph does not define a non-empty 'unicode' attribute and will be ignored");
        return false;
    }
    font->addGlyph(uncStr.toString(), path, havx);
    return true;
}

static void parseColor(QSvgNode *,
                       const QSvgAttributes &attributes,
                       QSvgHandler *handler)
{
    QColor color;
    if (constructColor(attributes.color, attributes.colorOpacity, color, handler)) {
        handler->popColor();
        handler->pushColor(color);
    }
}

static QSvgStyleProperty *styleFromUrl(QSvgNode *node, QStringView url)
{
    return node ? node->styleProperty(idFromUrl(url)) : 0;
}

static void parseBrush(QSvgNode *node,
                       const QSvgAttributes &attributes,
                       QSvgHandler *handler)
{
    if (!attributes.fill.isEmpty() || !attributes.fillRule.isEmpty() || !attributes.fillOpacity.isEmpty()) {
        QSvgFillStyle *prop = new QSvgFillStyle;

        //fill-rule attribute handling
        if (!attributes.fillRule.isEmpty() && attributes.fillRule != QT_INHERIT) {
            if (attributes.fillRule == QLatin1String("evenodd"))
                prop->setFillRule(Qt::OddEvenFill);
            else if (attributes.fillRule == QLatin1String("nonzero"))
                prop->setFillRule(Qt::WindingFill);
        }

        //fill-opacity attribute handling
        if (!attributes.fillOpacity.isEmpty() && attributes.fillOpacity != QT_INHERIT) {
            prop->setFillOpacity(qMin(qreal(1.0), qMax(qreal(0.0), QSvgUtils::toDouble(attributes.fillOpacity))));
        }

        //fill attribute handling
        if ((!attributes.fill.isEmpty()) && (attributes.fill != QT_INHERIT) ) {
            if (attributes.fill.startsWith(QLatin1String("url"))) {
                QStringView value = attributes.fill.sliced(3);
                QSvgStyleProperty *style = styleFromUrl(node, value);
                if (style) {
                    if (style->type() == QSvgStyleProperty::SOLID_COLOR || style->type() == QSvgStyleProperty::GRADIENT
                            || style->type() == QSvgStyleProperty::PATTERN)
                        prop->setFillStyle(reinterpret_cast<QSvgPaintStyleProperty *>(style));
                } else {
                    QString id = idFromUrl(value).toString();
                    prop->setPaintStyleId(id);
                    prop->setPaintStyleResolved(false);
                }
            } else if (attributes.fill != QLatin1String("none")) {
                QColor color;
                if (resolveColor(attributes.fill, color, handler))
                    prop->setBrush(QBrush(color));
            } else {
                prop->setBrush(QBrush(Qt::NoBrush));
            }
        }
        node->appendStyleProperty(prop, attributes.id);
    }
}



static QTransform parseTransformationMatrix(QStringView value)
{
    if (value.isEmpty())
        return QTransform();

    QTransform matrix;

    while (!value.isEmpty()) {
        if (value.first().isSpace() || value.startsWith(QLatin1Char(','))) {
            value.slice(1);
            continue;
        }
        enum State {
            Matrix,
            Translate,
            Rotate,
            Scale,
            SkewX,
            SkewY
        };
        State state = Matrix;
        if (value.startsWith(QLatin1Char('m'))) { //matrix
            const char *ident = "atrix";
            for (int i = 0; i < 5; ++i)
                if (!value.slice(1).startsWith(QLatin1Char(ident[i])))
                    goto error;
            value.slice(1);
            state = Matrix;
        } else if (value.startsWith(QLatin1Char('t'))) { //translate
            const char *ident = "ranslate";
            for (int i = 0; i < 8; ++i)
                if (!value.slice(1).startsWith(QLatin1Char(ident[i])))
                    goto error;
            value.slice(1);
            state = Translate;
        } else if (value.startsWith(QLatin1Char('r'))) { //rotate
            const char *ident = "otate";
            for (int i = 0; i < 5; ++i)
                if (!value.slice(1).startsWith(QLatin1Char(ident[i])))
                    goto error;
            value.slice(1);
            state = Rotate;
        } else if (value.startsWith(QLatin1Char('s'))) { //scale, skewX, skewY
            value.slice(1);
            if (value.startsWith(QLatin1Char('c'))) {
                const char *ident = "ale";
                for (int i = 0; i < 3; ++i)
                    if (!value.slice(1).startsWith(QLatin1Char(ident[i])))
                        goto error;
                value.slice(1);
                state = Scale;
            } else if (value.startsWith(QLatin1Char('k'))) {
                if (!value.slice(1).startsWith(QLatin1Char('e')))
                    goto error;
                if (!value.slice(1).startsWith(QLatin1Char('w')))
                    goto error;
                value.slice(1);
                if (value.startsWith(QLatin1Char('X')))
                    state = SkewX;
                else if (value.startsWith(QLatin1Char('Y')))
                    state = SkewY;
                else
                    goto error;
                value.slice(1);
            } else {
                goto error;
            }
        } else {
            goto error;
        }

        while (!value.isEmpty() && value.first().isSpace())
            value.slice(1);
        if (!value.startsWith(QLatin1Char('(')))
            goto error;
        value.slice(1);
        QVarLengthArray<qreal, 8> points;
        QSvgUtils::parseNumbersArray(&value, points);
        if (!value.startsWith(QLatin1Char(')')))
            goto error;
        value.slice(1);

        if(state == Matrix) {
            if(points.size() != 6)
                goto error;
            matrix = QTransform(points[0], points[1],
                                points[2], points[3],
                                points[4], points[5]) * matrix;
        } else if (state == Translate) {
            if (points.size() == 1)
                matrix.translate(points[0], 0);
            else if (points.size() == 2)
                matrix.translate(points[0], points[1]);
            else
                goto error;
        } else if (state == Rotate) {
            if(points.size() == 1) {
                matrix.rotate(points[0]);
            } else if (points.size() == 3) {
                matrix.translate(points[1], points[2]);
                matrix.rotate(points[0]);
                matrix.translate(-points[1], -points[2]);
            } else {
                goto error;
            }
        } else if (state == Scale) {
            if (points.size() < 1 || points.size() > 2)
                goto error;
            qreal sx = points[0];
            qreal sy = sx;
            if(points.size() == 2)
                sy = points[1];
            matrix.scale(sx, sy);
        } else if (state == SkewX) {
            if (points.size() != 1)
                goto error;
            matrix.shear(qTan(qDegreesToRadians(points[0])), 0);
        } else if (state == SkewY) {
            if (points.size() != 1)
                goto error;
            matrix.shear(0, qTan(qDegreesToRadians(points[0])));
        }
    }
  error:
    return matrix;
}

static void parsePen(QSvgNode *node,
                     const QSvgAttributes &attributes,
                     QSvgHandler *handler)
{
    if (!attributes.stroke.isEmpty() || !attributes.strokeDashArray.isEmpty() || !attributes.strokeDashOffset.isEmpty() || !attributes.strokeLineCap.isEmpty()
        || !attributes.strokeLineJoin.isEmpty() || !attributes.strokeMiterLimit.isEmpty() || !attributes.strokeOpacity.isEmpty() || !attributes.strokeWidth.isEmpty()
        || !attributes.vectorEffect.isEmpty()) {

        QSvgStrokeStyle *prop = new QSvgStrokeStyle;

        //stroke attribute handling
        if ((!attributes.stroke.isEmpty()) && (attributes.stroke != QT_INHERIT) ) {
            if (attributes.stroke.startsWith(QLatin1String("url"))) {
                 QStringView value = attributes.stroke.sliced(3);
                    QSvgStyleProperty *style = styleFromUrl(node, value);
                    if (style) {
                        if (style->type() == QSvgStyleProperty::SOLID_COLOR || style->type() == QSvgStyleProperty::GRADIENT
                            || style->type() == QSvgStyleProperty::PATTERN)
                        prop->setStyle(reinterpret_cast<QSvgPaintStyleProperty *>(style));
                    } else {
                        QString id = idFromUrl(value).toString();
                        prop->setPaintStyleId(id);
                        prop->setPaintStyleResolved(false);
                    }
            } else if (attributes.stroke != QLatin1String("none")) {
                QColor color;
                if (resolveColor(attributes.stroke, color, handler))
                    prop->setStroke(QBrush(color));
            } else {
                prop->setStroke(QBrush(Qt::NoBrush));
            }
        }

        //stroke-width handling
        if (!attributes.strokeWidth.isEmpty() && attributes.strokeWidth != QT_INHERIT) {
            QSvgUtils::LengthType lt;
            prop->setWidth(QSvgUtils::parseLength(attributes.strokeWidth, &lt));
        }

        //stroke-dasharray
        if (!attributes.strokeDashArray.isEmpty() && attributes.strokeDashArray != QT_INHERIT) {
            if (attributes.strokeDashArray == QLatin1String("none")) {
                prop->setDashArrayNone();
            } else {
                QStringView dashArray = attributes.strokeDashArray;
                QList<qreal> dashes = parseNumbersList(&dashArray);
                const bool allZeroes = std::all_of(dashes.cbegin(), dashes.cend(),
                                                   [](qreal i) { return qFuzzyIsNull(i); });
                const bool hasNegative = !allZeroes && std::any_of(dashes.cbegin(), dashes.cend(),
                                                                   [](qreal i) { return i < 0.; });

                if (hasNegative)
                    qCWarning(lcSvgHandler) << "QSvgHandler: Stroke dash array "
                                               "with a negative value is invalid";
                // if the stroke dash array contains only zeros or a negative value,
                // force drawing of solid line.
                if (allZeroes || hasNegative) {
                    prop->setDashArrayNone();
                } else {
                    // if the dash count is odd the dashes should be duplicated
                    if ((dashes.size() & 1) != 0)
                        dashes << QList<qreal>(dashes);
                    prop->setDashArray(dashes);
                }
            }
        }

        //stroke-linejoin attribute handling
        if (!attributes.strokeLineJoin.isEmpty()) {
            if (attributes.strokeLineJoin == QLatin1String("miter"))
                prop->setLineJoin(Qt::SvgMiterJoin);
            else if (attributes.strokeLineJoin == QLatin1String("round"))
                prop->setLineJoin(Qt::RoundJoin);
            else if (attributes.strokeLineJoin == QLatin1String("bevel"))
                prop->setLineJoin(Qt::BevelJoin);
        }

        //stroke-linecap attribute handling
        if (!attributes.strokeLineCap.isEmpty()) {
            if (attributes.strokeLineCap == QLatin1String("butt"))
                prop->setLineCap(Qt::FlatCap);
            else if (attributes.strokeLineCap == QLatin1String("round"))
                prop->setLineCap(Qt::RoundCap);
            else if (attributes.strokeLineCap == QLatin1String("square"))
                prop->setLineCap(Qt::SquareCap);
        }

        //stroke-dashoffset attribute handling
        if (!attributes.strokeDashOffset.isEmpty() && attributes.strokeDashOffset != QT_INHERIT)
            prop->setDashOffset(QSvgUtils::toDouble(attributes.strokeDashOffset));

        //vector-effect attribute handling
        if (!attributes.vectorEffect.isEmpty()) {
            if (attributes.vectorEffect == QLatin1String("non-scaling-stroke"))
                prop->setVectorEffect(true);
            else if (attributes.vectorEffect == QLatin1String("none"))
                prop->setVectorEffect(false);
        }

        //stroke-miterlimit
        if (!attributes.strokeMiterLimit.isEmpty() && attributes.strokeMiterLimit != QT_INHERIT)
            prop->setMiterLimit(QSvgUtils::toDouble(attributes.strokeMiterLimit));

        //stroke-opacity atttribute handling
        if (!attributes.strokeOpacity.isEmpty() && attributes.strokeOpacity != QT_INHERIT)
            prop->setOpacity(qMin(qreal(1.0), qMax(qreal(0.0), QSvgUtils::toDouble(attributes.strokeOpacity))));

        node->appendStyleProperty(prop, attributes.id);
    }
}

enum FontSizeSpec { XXSmall, XSmall, Small, Medium, Large, XLarge, XXLarge,
                   FontSizeNone, FontSizeValue };

static const qreal sizeTable[] =
{ qreal(6.9), qreal(8.3), qreal(10.0), qreal(12.0), qreal(14.4), qreal(17.3), qreal(20.7) };

Q_STATIC_ASSERT(sizeof(sizeTable)/sizeof(sizeTable[0]) == FontSizeNone);

static FontSizeSpec fontSizeSpec(QStringView spec)
{
    switch (spec.at(0).unicode()) {
    case 'x':
        if (spec == QLatin1String("xx-small"))
            return XXSmall;
        if (spec == QLatin1String("x-small"))
            return XSmall;
        if (spec == QLatin1String("x-large"))
            return XLarge;
        if (spec == QLatin1String("xx-large"))
            return XXLarge;
        break;
    case 's':
        if (spec == QLatin1String("small"))
            return Small;
        break;
    case 'm':
        if (spec == QLatin1String("medium"))
            return Medium;
        break;
    case 'l':
        if (spec == QLatin1String("large"))
            return Large;
        break;
    case 'n':
        if (spec == QLatin1String("none"))
            return FontSizeNone;
        break;
    default:
        break;
    }
    return FontSizeValue;
}

static void parseFont(QSvgNode *node,
                      const QSvgAttributes &attributes,
                      QSvgHandler *)
{
    if (attributes.fontFamily.isEmpty() && attributes.fontSize.isEmpty() && attributes.fontStyle.isEmpty() &&
        attributes.fontWeight.isEmpty() && attributes.fontVariant.isEmpty() && attributes.textAnchor.isEmpty())
        return;

    QSvgFontStyle *fontStyle = nullptr;
    if (!attributes.fontFamily.isEmpty()) {
        QSvgDocument *doc = node->document();
        if (doc) {
            QSvgFont *svgFont = doc->svgFont(attributes.fontFamily.toString());
            if (svgFont)
                fontStyle = new QSvgFontStyle(svgFont, doc);
        }
    }
    if (!fontStyle)
        fontStyle = new QSvgFontStyle;
    if (!attributes.fontFamily.isEmpty() && attributes.fontFamily != QT_INHERIT) {
        QStringView family = attributes.fontFamily.trimmed();
        if (!family.isEmpty() && (family.at(0) == QLatin1Char('\'') || family.at(0) == QLatin1Char('\"')))
            family = family.mid(1, family.size() - 2);
        fontStyle->setFamily(family.toString());
    }

    if (!attributes.fontSize.isEmpty() && attributes.fontSize != QT_INHERIT) {
        // TODO: Support relative sizes 'larger' and 'smaller'.
        const FontSizeSpec spec = fontSizeSpec(attributes.fontSize);
        switch (spec) {
        case FontSizeNone:
            break;
        case FontSizeValue: {
            QSvgUtils::LengthType type;
            qreal fs = QSvgUtils::parseLength(attributes.fontSize, &type);
            fs = QSvgUtils::convertToPixels(fs, true, type);
            fontStyle->setSize(qMin(fs, qreal(0xffff)));
        }
            break;
        default:
            fontStyle->setSize(sizeTable[spec]);
            break;
        }
    }

    if (!attributes.fontStyle.isEmpty() && attributes.fontStyle != QT_INHERIT) {
        if (attributes.fontStyle == QLatin1String("normal")) {
            fontStyle->setStyle(QFont::StyleNormal);
        } else if (attributes.fontStyle == QLatin1String("italic")) {
            fontStyle->setStyle(QFont::StyleItalic);
        } else if (attributes.fontStyle == QLatin1String("oblique")) {
            fontStyle->setStyle(QFont::StyleOblique);
        }
    }

    if (!attributes.fontWeight.isEmpty() && attributes.fontWeight != QT_INHERIT) {
        bool ok = false;
        const int weightNum = attributes.fontWeight.toInt(&ok);
        if (ok) {
            fontStyle->setWeight(weightNum);
        } else {
            if (attributes.fontWeight == QLatin1String("normal")) {
                fontStyle->setWeight(QFont::Normal);
            } else if (attributes.fontWeight == QLatin1String("bold")) {
                fontStyle->setWeight(QFont::Bold);
            } else if (attributes.fontWeight == QLatin1String("bolder")) {
                fontStyle->setWeight(QSvgFontStyle::BOLDER);
            } else if (attributes.fontWeight == QLatin1String("lighter")) {
                fontStyle->setWeight(QSvgFontStyle::LIGHTER);
            }
        }
    }

    if (!attributes.fontVariant.isEmpty() && attributes.fontVariant != QT_INHERIT) {
        if (attributes.fontVariant == QLatin1String("normal"))
            fontStyle->setVariant(QFont::MixedCase);
        else if (attributes.fontVariant == QLatin1String("small-caps"))
            fontStyle->setVariant(QFont::SmallCaps);
    }

    if (!attributes.textAnchor.isEmpty() && attributes.textAnchor != QT_INHERIT) {
        if (attributes.textAnchor == QLatin1String("start"))
            fontStyle->setTextAnchor(Qt::AlignLeft);
        if (attributes.textAnchor == QLatin1String("middle"))
           fontStyle->setTextAnchor(Qt::AlignHCenter);
        else if (attributes.textAnchor == QLatin1String("end"))
           fontStyle->setTextAnchor(Qt::AlignRight);
    }

    node->appendStyleProperty(fontStyle, attributes.id);
}

static void parseTransform(QSvgNode *node,
                           const QSvgAttributes &attributes,
                           QSvgHandler *)
{
    if (attributes.transform.isEmpty())
        return;
    QTransform matrix = parseTransformationMatrix(attributes.transform.trimmed());

    if (!matrix.isIdentity()) {
        node->appendStyleProperty(new QSvgTransformStyle(QTransform(matrix)), attributes.id);
    }

}

static void parseVisibility(QSvgNode *node,
                            const QSvgAttributes &attributes,
                            QSvgHandler *)
{
    QSvgNode *parent = node->parent();

    if (parent && (attributes.visibility.isEmpty() || attributes.visibility == QT_INHERIT))
        node->setVisible(parent->isVisible());
    else if (attributes.visibility == QLatin1String("hidden") || attributes.visibility == QLatin1String("collapse")) {
        node->setVisible(false);
    } else
        node->setVisible(true);
}

static bool parseStyle(QSvgNode *node,
                       const QXmlStreamAttributes &attributes,
                       QSvgHandler *handler);

static int parseClockValue(QStringView str, bool *ok)
{
    int res = 0;
    int ms = 1000;
    str = str.trimmed();
    if (str.endsWith(QLatin1String("ms"))) {
        str.chop(2);
        ms = 1;
    } else if (str.endsWith(QLatin1String("s"))) {
        str.chop(1);
    }
    double val = ms * QSvgUtils::toDouble(str, ok);
    if (ok) {
        if (val > std::numeric_limits<int>::min() && val < std::numeric_limits<int>::max())
            res = static_cast<int>(val);
        else
            *ok = false;
    }
    return res;
}

#ifndef QT_NO_CSSPARSER

static void parseCssAnimations(QSvgNode *node,
                               const QXmlStreamAttributes &attributes,
                               QSvgHandler *handler)
{
    QSvgCssProperties cssAnimProps(attributes);
    QList<QSvgAnimationProperty> parsedProperties = cssAnimProps.animations();

    for (auto &property : parsedProperties) {
        QSvgCssAnimation *anim = handler->cssHandler().createAnimation(property.name);
        if (!anim)
            continue;

        anim->setRunningTime(property.delay, property.duration);
        anim->setIterationCount(property.iteration);
        QSvgCssEasingPtr easing = handler->cssHandler().createEasing(property.easingFunction, property.easingValues);
        anim->setEasing(std::move(easing));

        handler->setAnimPeriod(property.delay, property.delay + property.duration);
        handler->document()->animator()->appendAnimation(node, anim);
        handler->document()->setAnimated(true);
    }
}

static void parseOffsetPath(QSvgNode *node,
                            const QXmlStreamAttributes &attributes)
{
    QSvgCssProperties cssProperties(attributes);
    QSvgOffsetProperty offset = cssProperties.offset();

    if (!offset.path)
        return;

    QSvgOffsetStyle *offsetStyle = new QSvgOffsetStyle();
    offsetStyle->setPath(offset.path.value());
    offsetStyle->setRotateAngle(offset.angle);
    offsetStyle->setRotateType(offset.rotateType);
    offsetStyle->setDistance(offset.distance);
    node->appendStyleProperty(offsetStyle, QString());
}

#endif // QT_NO_CSSPARSER

QtSvg::Options QSvgHandler::options() const
{
    return m_options;
}

QtSvg::AnimatorType QSvgHandler::animatorType() const
{
    return m_animatorType;
}

bool QSvgHandler::trustedSourceMode() const
{
    return m_options.testFlag(QtSvg::AssumeTrustedSource);
}

static inline QStringList stringToList(const QString &str)
{
    QStringList lst = str.split(QLatin1Char(','), Qt::SkipEmptyParts);
    return lst;
}

static bool parseCoreNode(QSvgNode *node,
                          const QXmlStreamAttributes &attributes)
{
    QStringList features;
    QStringList extensions;
    QStringList languages;
    QStringList formats;
    QStringList fonts;
    QStringView xmlClassStr;

    for (const QXmlStreamAttribute &attribute : attributes) {
        QStringView name = attribute.qualifiedName();
        if (name.isEmpty())
            continue;
        QStringView value = attribute.value();
        switch (name.at(0).unicode()) {
        case 'c':
            if (name == QLatin1String("class"))
                xmlClassStr = value;
            break;
        case 'r':
            if (name == QLatin1String("requiredFeatures"))
                features = stringToList(value.toString());
            else if (name == QLatin1String("requiredExtensions"))
                extensions = stringToList(value.toString());
            else if (name == QLatin1String("requiredFormats"))
                formats = stringToList(value.toString());
            else if (name == QLatin1String("requiredFonts"))
                fonts = stringToList(value.toString());
            break;
        case 's':
            if (name == QLatin1String("systemLanguage"))
                languages = stringToList(value.toString());
            break;
        default:
            break;
        }
    }

    node->setRequiredFeatures(features);
    node->setRequiredExtensions(extensions);
    node->setRequiredLanguages(languages);
    node->setRequiredFormats(formats);
    node->setRequiredFonts(fonts);
    node->setNodeId(someId(attributes));
    node->setXmlClass(xmlClassStr.toString());

    return true;
}

static void parseOpacity(QSvgNode *node,
                         const QSvgAttributes &attributes,
                         QSvgHandler *)
{
    if (attributes.opacity.isEmpty())
        return;

    const QStringView value = attributes.opacity.trimmed();

    bool ok = false;
    qreal op = value.toDouble(&ok);

    if (ok) {
        QSvgOpacityStyle *opacity = new QSvgOpacityStyle(qBound(qreal(0.0), op, qreal(1.0)));
        node->appendStyleProperty(opacity, attributes.id);
    }
}

static QPainter::CompositionMode svgToQtCompositionMode(const QStringView op)
{
#define NOOP qDebug()<<"Operation: "<<op<<" is not implemented"
    if (op == QLatin1String("clear")) {
        return QPainter::CompositionMode_Clear;
    } else if (op == QLatin1String("src")) {
        return QPainter::CompositionMode_Source;
    } else if (op == QLatin1String("dst")) {
        return QPainter::CompositionMode_Destination;
    } else if (op == QLatin1String("src-over")) {
        return QPainter::CompositionMode_SourceOver;
    } else if (op == QLatin1String("dst-over")) {
        return QPainter::CompositionMode_DestinationOver;
    } else if (op == QLatin1String("src-in")) {
        return QPainter::CompositionMode_SourceIn;
    } else if (op == QLatin1String("dst-in")) {
        return QPainter::CompositionMode_DestinationIn;
    } else if (op == QLatin1String("src-out")) {
        return QPainter::CompositionMode_SourceOut;
    } else if (op == QLatin1String("dst-out")) {
        return QPainter::CompositionMode_DestinationOut;
    } else if (op == QLatin1String("src-atop")) {
        return QPainter::CompositionMode_SourceAtop;
    } else if (op == QLatin1String("dst-atop")) {
        return QPainter::CompositionMode_DestinationAtop;
    } else if (op == QLatin1String("xor")) {
        return QPainter::CompositionMode_Xor;
    } else if (op == QLatin1String("plus")) {
        return QPainter::CompositionMode_Plus;
    } else if (op == QLatin1String("multiply")) {
        return QPainter::CompositionMode_Multiply;
    } else if (op == QLatin1String("screen")) {
        return QPainter::CompositionMode_Screen;
    } else if (op == QLatin1String("overlay")) {
        return QPainter::CompositionMode_Overlay;
    } else if (op == QLatin1String("darken")) {
        return QPainter::CompositionMode_Darken;
    } else if (op == QLatin1String("lighten")) {
        return QPainter::CompositionMode_Lighten;
    } else if (op == QLatin1String("color-dodge")) {
        return QPainter::CompositionMode_ColorDodge;
    } else if (op == QLatin1String("color-burn")) {
        return QPainter::CompositionMode_ColorBurn;
    } else if (op == QLatin1String("hard-light")) {
        return QPainter::CompositionMode_HardLight;
    } else if (op == QLatin1String("soft-light")) {
        return QPainter::CompositionMode_SoftLight;
    } else if (op == QLatin1String("difference")) {
        return QPainter::CompositionMode_Difference;
    } else if (op == QLatin1String("exclusion")) {
        return QPainter::CompositionMode_Exclusion;
    } else {
        NOOP;
    }

    return QPainter::CompositionMode_SourceOver;
}

static void parseCompOp(QSvgNode *node,
                        const QSvgAttributes &attributes,
                        QSvgHandler *)
{
    if (attributes.compOp.isEmpty())
        return;
    QStringView value = attributes.compOp.trimmed();

    if (!value.isEmpty()) {
        QSvgCompOpStyle *compop = new QSvgCompOpStyle(svgToQtCompositionMode(value));
        node->appendStyleProperty(compop, attributes.id);
    }
}

static QSvgNode::DisplayMode displayStringToEnum(const QStringView str)
{
    if (str == QLatin1String("inline")) {
        return QSvgNode::InlineMode;
    } else if (str == QLatin1String("block")) {
        return QSvgNode::BlockMode;
    } else if (str == QLatin1String("list-item")) {
        return QSvgNode::ListItemMode;
    } else if (str == QLatin1String("run-in")) {
        return QSvgNode::RunInMode;
    } else if (str == QLatin1String("compact")) {
        return QSvgNode::CompactMode;
    } else if (str == QLatin1String("marker")) {
        return QSvgNode::MarkerMode;
    } else if (str == QLatin1String("table")) {
        return QSvgNode::TableMode;
    } else if (str == QLatin1String("inline-table")) {
        return QSvgNode::InlineTableMode;
    } else if (str == QLatin1String("table-row-group")) {
        return QSvgNode::TableRowGroupMode;
    } else if (str == QLatin1String("table-header-group")) {
        return QSvgNode::TableHeaderGroupMode;
    } else if (str == QLatin1String("table-footer-group")) {
        return QSvgNode::TableFooterGroupMode;
    } else if (str == QLatin1String("table-row")) {
        return QSvgNode::TableRowMode;
    } else if (str == QLatin1String("table-column-group")) {
        return QSvgNode::TableColumnGroupMode;
    } else if (str == QLatin1String("table-column")) {
        return QSvgNode::TableColumnMode;
    } else if (str == QLatin1String("table-cell")) {
        return QSvgNode::TableCellMode;
    } else if (str == QLatin1String("table-caption")) {
        return QSvgNode::TableCaptionMode;
    } else if (str == QLatin1String("none")) {
        return QSvgNode::NoneMode;
    } else if (str == QT_INHERIT) {
        return QSvgNode::InheritMode;
    }
    return QSvgNode::BlockMode;
}

static void parseOthers(QSvgNode *node,
                        const QSvgAttributes &attributes,
                        QSvgHandler *)
{
    if (attributes.display.isEmpty())
        return;
    QStringView displayStr = attributes.display.trimmed();

    if (!displayStr.isEmpty()) {
        node->setDisplayMode(displayStringToEnum(displayStr));
    }
}

static std::optional<QStringView> getAttributeId(const QStringView &attribute)
{
    if (attribute.isEmpty())
        return std::nullopt;

    constexpr QLatin1String urlKeyword("url");
    QStringView attrStr = attribute.trimmed();
    if (attrStr.startsWith(urlKeyword))
        attrStr.slice(urlKeyword.size());
    QStringView id = idFromUrl(attrStr);
    if (id.startsWith(QLatin1Char('#')))
        id.slice(1);
    return id;
}

static void parseExtendedAttributes(QSvgNode *node,
                                    const QSvgAttributes &attributes,
                                    QSvgHandler *handler)
{
    if (handler->options().testFlag(QtSvg::Tiny12FeaturesOnly))
        return;

    if (auto id = getAttributeId(attributes.mask))
        node->setMaskId(id->toString());
    if (auto id = getAttributeId(attributes.markerStart))
        node->setMarkerStartId(id->toString());
    if (auto id = getAttributeId(attributes.markerMid))
        node->setMarkerMidId(id->toString());
    if (auto id = getAttributeId(attributes.markerEnd))
        node->setMarkerEndId(id->toString());
    if (auto id = getAttributeId(attributes.filter))
        node->setFilterId(id->toString());
}

static void parseRenderingHints(QSvgNode *node,
                                const QSvgAttributes &attributes,
                                QSvgHandler *)
{
    if (attributes.imageRendering.isEmpty())
        return;

    QStringView ir = attributes.imageRendering.trimmed();
    QSvgQualityStyle *p = new QSvgQualityStyle(0);
    if (ir == QLatin1String("auto"))
        p->setImageRendering(QSvgQualityStyle::ImageRenderingAuto);
    else if (ir == QLatin1String("optimizeSpeed"))
        p->setImageRendering(QSvgQualityStyle::ImageRenderingOptimizeSpeed);
    else if (ir == QLatin1String("optimizeQuality"))
        p->setImageRendering(QSvgQualityStyle::ImageRenderingOptimizeQuality);
    node->appendStyleProperty(p, attributes.id);
}

static bool parseStyle(QSvgNode *node,
                       const QXmlStreamAttributes &attributes,
                       QSvgHandler *handler)
{
    // Get style in the following order :
    // 1) values from svg attributes
    // 2) CSS style
    // 3) values defined in the svg "style" property
    QSvgAttributes svgAttributes(attributes, handler);

#ifndef QT_NO_CSSPARSER
    QXmlStreamAttributes cssAttributes;
    handler->cssHandler().styleLookup(node, cssAttributes);

    QStringView style = attributes.value(QLatin1String("style"));
    if (!style.isEmpty())
        handler->cssHandler().parseCSStoXMLAttrs(style.toString(), cssAttributes);
    svgAttributes.setAttributes(cssAttributes, handler);

    parseOffsetPath(node, cssAttributes);
    if (!handler->options().testFlag(QtSvg::DisableCSSAnimations))
        parseCssAnimations(node, cssAttributes, handler);
#endif

    parseColor(node, svgAttributes, handler);
    parseBrush(node, svgAttributes, handler);
    parsePen(node, svgAttributes, handler);
    parseFont(node, svgAttributes, handler);
    parseTransform(node, svgAttributes, handler);
    parseVisibility(node, svgAttributes, handler);
    parseOpacity(node, svgAttributes, handler);
    parseCompOp(node, svgAttributes, handler);
    parseRenderingHints(node, svgAttributes, handler);
    parseOthers(node, svgAttributes, handler);
    parseExtendedAttributes(node, svgAttributes, handler);

    return true;
}

static bool parseAnchorNode(QSvgNode *parent,
                            const QXmlStreamAttributes &attributes,
                            QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseBaseAnimate(QSvgNode *parent,
                             const QXmlStreamAttributes &attributes,
                             QSvgAnimateNode *anim,
                             QSvgHandler *handler)
{
    const QStringView beginStr   = attributes.value(QLatin1String("begin"));
    const QStringView durStr     = attributes.value(QLatin1String("dur"));
    const QStringView endStr     = attributes.value(QLatin1String("end"));
    const QStringView repeatStr  = attributes.value(QLatin1String("repeatCount"));
    const QStringView fillStr    = attributes.value(QLatin1String("fill"));
    const QStringView addtv      = attributes.value(QLatin1String("additive"));
    QStringView linkId           = attributes.value(QLatin1String("xlink:href"));

    if (linkId.isEmpty())
        linkId = attributes.value(QLatin1String("href"));

    linkId = linkId.mid(1);

    bool ok = true;
    int begin = parseClockValue(beginStr, &ok);
    if (!ok)
        return false;
    int dur = parseClockValue(durStr, &ok);
    if (!ok)
        return false;
    int end = parseClockValue(endStr, &ok);
    if (!ok)
        return false;
    qreal repeatCount = (repeatStr == QLatin1String("indefinite")) ? -1 :
                            qMax(1.0, QSvgUtils::toDouble(repeatStr));

    QSvgAnimateNode::Fill fill = (fillStr == QLatin1String("freeze")) ? QSvgAnimateNode::Freeze :
                                     QSvgAnimateNode::Remove;

    QSvgAnimateNode::Additive additive = (addtv == QLatin1String("sum")) ? QSvgAnimateNode::Sum :
                                             QSvgAnimateNode::Replace;

    anim->setRunningTime(begin, dur, end, 0);
    anim->setRepeatCount(repeatCount);
    anim->setFill(fill);
    anim->setAdditiveType(additive);
    anim->setLinkId(linkId.toString());

    parent->document()->setAnimated(true);

    handler->setAnimPeriod(begin, begin + dur);
    return true;
}

static void generateKeyFrames(QList<qreal> &keyFrames, uint count)
{
    if (count < 2)
        return;

    qreal spacing = 1.0f / (count - 1);
    for (uint i = 0; i < count; i++) {
        keyFrames.append(i * spacing);
    }
}

static QSvgNode *createAnimateColorNode(QSvgNode *parent,
                                        const QXmlStreamAttributes &attributes,
                                        QSvgHandler *handler)
{
    const QStringView fromStr   = attributes.value(QLatin1String("from"));
    const QStringView toStr     = attributes.value(QLatin1String("to"));
    const QStringView valuesStr = attributes.value(QLatin1String("values"));
    const QString targetStr     = attributes.value(QLatin1String("attributeName")).toString();

    if (targetStr != QLatin1String("fill") && targetStr != QLatin1String("stroke"))
        return nullptr;

    QList<QColor> colors;
    if (valuesStr.isEmpty()) {
        QColor startColor, endColor;
        resolveColor(fromStr, startColor, handler);
        resolveColor(toStr, endColor, handler);
        colors.reserve(2);
        colors.append(startColor);
        colors.append(endColor);
    } else {
        for (auto part : qTokenize(valuesStr, u';')) {
            QColor color;
            resolveColor(part, color, handler);
            colors.append(color);
        }
    }

    QSvgAnimatedPropertyColor *prop = static_cast<QSvgAnimatedPropertyColor *>
                                            (QSvgAbstractAnimatedProperty::createAnimatedProperty(targetStr));
    if (!prop)
        return nullptr;

    prop->setColors(colors);

    QList<qreal> keyFrames;
    generateKeyFrames(keyFrames, colors.size());
    prop->setKeyFrames(keyFrames);

    QSvgAnimateColor *anim = new QSvgAnimateColor(parent);
    anim->appendProperty(prop);

    if (!parseBaseAnimate(parent, attributes, anim, handler)) {
        delete anim;
        return nullptr;
    }

    return anim;
}

static QSvgNode *createAimateMotionNode(QSvgNode *parent,
                                        const QXmlStreamAttributes &attributes,
                                        QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return nullptr;
}

static void parseNumberTriplet(QList<qreal> &values, QStringView *s)
{
    QList<qreal> list = parseNumbersList(s);
    values << list;
    for (int i = 3 - list.size(); i > 0; --i)
        values.append(0.0);
}

static void parseNumberTriplet(QList<qreal> &values, QStringView s)
{
    parseNumberTriplet(values, &s);
}

static QSvgNode *createAnimateTransformNode(QSvgNode *parent,
                                            const QXmlStreamAttributes &attributes,
                                            QSvgHandler *handler)
{
    const QStringView typeStr = attributes.value(QLatin1String("type"));
    const QStringView values  = attributes.value(QLatin1String("values"));
    const QStringView fromStr = attributes.value(QLatin1String("from"));
    const QStringView toStr   = attributes.value(QLatin1String("to"));
    const QStringView byStr   = attributes.value(QLatin1String("by"));

    QList<qreal> vals;
    if (values.isEmpty()) {
        if (fromStr.isEmpty()) {
            if (!byStr.isEmpty()) {
                vals.append(0.0);
                vals.append(0.0);
                vals.append(0.0);
                parseNumberTriplet(vals, byStr);
            } else {
                // To-animation not defined.
                return nullptr;
            }
        } else {
            if (!toStr.isEmpty()) {
                // From-to-animation.
                parseNumberTriplet(vals, fromStr);
                parseNumberTriplet(vals, toStr);
            } else if (!byStr.isEmpty()) {
                // From-by-animation.
                parseNumberTriplet(vals, fromStr);
                parseNumberTriplet(vals, byStr);
                for (int i = vals.size() - 3; i < vals.size(); ++i)
                    vals[i] += vals[i - 3];
            } else {
                return nullptr;
            }
        }
    } else {
        QStringView s = values;
        while (!s.isEmpty()) {
            parseNumberTriplet(vals, &s);
            if (!s.isEmpty())
                s.slice(1);
        }
    }
    if (vals.size() % 3 != 0)
        return nullptr;


    QList<QSvgAnimatedPropertyTransform::TransformComponent> components;
    for (int i = 0; i < vals.size(); i+=3) {
        QSvgAnimatedPropertyTransform::TransformComponent component;
        if (typeStr == QLatin1String("translate")) {
            component.type = QSvgAnimatedPropertyTransform::TransformComponent::Translate;
            component.values.append(vals.at(i));
            component.values.append(vals.at(i + 1));
        } else if (typeStr == QLatin1String("scale")) {
            component.type = QSvgAnimatedPropertyTransform::TransformComponent::Scale;
            component.values.append(vals.at(i));
            component.values.append(vals.at(i + 1));
        } else if (typeStr == QLatin1String("rotate")) {
            component.type = QSvgAnimatedPropertyTransform::TransformComponent::Rotate;
            component.values.append(vals.at(i));
            component.values.append(vals.at(i + 1));
            component.values.append(vals.at(i + 2));
        } else if (typeStr == QLatin1String("skewX")) {
            component.type = QSvgAnimatedPropertyTransform::TransformComponent::Skew;
            component.values.append(vals.at(i));
            component.values.append(0);
        } else if (typeStr == QLatin1String("skewY")) {
            component.type = QSvgAnimatedPropertyTransform::TransformComponent::Skew;
            component.values.append(0);
            component.values.append(vals.at(i));
        } else {
            return nullptr;
        }
        components.append(component);
    }

    QSvgAnimatedPropertyTransform *prop = static_cast<QSvgAnimatedPropertyTransform *>
        (QSvgAbstractAnimatedProperty::createAnimatedProperty(QLatin1String("transform")));
    if (!prop)
        return nullptr;

    prop->appendComponents(components);
    // <animateTransform> always has one component per key frame
    prop->setTransformCount(1);
    QList<qreal> keyFrames;
    generateKeyFrames(keyFrames, vals.size() / 3);
    prop->setKeyFrames(keyFrames);

    QSvgAnimateTransform *anim = new QSvgAnimateTransform(parent);
    anim->appendProperty(prop);

    if (!parseBaseAnimate(parent, attributes, anim, handler)) {
        delete anim;
        return nullptr;
    }

    return anim;
}

static QSvgNode *createAnimateNode(QSvgNode *parent,
                                   const QXmlStreamAttributes &attributes,
                                   QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return nullptr;
}

static bool parseAudioNode(QSvgNode *parent,
                           const QXmlStreamAttributes &attributes,
                           QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createCircleNode(QSvgNode *parent,
                                  const QXmlStreamAttributes &attributes,
                                  QSvgHandler *)
{
    const QStringView cx = attributes.value(QLatin1String("cx"));
    const QStringView cy = attributes.value(QLatin1String("cy"));
    const QStringView r = attributes.value(QLatin1String("r"));
    qreal ncx = QSvgUtils::toDouble(cx);
    qreal ncy = QSvgUtils::toDouble(cy);
    qreal nr  = QSvgUtils::toDouble(r);
    if (nr < 0.0)
        return nullptr;

    QRectF rect(ncx-nr, ncy-nr, nr*2, nr*2);
    QSvgNode *circle = new QSvgCircle(parent, rect);
    return circle;
}

static QSvgNode *createDefsNode(QSvgNode *parent,
                                const QXmlStreamAttributes &attributes,
                                QSvgHandler *)
{
    Q_UNUSED(attributes);
    QSvgDefs *defs = new QSvgDefs(parent);
    return defs;
}

static bool parseDiscardNode(QSvgNode *parent,
                             const QXmlStreamAttributes &attributes,
                             QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createEllipseNode(QSvgNode *parent,
                                   const QXmlStreamAttributes &attributes,
                                   QSvgHandler *)
{
    const QStringView cx = attributes.value(QLatin1String("cx"));
    const QStringView cy = attributes.value(QLatin1String("cy"));
    const QStringView rx = attributes.value(QLatin1String("rx"));
    const QStringView ry = attributes.value(QLatin1String("ry"));
    qreal ncx = QSvgUtils::toDouble(cx);
    qreal ncy = QSvgUtils::toDouble(cy);
    qreal nrx = QSvgUtils::toDouble(rx);
    qreal nry = QSvgUtils::toDouble(ry);

    QRectF rect(ncx-nrx, ncy-nry, nrx*2, nry*2);
    QSvgNode *ellipse = new QSvgEllipse(parent, rect);
    return ellipse;
}

static QSvgStyleProperty *createFontNode(QSvgNode *parent,
                                         const QXmlStreamAttributes &attributes,
                                         QSvgHandler *)
{
    const QStringView hax = attributes.value(QLatin1String("horiz-adv-x"));
    QString myId     = someId(attributes);

    qreal horizAdvX = QSvgUtils::toDouble(hax);

    while (parent && parent->type() != QSvgNode::Doc) {
        parent = parent->parent();
    }

    if (parent && !myId.isEmpty()) {
        QSvgDocument *doc = static_cast<QSvgDocument*>(parent);
        QSvgFont *font = doc->svgFont(myId);
        if (!font) {
            font = new QSvgFont(horizAdvX);
            font->setFamilyName(myId);
            doc->addSvgFont(font);
        }
        return new QSvgFontStyle(font, doc);
    }
    return nullptr;
}

static bool parseFontFaceNode(QSvgStyleProperty *parent,
                              const QXmlStreamAttributes &attributes,
                              QSvgHandler *)
{
    if (parent->type() != QSvgStyleProperty::FONT) {
        return false;
    }

    QSvgFontStyle *style = static_cast<QSvgFontStyle*>(parent);
    QSvgFont *font = style->svgFont();
    const QStringView name = attributes.value(QLatin1String("font-family"));
    const QStringView unitsPerEmStr = attributes.value(QLatin1String("units-per-em"));

    qreal unitsPerEm = QSvgUtils::toDouble(unitsPerEmStr);
    if (!unitsPerEm)
        unitsPerEm = QSvgFont::DEFAULT_UNITS_PER_EM;

    if (!name.isEmpty())
        font->setFamilyName(name.toString());
    font->setUnitsPerEm(unitsPerEm);

    if (!font->familyName().isEmpty())
        if (!style->doc()->svgFont(font->familyName()))
            style->doc()->addSvgFont(font);

    return true;
}

static bool parseFontFaceNameNode(QSvgStyleProperty *parent,
                                  const QXmlStreamAttributes &attributes,
                                  QSvgHandler *)
{
    if (parent->type() != QSvgStyleProperty::FONT) {
        return false;
    }

    QSvgFontStyle *style = static_cast<QSvgFontStyle*>(parent);
    QSvgFont *font = style->svgFont();
    const QStringView name = attributes.value(QLatin1String("name"));

    if (!name.isEmpty())
        font->setFamilyName(name.toString());

    if (!font->familyName().isEmpty())
        if (!style->doc()->svgFont(font->familyName()))
            style->doc()->addSvgFont(font);

    return true;
}

static bool parseFontFaceSrcNode(QSvgStyleProperty *parent,
                                 const QXmlStreamAttributes &attributes,
                                 QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseFontFaceUriNode(QSvgStyleProperty *parent,
                                 const QXmlStreamAttributes &attributes,
                                 QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseForeignObjectNode(QSvgNode *parent,
                                   const QXmlStreamAttributes &attributes,
                                   QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createGNode(QSvgNode *parent,
                             const QXmlStreamAttributes &attributes,
                             QSvgHandler *)
{
    Q_UNUSED(attributes);
    QSvgG *node = new QSvgG(parent);
    return node;
}

static bool parseGlyphNode(QSvgStyleProperty *parent,
                           const QXmlStreamAttributes &attributes,
                           QSvgHandler *)
{
    if (parent->type() != QSvgStyleProperty::FONT) {
        return false;
    }

    QSvgFontStyle *style = static_cast<QSvgFontStyle*>(parent);
    QSvgFont *font = style->svgFont();
    return createSvgGlyph(font, attributes, false);
}

static bool parseHandlerNode(QSvgNode *parent,
                             const QXmlStreamAttributes &attributes,
                             QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseHkernNode(QSvgNode *parent,
                           const QXmlStreamAttributes &attributes,
                           QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgNode *createImageNode(QSvgNode *parent,
                                 const QXmlStreamAttributes &attributes,
                                 QSvgHandler *handler)
{
    const QStringView x      = attributes.value(QLatin1String("x"));
    const QStringView y      = attributes.value(QLatin1String("y"));
    const QStringView width  = attributes.value(QLatin1String("width"));
    const QStringView height = attributes.value(QLatin1String("height"));
    QString filename         = attributes.value(QLatin1String("xlink:href")).toString();
    if (filename.isEmpty() && !handler->options().testFlag(QtSvg::Tiny12FeaturesOnly))
        filename = attributes.value(QLatin1String("href")).toString();
    qreal nx = QSvgUtils::toDouble(x);
    qreal ny = QSvgUtils::toDouble(y);
    QSvgUtils::LengthType type;
    qreal nwidth = QSvgUtils::parseLength(width.toString(), &type);
    nwidth = QSvgUtils::convertToPixels(nwidth, true, type);

    qreal nheight = QSvgUtils::parseLength(height.toString(), &type);
    nheight = QSvgUtils::convertToPixels(nheight, false, type);

    filename = filename.trimmed();
    if (filename.isEmpty()) {
        qCWarning(lcSvgHandler) << "QSvgHandler: Image filename is empty";
        return 0;
    }
    if (nwidth <= 0 || nheight <= 0) {
        qCWarning(lcSvgHandler) << "QSvgHandler: Width or height for" << filename << "image was not greater than 0";
        return 0;
    }

    QImage image;
    enum {
        NotLoaded,
        LoadedFromData,
        LoadedFromFile
    } filenameType = NotLoaded;

    if (filename.startsWith(QLatin1String("data"))) {
        QString mimeType;
        QByteArray data;
        if (qDecodeDataUrl(QUrl{filename}, mimeType, data)) {
            image = QImage::fromData(data);
            filenameType = LoadedFromData;
        }
    }

    if (image.isNull()) {
        const auto *file = qobject_cast<QFile *>(handler->device());
        if (file) {
            QUrl url(filename);
            if (url.isRelative()) {
                QFileInfo info(file->fileName());
                filename = info.absoluteDir().absoluteFilePath(filename);
            }
        }

        if (handler->trustedSourceMode() || !QImageReader::imageFormat(filename).startsWith("svg")) {
            image = QImage(filename);
            filenameType = LoadedFromFile;
        }
    }

    if (image.isNull()) {
        qCWarning(lcSvgHandler) << "Could not create image from" << filename;
        return 0;
    }

    if (image.format() == QImage::Format_ARGB32)
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    QSvgNode *img = new QSvgImage(parent,
                                  image,
                                  filenameType == LoadedFromFile ? filename : QString{},
                                  QRectF(nx,
                                         ny,
                                         nwidth,
                                         nheight));
    return img;
}

static QSvgNode *createLineNode(QSvgNode *parent,
                                const QXmlStreamAttributes &attributes,
                                QSvgHandler *)
{
    const QStringView x1 = attributes.value(QLatin1String("x1"));
    const QStringView y1 = attributes.value(QLatin1String("y1"));
    const QStringView x2 = attributes.value(QLatin1String("x2"));
    const QStringView y2 = attributes.value(QLatin1String("y2"));
    qreal nx1 = QSvgUtils::toDouble(x1);
    qreal ny1 = QSvgUtils::toDouble(y1);
    qreal nx2 = QSvgUtils::toDouble(x2);
    qreal ny2 = QSvgUtils::toDouble(y2);

    QLineF lineBounds(nx1, ny1, nx2, ny2);
    QSvgNode *line = new QSvgLine(parent, lineBounds);
    return line;
}


static void parseBaseGradient(QSvgNode *node,
                              const QXmlStreamAttributes &attributes,
                              QSvgGradientStyle *gradProp,
                              QSvgHandler *handler)
{
    const QStringView link             = attributes.value(QLatin1String("xlink:href"));
    const QStringView trans            = attributes.value(QLatin1String("gradientTransform"));
    const QStringView spread           = attributes.value(QLatin1String("spreadMethod"));
    const QStringView units            = attributes.value(QLatin1String("gradientUnits"));
    const QStringView colorStr         = attributes.value(QLatin1String("color"));
    const QStringView colorOpacityStr  = attributes.value(QLatin1String("color-opacity"));

    QColor color;
    if (constructColor(colorStr, colorOpacityStr, color, handler)) {
        handler->popColor();
        handler->pushColor(color);
    }

    QTransform matrix;
    QGradient *grad = gradProp->qgradient();
    if (node && !link.isEmpty()) {
        QSvgStyleProperty *prop = node->styleProperty(link);
        if (prop && prop->type() == QSvgStyleProperty::GRADIENT) {
            QSvgGradientStyle *inherited =
                static_cast<QSvgGradientStyle*>(prop);
            if (!inherited->stopLink().isEmpty()) {
                gradProp->setStopLink(inherited->stopLink(), handler->document());
            } else {
                grad->setStops(inherited->qgradient()->stops());
                gradProp->setGradientStopsSet(inherited->gradientStopsSet());
            }

            matrix = inherited->qtransform();
        } else {
            gradProp->setStopLink(link.toString(), handler->document());
        }
    }

    if (!trans.isEmpty()) {
        matrix = parseTransformationMatrix(trans);
        gradProp->setTransform(matrix);
    } else if (!matrix.isIdentity()) {
        gradProp->setTransform(matrix);
    }

    if (!spread.isEmpty()) {
        if (spread == QLatin1String("pad")) {
            grad->setSpread(QGradient::PadSpread);
        } else if (spread == QLatin1String("reflect")) {
            grad->setSpread(QGradient::ReflectSpread);
        } else if (spread == QLatin1String("repeat")) {
            grad->setSpread(QGradient::RepeatSpread);
        }
    }

    if (units.isEmpty() || units == QLatin1String("objectBoundingBox")) {
         grad->setCoordinateMode(QGradient::ObjectMode);
    }
}

static QSvgStyleProperty *createLinearGradientNode(QSvgNode *node,
                                                   const QXmlStreamAttributes &attributes,
                                                   QSvgHandler *handler)
{
    const QStringView x1 = attributes.value(QLatin1String("x1"));
    const QStringView y1 = attributes.value(QLatin1String("y1"));
    const QStringView x2 = attributes.value(QLatin1String("x2"));
    const QStringView y2 = attributes.value(QLatin1String("y2"));

    qreal nx1 = 0.0;
    qreal ny1 = 0.0;
    qreal nx2 = 1.0;
    qreal ny2 = 0.0;

    if (!x1.isEmpty())
        nx1 =  convertToNumber(x1);
    if (!y1.isEmpty())
        ny1 =  convertToNumber(y1);
    if (!x2.isEmpty())
        nx2 =  convertToNumber(x2);
    if (!y2.isEmpty())
        ny2 =  convertToNumber(y2);

    QSvgNode *itr = node;
    while (itr && itr->type() != QSvgNode::Doc) {
        itr = itr->parent();
    }

    QLinearGradient *grad = new QLinearGradient(nx1, ny1, nx2, ny2);
    grad->setInterpolationMode(QGradient::ComponentInterpolation);
    QSvgGradientStyle *prop = new QSvgGradientStyle(grad);
    parseBaseGradient(node, attributes, prop, handler);

    return prop;
}

static bool parseMetadataNode(QSvgNode *parent,
                              const QXmlStreamAttributes &attributes,
                              QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseMissingGlyphNode(QSvgStyleProperty *parent,
                                  const QXmlStreamAttributes &attributes,
                                  QSvgHandler *)
{
    if (parent->type() != QSvgStyleProperty::FONT) {
        return false;
    }

    QSvgFontStyle *style = static_cast<QSvgFontStyle*>(parent);
    QSvgFont *font = style->svgFont();
    return createSvgGlyph(font, attributes, true);
}

static bool parseMpathNode(QSvgNode *parent,
                           const QXmlStreamAttributes &attributes,
                           QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseMaskNode(QSvgNode *parent,
                          const QXmlStreamAttributes &attributes,
                          QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseMarkerNode(QSvgNode *,
                          const QXmlStreamAttributes &,
                          QSvgHandler *)
{
    return true;
}

static QSvgNode *createMaskNode(QSvgNode *parent,
                          const QXmlStreamAttributes &attributes,
                          QSvgHandler *handler)
{
    const QStringView x      = attributes.value(QLatin1String("x"));
    const QStringView y      = attributes.value(QLatin1String("y"));
    const QStringView width  = attributes.value(QLatin1String("width"));
    const QStringView height = attributes.value(QLatin1String("height"));
    const QStringView mU     = attributes.value(QLatin1String("maskUnits"));
    const QStringView mCU    = attributes.value(QLatin1String("maskContentUnits"));

    QtSvg::UnitTypes nmU = mU.contains(QLatin1String("userSpaceOnUse")) ?
                QtSvg::UnitTypes::userSpaceOnUse : QtSvg::UnitTypes::objectBoundingBox;

    QtSvg::UnitTypes nmCU = mCU.contains(QLatin1String("objectBoundingBox")) ?
                QtSvg::UnitTypes::objectBoundingBox : QtSvg::UnitTypes::userSpaceOnUse;

    bool ok;
    QSvgUtils::LengthType type;

    QtSvg::UnitTypes nmUx = nmU;
    QtSvg::UnitTypes nmUy = nmU;
    QtSvg::UnitTypes nmUw = nmU;
    QtSvg::UnitTypes nmUh = nmU;
    qreal nx = QSvgUtils::parseLength(x, &type, &ok);
    nx = QSvgUtils::convertToPixels(nx, true, type);
    if (x.isEmpty() || !ok) {
        nx = -0.1;
        nmUx = QtSvg::UnitTypes::objectBoundingBox;
    } else if (type == QSvgUtils::LengthType::LT_PERCENT && nmU == QtSvg::UnitTypes::userSpaceOnUse) {
        nx = nx / 100. * handler->document()->viewBox().width();
    } else if (type == QSvgUtils::LengthType::LT_PERCENT) {
        nx = nx / 100.;
    }

    qreal ny = QSvgUtils::parseLength(y, &type, &ok);
    ny = QSvgUtils::convertToPixels(ny, true, type);
    if (y.isEmpty() || !ok) {
        ny = -0.1;
        nmUy = QtSvg::UnitTypes::objectBoundingBox;
    } else if (type == QSvgUtils::LengthType::LT_PERCENT && nmU == QtSvg::UnitTypes::userSpaceOnUse) {
        ny = ny / 100. * handler->document()->viewBox().height();
    } else if (type == QSvgUtils::LengthType::LT_PERCENT) {
        ny = ny / 100.;
    }

    qreal nwidth = QSvgUtils::parseLength(width, &type, &ok);
    nwidth = QSvgUtils::convertToPixels(nwidth, true, type);
    if (width.isEmpty() || !ok) {
        nwidth = 1.2;
        nmUw = QtSvg::UnitTypes::objectBoundingBox;
    } else if (type == QSvgUtils::LengthType::LT_PERCENT && nmU == QtSvg::UnitTypes::userSpaceOnUse) {
        nwidth = nwidth / 100. * handler->document()->viewBox().width();
    } else if (type == QSvgUtils::LengthType::LT_PERCENT) {
        nwidth = nwidth / 100.;
    }

    qreal nheight = QSvgUtils::parseLength(height, &type, &ok);
    nheight = QSvgUtils::convertToPixels(nheight, true, type);
    if (height.isEmpty() || !ok) {
        nheight = 1.2;
        nmUh = QtSvg::UnitTypes::objectBoundingBox;
    } else if (type == QSvgUtils::LengthType::LT_PERCENT && nmU == QtSvg::UnitTypes::userSpaceOnUse) {
        nheight = nheight / 100. * handler->document()->viewBox().height();
    } else if (type == QSvgUtils::LengthType::LT_PERCENT) {
        nheight = nheight / 100.;
    }

    QRectF bounds(nx, ny, nwidth, nheight);
    if (bounds.isEmpty())
        return nullptr;

    QSvgNode *mask = new QSvgMask(parent, QSvgRectF(bounds, nmUx, nmUy, nmUw, nmUh), nmCU);

    return mask;
}

static void parseFilterBounds(const QXmlStreamAttributes &attributes, QSvgRectF *rect)
{
    const QStringView xStr        = attributes.value(QLatin1String("x"));
    const QStringView yStr        = attributes.value(QLatin1String("y"));
    const QStringView widthStr    = attributes.value(QLatin1String("width"));
    const QStringView heightStr   = attributes.value(QLatin1String("height"));

    qreal x = 0;
    if (!xStr.isEmpty()) {
        QSvgUtils::LengthType type;
        x = QSvgUtils::parseLength(xStr, &type);
        if (type != QSvgUtils::LengthType::LT_PT) {
            x = QSvgUtils::convertToPixels(x, true, type);
            rect->setUnitX(QtSvg::UnitTypes::userSpaceOnUse);
        }
        if (type == QSvgUtils::LengthType::LT_PERCENT) {
            x /= 100.;
            rect->setUnitX(QtSvg::UnitTypes::objectBoundingBox);
        }
        rect->setX(x);
    }
    qreal y = 0;
    if (!yStr.isEmpty()) {
        QSvgUtils::LengthType type;
        y = QSvgUtils::parseLength(yStr, &type);
        if (type != QSvgUtils::LengthType::LT_PT) {
            y = QSvgUtils::convertToPixels(y, false, type);
            rect->setUnitY(QtSvg::UnitTypes::userSpaceOnUse);
        }
        if (type == QSvgUtils::LengthType::LT_PERCENT) {
            y /= 100.;
            rect->setUnitX(QtSvg::UnitTypes::objectBoundingBox);
        }
        rect->setY(y);
    }
    qreal width = 0;
    if (!widthStr.isEmpty()) {
        QSvgUtils::LengthType type;
        width = QSvgUtils::parseLength(widthStr, &type);
        if (type != QSvgUtils::LengthType::LT_PT) {
            width = QSvgUtils::convertToPixels(width, true, type);
            rect->setUnitW(QtSvg::UnitTypes::userSpaceOnUse);
        }
        if (type == QSvgUtils::LengthType::LT_PERCENT) {
            width /= 100.;
            rect->setUnitX(QtSvg::UnitTypes::objectBoundingBox);
        }
        rect->setWidth(width);
    }
    qreal height = 0;
    if (!heightStr.isEmpty()) {
        QSvgUtils::LengthType type;
        height = QSvgUtils::parseLength(heightStr, &type);
        if (type != QSvgUtils::LengthType::LT_PT) {
            height = QSvgUtils::convertToPixels(height, false, type);
            rect->setUnitH(QtSvg::UnitTypes::userSpaceOnUse);
        }
        if (type == QSvgUtils::LengthType::LT_PERCENT) {
            height /= 100.;
            rect->setUnitX(QtSvg::UnitTypes::objectBoundingBox);
        }
        rect->setHeight(height);
    }
}

static QSvgNode *createFilterNode(QSvgNode *parent,
                          const QXmlStreamAttributes &attributes,
                          QSvgHandler *handler)
{
    const QStringView fU = attributes.value(QLatin1String("filterUnits"));
    const QStringView pU = attributes.value(QLatin1String("primitiveUnits"));

    const QtSvg::UnitTypes filterUnits = fU.contains(QLatin1String("userSpaceOnUse")) ?
                QtSvg::UnitTypes::userSpaceOnUse : QtSvg::UnitTypes::objectBoundingBox;

    const QtSvg::UnitTypes primitiveUnits = pU.contains(QLatin1String("objectBoundingBox")) ?
                QtSvg::UnitTypes::objectBoundingBox : QtSvg::UnitTypes::userSpaceOnUse;

    // https://www.w3.org/TR/SVG11/filters.html#FilterEffectsRegion
    // If ‘x’ or ‘y’ is not specified, the effect is as if a value of -10% were specified.
    // If ‘width’ or ‘height’ is not specified, the effect is as if a value of 120% were specified.
    QSvgRectF rect;
    if (filterUnits == QtSvg::UnitTypes::userSpaceOnUse) {
        qreal width = handler->document()->viewBox().width();
        qreal height = handler->document()->viewBox().height();
        rect = QSvgRectF(QRectF(-0.1 * width, -0.1 * height, 1.2 * width, 1.2 * height),
                         QtSvg::UnitTypes::userSpaceOnUse, QtSvg::UnitTypes::userSpaceOnUse,
                         QtSvg::UnitTypes::userSpaceOnUse, QtSvg::UnitTypes::userSpaceOnUse);
    } else {
        rect = QSvgRectF(QRectF(-0.1, -0.1, 1.2, 1.2),
                         QtSvg::UnitTypes::objectBoundingBox, QtSvg::UnitTypes::objectBoundingBox,
                         QtSvg::UnitTypes::objectBoundingBox, QtSvg::UnitTypes::objectBoundingBox);
    }

    parseFilterBounds(attributes, &rect);

    QSvgNode *filter = new QSvgFilterContainer(parent, rect, filterUnits, primitiveUnits);
    return filter;
}

static void parseFilterAttributes(const QXmlStreamAttributes &attributes, QString *inString,
                                  QString *outString, QSvgRectF *rect)
{
    *inString = attributes.value(QLatin1String("in")).toString();
    *outString = attributes.value(QLatin1String("result")).toString();

    // https://www.w3.org/TR/SVG11/filters.html#FilterPrimitiveSubRegion
    // the default subregion is 0%,0%,100%,100%, where as a special-case the percentages are
    // relative to the dimensions of the filter region, thus making the the default filter primitive
    // subregion equal to the filter region.
    *rect = QSvgRectF(QRectF(0, 0, 1.0, 1.0),
                      QtSvg::UnitTypes::unknown, QtSvg::UnitTypes::unknown,
                      QtSvg::UnitTypes::unknown, QtSvg::UnitTypes::unknown);
    // if we recognize unit == unknown we use the filter as a reference instead of the item, see
    // QSvgFeFilterPrimitive::localSubRegion

    parseFilterBounds(attributes, rect);
}

static QSvgNode *createFeColorMatrixNode(QSvgNode *parent,
                                        const QXmlStreamAttributes &attributes,
                                        QSvgHandler *)
{
    const QStringView typeString   = attributes.value(QLatin1String("type"));
    const QStringView valuesString = attributes.value(QLatin1String("values"));

    QString inputString;
    QString outputString;
    QSvgRectF rect;

    QSvgFeColorMatrix::ColorShiftType type;
    QSvgFeColorMatrix::Matrix values;
    values.fill(0);

    parseFilterAttributes(attributes, &inputString, &outputString, &rect);

    if (typeString.startsWith(QLatin1String("saturate")))
        type = QSvgFeColorMatrix::ColorShiftType::Saturate;
    else if (typeString.startsWith(QLatin1String("hueRotate")))
        type = QSvgFeColorMatrix::ColorShiftType::HueRotate;
    else if (typeString.startsWith(QLatin1String("luminanceToAlpha")))
        type = QSvgFeColorMatrix::ColorShiftType::LuminanceToAlpha;
    else
        type = QSvgFeColorMatrix::ColorShiftType::Matrix;

    if (!valuesString.isEmpty()) {
        const auto valueStringList = splitWithDelimiter(valuesString);
        for (int i = 0, j = 0; i < qMin(20, valueStringList.size()); i++) {
            bool ok;
            qreal v = QSvgUtils::toDouble(valueStringList.at(i), &ok);
            if (ok) {
                values.data()[j] = v;
                j++;
            }
        }
    } else {
        values.setToIdentity();
    }

    QSvgNode *filter = new QSvgFeColorMatrix(parent, inputString, outputString, rect,
                                             type, values);
    return filter;
}

static QSvgNode *createFeGaussianBlurNode(QSvgNode *parent,
                                          const QXmlStreamAttributes &attributes,
                                          QSvgHandler *)
{
    const QStringView edgeModeString     = attributes.value(QLatin1String("edgeMode"));
    const QStringView stdDeviationString = attributes.value(QLatin1String("stdDeviation"));

    QString inputString;
    QString outputString;
    QSvgRectF rect;

    QSvgFeGaussianBlur::EdgeMode edgemode = QSvgFeGaussianBlur::EdgeMode::Duplicate;

    parseFilterAttributes(attributes, &inputString, &outputString, &rect);
    qreal stdDeviationX = 0;
    qreal stdDeviationY = 0;
    if (stdDeviationString.contains(QStringLiteral(" "))){
        stdDeviationX = qMax(0., QSvgUtils::toDouble(stdDeviationString.split(u" ").first()));
        stdDeviationY = qMax(0., QSvgUtils::toDouble(stdDeviationString.split(u" ").last()));
    } else {
        stdDeviationY = stdDeviationX = qMax(0., QSvgUtils::toDouble(stdDeviationString));
    }

    if (edgeModeString.startsWith(QLatin1String("wrap")))
        edgemode = QSvgFeGaussianBlur::EdgeMode::Wrap;
    else if (edgeModeString.startsWith(QLatin1String("none")))
        edgemode = QSvgFeGaussianBlur::EdgeMode::None;

    QSvgNode *filter = new QSvgFeGaussianBlur(parent, inputString, outputString, rect,
                                              stdDeviationX, stdDeviationY, edgemode);
    return filter;
}

static QSvgNode *createFeOffsetNode(QSvgNode *parent,
                                    const QXmlStreamAttributes &attributes,
                                    QSvgHandler *)
{
    QStringView dxString = attributes.value(QLatin1String("dx"));
    QStringView dyString = attributes.value(QLatin1String("dy"));

    QString inputString;
    QString outputString;
    QSvgRectF rect;

    parseFilterAttributes(attributes, &inputString, &outputString, &rect);

    qreal dx = 0;
    if (!dxString.isEmpty()) {
        QSvgUtils::LengthType type;
        dx = QSvgUtils::parseLength(dxString, &type);
        if (type != QSvgUtils::LengthType::LT_PT)
            dx = QSvgUtils::convertToPixels(dx, true, type);
    }

    qreal dy = 0;
    if (!dyString.isEmpty()) {
        QSvgUtils::LengthType type;
        dy = QSvgUtils::parseLength(dyString, &type);
        if (type != QSvgUtils::LengthType::LT_PT)
            dy = QSvgUtils::convertToPixels(dy, true, type);
    }

    QSvgNode *filter = new QSvgFeOffset(parent, inputString, outputString, rect,
                                        dx, dy);
    return filter;
}

static QSvgNode *createFeCompositeNode(QSvgNode *parent,
                                  const QXmlStreamAttributes &attributes,
                                  QSvgHandler *)
{
    const QStringView in2String      = attributes.value(QLatin1String("in2"));
    const QStringView operatorString = attributes.value(QLatin1String("operator"));
    const QStringView k1String       = attributes.value(QLatin1String("k1"));
    const QStringView k2String       = attributes.value(QLatin1String("k2"));
    const QStringView k3String       = attributes.value(QLatin1String("k3"));
    const QStringView k4String       = attributes.value(QLatin1String("k4"));

    QString inputString;
    QString outputString;
    QSvgRectF rect;

    parseFilterAttributes(attributes, &inputString, &outputString, &rect);

    QSvgFeComposite::Operator op = QSvgFeComposite::Operator::Over;
    if (operatorString.startsWith(QLatin1String("in")))
        op = QSvgFeComposite::Operator::In;
    else if (operatorString.startsWith(QLatin1String("out")))
        op = QSvgFeComposite::Operator::Out;
    else if (operatorString.startsWith(QLatin1String("atop")))
        op = QSvgFeComposite::Operator::Atop;
    else if (operatorString.startsWith(QLatin1String("xor")))
        op = QSvgFeComposite::Operator::Xor;
    else if (operatorString.startsWith(QLatin1String("lighter")))
        op = QSvgFeComposite::Operator::Lighter;
    else if (operatorString.startsWith(QLatin1String("arithmetic")))
        op = QSvgFeComposite::Operator::Arithmetic;

    QVector4D k(0, 0, 0, 0);

    if (op == QSvgFeComposite::Operator::Arithmetic) {
        bool ok;
        qreal v = QSvgUtils::toDouble(k1String, &ok);
        if (ok)
            k.setX(v);
        v = QSvgUtils::toDouble(k2String, &ok);
        if (ok)
            k.setY(v);
        v = QSvgUtils::toDouble(k3String, &ok);
        if (ok)
            k.setZ(v);
        v = QSvgUtils::toDouble(k4String, &ok);
        if (ok)
            k.setW(v);
    }

    QSvgNode *filter = new QSvgFeComposite(parent, inputString, outputString, rect,
                                           in2String.toString(), op, k);
    return filter;
}


static QSvgNode *createFeMergeNode(QSvgNode *parent,
                                   const QXmlStreamAttributes &attributes,
                                   QSvgHandler *)
{
    QString inputString;
    QString outputString;
    QSvgRectF rect;

    parseFilterAttributes(attributes, &inputString, &outputString, &rect);

    QSvgNode *filter = new QSvgFeMerge(parent, inputString, outputString, rect);
    return filter;
}

static QSvgNode *createFeFloodNode(QSvgNode *parent,
                                   const QXmlStreamAttributes &attributes,
                                   QSvgHandler *handler)
{
    QStringView colorStr          = attributes.value(QLatin1String("flood-color"));
    const QStringView opacityStr  = attributes.value(QLatin1String("flood-opacity"));

    QColor color;
    if (!constructColor(colorStr, opacityStr, color, handler)) {
        color = QColor(Qt::black);
        if (opacityStr.isEmpty())
            color.setAlphaF(1.0);
        else
            setAlpha(opacityStr, &color);
    }

    QString inputString;
    QString outputString;
    QSvgRectF rect;

    parseFilterAttributes(attributes, &inputString, &outputString, &rect);

    QSvgNode *filter = new QSvgFeFlood(parent, inputString, outputString, rect, color);
    return filter;
}

static QSvgNode *createFeMergeNodeNode(QSvgNode *parent,
                                       const QXmlStreamAttributes &attributes,
                                       QSvgHandler *)
{
    QString inputString;
    QString outputString;
    QSvgRectF rect;

    parseFilterAttributes(attributes, &inputString, &outputString, &rect);

    QSvgNode *filter = new QSvgFeMergeNode(parent, inputString, outputString, rect);
    return filter;
}

static QSvgNode *createFeBlendNode(QSvgNode *parent,
                                   const QXmlStreamAttributes &attributes,
                                   QSvgHandler *)
{
    const QStringView in2String = attributes.value(QLatin1String("in2"));
    const QStringView modeString = attributes.value(QLatin1String("mode"));

    QString inputString;
    QString outputString;
    QSvgRectF rect;

    parseFilterAttributes(attributes, &inputString, &outputString, &rect);

    QSvgFeBlend::Mode mode = QSvgFeBlend::Mode::Normal;
    if (modeString.startsWith(QLatin1StringView("multiply")))
        mode = QSvgFeBlend::Mode::Multiply;
    else if (modeString.startsWith(QLatin1StringView("screen")))
        mode = QSvgFeBlend::Mode::Screen;
    else if (modeString.startsWith(QLatin1StringView("darken")))
        mode = QSvgFeBlend::Mode::Darken;
    else if (modeString.startsWith(QLatin1StringView("lighten")))
        mode = QSvgFeBlend::Mode::Lighten;

    QSvgNode *filter = new QSvgFeBlend(parent, inputString, outputString, rect,
                                       in2String.toString(), mode);
    return filter;
}

static QSvgNode *createFeUnsupportedNode(QSvgNode *parent,
                                         const QXmlStreamAttributes &attributes,
                                         QSvgHandler *)
{
    QString inputString;
    QString outputString;
    QSvgRectF rect;

    parseFilterAttributes(attributes, &inputString, &outputString, &rect);

    QSvgNode *filter = new QSvgFeUnsupported(parent, inputString, outputString, rect);
    return filter;
}

static std::optional<QRectF> parseViewBox(QStringView str)
{
    QList<QStringView> viewBoxValues;

    if (!str.isEmpty())
        viewBoxValues = splitWithDelimiter(str);
    if (viewBoxValues.size() == 4) {
        QSvgUtils::LengthType type;
        qreal x = QSvgUtils::parseLength(viewBoxValues.at(0).trimmed(), &type);
        qreal y = QSvgUtils::parseLength(viewBoxValues.at(1).trimmed(), &type);
        qreal w = QSvgUtils::parseLength(viewBoxValues.at(2).trimmed(), &type);
        qreal h = QSvgUtils::parseLength(viewBoxValues.at(3).trimmed(), &type);
        return QRectF(x, y, w, h);
    }
    return std::nullopt;
}

static bool parseSymbolLikeAttributes(const QXmlStreamAttributes &attributes, QSvgHandler *handler,
                                      QRectF *rect, QRectF *viewBox, QPointF *refPoint,
                                      QSvgSymbolLike::PreserveAspectRatios *aspect,
                                      QSvgSymbolLike::Overflow *overflow,
                                      bool marker = false)
{
    const QStringView xStr        = attributes.value(QLatin1String("x"));
    const QStringView yStr        = attributes.value(QLatin1String("y"));
    const QStringView refXStr     = attributes.value(QLatin1String("refX"));
    const QStringView refYStr     = attributes.value(QLatin1String("refY"));
    const QStringView widthStr    = attributes.value(marker ? QLatin1String("markerWidth")
                                                            : QLatin1String("width"));
    const QStringView heightStr   = attributes.value(marker ? QLatin1String("markerHeight")
                                                            : QLatin1String("height"));
    const QStringView pAspectRStr = attributes.value(QLatin1String("preserveAspectRatio"));
    const QStringView overflowStr = attributes.value(QLatin1String("overflow"));
    const QStringView viewBoxStr  = attributes.value(QLatin1String("viewBox"));


    qreal x = 0;
    if (!xStr.isEmpty()) {
        QSvgUtils::LengthType type;
        x = QSvgUtils::parseLength(xStr, &type);
        if (type != QSvgUtils::LengthType::LT_PT)
            x = QSvgUtils::convertToPixels(x, true, type);
    }
    qreal y = 0;
    if (!yStr.isEmpty()) {
        QSvgUtils::LengthType type;
        y = QSvgUtils::parseLength(yStr, &type);
        if (type != QSvgUtils::LengthType::LT_PT)
            y = QSvgUtils::convertToPixels(y, false, type);
    }
    qreal width = 0;
    if (!widthStr.isEmpty()) {
        QSvgUtils::LengthType type;
        width = QSvgUtils::parseLength(widthStr, &type);
        if (type != QSvgUtils::LengthType::LT_PT)
            width = QSvgUtils::convertToPixels(width, true, type);
    }
    qreal height = 0;
    if (!heightStr.isEmpty()) {
        QSvgUtils::LengthType type;
        height = QSvgUtils::parseLength(heightStr, &type);
        if (type != QSvgUtils::LengthType::LT_PT)
            height = QSvgUtils::convertToPixels(height, false, type);
    }

    *rect = QRectF(x, y, width, height);

    x = 0;
    if (!refXStr.isEmpty()) {
        QSvgUtils::LengthType type;
        x = QSvgUtils::parseLength(refXStr, &type);
        if (type != QSvgUtils::LengthType::LT_PT)
            x = QSvgUtils::convertToPixels(x, true, type);
    }
    y = 0;
    if (!refYStr.isEmpty()) {
        QSvgUtils::LengthType type;
        y = QSvgUtils::parseLength(refYStr, &type);
        if (type != QSvgUtils::LengthType::LT_PT)
            y = QSvgUtils::convertToPixels(y, false, type);
    }
    *refPoint = QPointF(x,y);

    auto viewBoxResult = parseViewBox(viewBoxStr);
    if (viewBoxResult)
        *viewBox = *viewBoxResult;
    else if (width > 0 && height > 0)
        *viewBox = QRectF(0, 0, width, height);
    else
        *viewBox = handler->document()->viewBox();

    if (viewBox->isNull())
        return false;

    auto pAspectRStrs = pAspectRStr.split(u" ");
    QSvgSymbolLike::PreserveAspectRatio aspectX = QSvgSymbolLike::PreserveAspectRatio::xMid;
    QSvgSymbolLike::PreserveAspectRatio aspectY = QSvgSymbolLike::PreserveAspectRatio::yMid;
    QSvgSymbolLike::PreserveAspectRatio aspectMS = QSvgSymbolLike::PreserveAspectRatio::meet;

    for (auto &pAStr : std::as_const(pAspectRStrs)) {
        if (pAStr.startsWith(QLatin1String("none"))) {
            aspectX = QSvgSymbolLike::PreserveAspectRatio::None;
            aspectY = QSvgSymbolLike::PreserveAspectRatio::None;
        }else {
            if (pAStr.startsWith(QLatin1String("xMin")))
                aspectX = QSvgSymbolLike::PreserveAspectRatio::xMin;
            else if (pAStr.startsWith(QLatin1String("xMax")))
                aspectX = QSvgSymbolLike::PreserveAspectRatio::xMax;
            if (pAStr.endsWith(QLatin1String("YMin")))
                aspectY = QSvgSymbolLike::PreserveAspectRatio::yMin;
            else if (pAStr.endsWith(QLatin1String("YMax")))
                aspectY = QSvgSymbolLike::PreserveAspectRatio::yMax;
        }

        if (pAStr.endsWith(QLatin1String("slice")))
            aspectMS = QSvgSymbolLike::PreserveAspectRatio::slice;
    }
    *aspect = aspectX | aspectY | aspectMS;

    // overflow is not limited to the symbol element but it is often found with the symbol element.
    // the symbol element makes little sense without the overflow attribute so it is added here.
    // if we decide to remove this from QSvgSymbol, the default value should be set to visible.

    // The default value is visible but chrome uses default value hidden.
    *overflow = QSvgSymbolLike::Overflow::Hidden;

    if (overflowStr.endsWith(QLatin1String("auto")))
        *overflow = QSvgSymbolLike::Overflow::Auto;
    else if (overflowStr.endsWith(QLatin1String("visible")))
        *overflow = QSvgSymbolLike::Overflow::Visible;
    else if (overflowStr.endsWith(QLatin1String("hidden")))
        *overflow = QSvgSymbolLike::Overflow::Hidden;
    else if (overflowStr.endsWith(QLatin1String("scroll")))
        *overflow = QSvgSymbolLike::Overflow::Scroll;

    return true;
}

static QSvgNode *createSymbolNode(QSvgNode *parent,
                          const QXmlStreamAttributes &attributes,
                          QSvgHandler *handler)
{
    QRectF rect, viewBox;
    QPointF refP;
    QSvgSymbolLike::PreserveAspectRatios aspect;
    QSvgSymbolLike::Overflow overflow;

    if (!parseSymbolLikeAttributes(attributes, handler, &rect, &viewBox, &refP, &aspect, &overflow))
        return nullptr;

    refP = QPointF(0, 0); //refX, refY is ignored in Symbol in Firefox and Chrome.
    QSvgNode *symbol = new QSvgSymbol(parent, rect, viewBox, refP, aspect, overflow);
    return symbol;
}

static QSvgNode *createMarkerNode(QSvgNode *parent,
                                  const QXmlStreamAttributes &attributes,
                                  QSvgHandler *handler)
{
    QRectF rect, viewBox;
    QPointF refP;
    QSvgSymbolLike::PreserveAspectRatios aspect;
    QSvgSymbolLike::Overflow overflow;

    const QStringView orientStr      = attributes.value(QLatin1String("orient"));
    const QStringView markerUnitsStr = attributes.value(QLatin1String("markerUnits"));

    qreal orientationAngle = 0;
    QSvgMarker::Orientation orientation;
    if (orientStr.startsWith(QLatin1String("auto-start-reverse")))
        orientation = QSvgMarker::Orientation::AutoStartReverse;
    else if (orientStr.startsWith(QLatin1String("auto")))
        orientation = QSvgMarker::Orientation::Auto;
    else {
        orientation = QSvgMarker::Orientation::Value;
        bool ok;
        qreal a;
        if (orientStr.endsWith(QLatin1String("turn")))
            a = 360. * QSvgUtils::toDouble(orientStr.mid(0, orientStr.length()-4), &ok);
        else if (orientStr.endsWith(QLatin1String("grad")))
            a = QSvgUtils::toDouble(orientStr.mid(0, orientStr.length()-4), &ok);
        else if (orientStr.endsWith(QLatin1String("rad")))
            a = 180. / M_PI * QSvgUtils::toDouble(orientStr.mid(0, orientStr.length()-3), &ok);
        else
            a = QSvgUtils::toDouble(orientStr, &ok);
        if (ok)
            orientationAngle = a;
    }

    QSvgMarker::MarkerUnits markerUnits = QSvgMarker::MarkerUnits::StrokeWidth;
    if (markerUnitsStr.startsWith(QLatin1String("userSpaceOnUse")))
        markerUnits = QSvgMarker::MarkerUnits::UserSpaceOnUse;

    if (!parseSymbolLikeAttributes(attributes, handler, &rect, &viewBox, &refP, &aspect, &overflow, true))
        return nullptr;

    QSvgNode *marker = new QSvgMarker(parent, rect, viewBox, refP, aspect, overflow,
                                      orientation, orientationAngle, markerUnits);
    return marker;
}

static QSvgNode *createPathNode(QSvgNode *parent,
                                const QXmlStreamAttributes &attributes,
                                QSvgHandler *handler)
{
    QStringView data = attributes.value(QLatin1String("d"));

    std::optional<QPainterPath> qpath = QSvgUtils::parsePathDataFast(data,
                                                !handler->trustedSourceMode());
    if (!qpath) {
        qCWarning(lcSvgHandler, "Invalid path data; path truncated.");
        return nullptr;
    }

    qpath.value().setFillRule(Qt::WindingFill);
    QSvgNode *path = new QSvgPath(parent, qpath.value());
    return path;
}

static QSvgNode *createPolyNode(QSvgNode *parent,
                                const QXmlStreamAttributes &attributes,
                                bool createLine)
{
    QStringView pointsStr = attributes.value(QLatin1String("points"));
    const QList<qreal> points = parseNumbersList(&pointsStr);
    if (points.size() < 4)
        return nullptr;
    QPolygonF poly(points.size()/2);
    for (int i = 0; i < poly.size(); ++i)
        poly[i] = QPointF(points.at(2 * i), points.at(2 * i + 1));
    if (createLine)
        return new QSvgPolyline(parent, poly);
    else
        return new QSvgPolygon(parent, poly);
}

static QSvgNode *createPolygonNode(QSvgNode *parent,
                                   const QXmlStreamAttributes &attributes,
                                   QSvgHandler *)
{
    return createPolyNode(parent, attributes, false);
}

static QSvgNode *createPolylineNode(QSvgNode *parent,
                                    const QXmlStreamAttributes &attributes,
                                    QSvgHandler *)
{
    return createPolyNode(parent, attributes, true);
}

static bool parsePrefetchNode(QSvgNode *parent,
                              const QXmlStreamAttributes &attributes,
                              QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgStyleProperty *createRadialGradientNode(QSvgNode *node,
                                                   const QXmlStreamAttributes &attributes,
                                                   QSvgHandler *handler)
{
    const QStringView cx = attributes.value(QLatin1String("cx"));
    const QStringView cy = attributes.value(QLatin1String("cy"));
    const QStringView r  = attributes.value(QLatin1String("r"));
    const QStringView fx = attributes.value(QLatin1String("fx"));
    const QStringView fy = attributes.value(QLatin1String("fy"));

    qreal ncx = 0.5;
    qreal ncy = 0.5;
    if (!cx.isEmpty())
        ncx = convertToNumber(cx);
    if (!cy.isEmpty())
        ncy = convertToNumber(cy);

    qreal nr = 0.5;
    if (!r.isEmpty())
        nr = convertToNumber(r);
    if (nr <= 0.0)
        return nullptr;

    qreal nfx = ncx;
    if (!fx.isEmpty())
        nfx = convertToNumber(fx);
    qreal nfy = ncy;
    if (!fy.isEmpty())
        nfy = convertToNumber(fy);

    QRadialGradient *grad = new QRadialGradient(ncx, ncy, nr, nfx, nfy, 0);
    grad->setInterpolationMode(QGradient::ComponentInterpolation);

    QSvgGradientStyle *prop = new QSvgGradientStyle(grad);
    parseBaseGradient(node, attributes, prop, handler);

    return prop;
}

static QSvgNode *createRectNode(QSvgNode *parent,
                                const QXmlStreamAttributes &attributes,
                                QSvgHandler *)
{
    const QStringView x      = attributes.value(QLatin1String("x"));
    const QStringView y      = attributes.value(QLatin1String("y"));
    const QStringView width  = attributes.value(QLatin1String("width"));
    const QStringView height = attributes.value(QLatin1String("height"));
    const QStringView rx      = attributes.value(QLatin1String("rx"));
    const QStringView ry      = attributes.value(QLatin1String("ry"));

    bool ok = true;
    QSvgUtils::LengthType type;
    qreal nwidth = QSvgUtils::parseLength(width, &type, &ok);
    if (!ok)
        return nullptr;
    nwidth = QSvgUtils::convertToPixels(nwidth, true, type);
    qreal nheight = QSvgUtils::parseLength(height, &type, &ok);
    if (!ok)
        return nullptr;
    nheight = QSvgUtils::convertToPixels(nheight, true, type);
    qreal nrx = QSvgUtils::toDouble(rx);
    qreal nry = QSvgUtils::toDouble(ry);

    QRectF bounds(QSvgUtils::toDouble(x), QSvgUtils::toDouble(y), nwidth, nheight);
    if (bounds.isEmpty())
        return nullptr;

    if (!rx.isEmpty() && ry.isEmpty())
        nry = nrx;
    else if (!ry.isEmpty() && rx.isEmpty())
        nrx = nry;

    //9.2 The 'rect'  element clearly specifies it
    // but the case might in fact be handled because
    // we draw rounded rectangles differently
    if (nrx > bounds.width()/2)
        nrx = bounds.width()/2;
    if (nry > bounds.height()/2)
        nry = bounds.height()/2;

    //we draw rounded rect from 0...99
    //svg from 0...bounds.width()/2 so we're adjusting the
    //coordinates
    nrx *= (100/(bounds.width()/2));
    nry *= (100/(bounds.height()/2));

    QSvgNode *rect = new QSvgRect(parent, bounds, nrx, nry);
    return rect;
}

static bool parseScriptNode(QSvgNode *parent,
                            const QXmlStreamAttributes &attributes,
                            QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static bool parseSetNode(QSvgNode *parent,
                         const QXmlStreamAttributes &attributes,
                         QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return true;
}

static QSvgStyleProperty *createSolidColorNode(QSvgNode *parent,
                                               const QXmlStreamAttributes &attributes,
                                               QSvgHandler *handler)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    QStringView solidColorStr = attributes.value(QLatin1String("solid-color"));
    QStringView solidOpacityStr = attributes.value(QLatin1String("solid-opacity"));

    if (solidOpacityStr.isEmpty())
        solidOpacityStr = attributes.value(QLatin1String("opacity"));

    QColor color;
    if (!constructColor(solidColorStr, solidOpacityStr, color, handler))
        return 0;
    QSvgSolidColorStyle *style = new QSvgSolidColorStyle(color);
    return style;
}

static bool parseStopNode(QSvgStyleProperty *parent,
                          const QXmlStreamAttributes &attributes,
                          QSvgHandler *handler)
{
    if (parent->type() != QSvgStyleProperty::GRADIENT)
        return false;
    QString nodeIdStr     = someId(attributes);
    QString xmlClassStr   = attributes.value(QLatin1String("class")).toString();

    //### nasty hack because stop gradients are not in the rendering tree
    //    we force a dummy node with the same id and class into a rendering
    //    tree to figure out whether the selector has a style for it
    //    QSvgStyleSelector should be coded in a way that could avoid it
    QSvgDummyNode dummy;
    dummy.setNodeId(nodeIdStr);
    dummy.setXmlClass(xmlClassStr);

    QSvgAttributes attrs(attributes, handler);

#ifndef QT_NO_CSSPARSER
    QXmlStreamAttributes cssAttributes;
    handler->cssHandler().styleLookup(&dummy, cssAttributes);
    attrs.setAttributes(cssAttributes, handler);

    QXmlStreamAttributes styleCssAttributes;
    QStringView style = attributes.value(QLatin1String("style"));
    if (!style.isEmpty())
        handler->cssHandler().parseCSStoXMLAttrs(style.toString(), styleCssAttributes);
    attrs.setAttributes(styleCssAttributes, handler);
#endif

    //TODO: Handle style parsing for gradients stop like the rest of the nodes.
    parseColor(&dummy, attrs, handler);

    QSvgGradientStyle *gradientStyle =
        static_cast<QSvgGradientStyle*>(parent);
    QStringView colorStr    = attrs.stopColor;
    QColor color;

    bool ok = true;
    qreal offset = convertToNumber(attrs.offset, &ok);
    if (!ok)
        offset = 0.0;

    if (!constructColor(colorStr, attrs.stopOpacity, color, handler)) {
        color = Qt::black;
        if (!attrs.stopOpacity.isEmpty())
            setAlpha(attrs.stopOpacity, &color);
    }

    QGradient *grad = gradientStyle->qgradient();

    offset = qMin(qreal(1), qMax(qreal(0), offset)); // Clamp to range [0, 1]
    QGradientStops stops;
    if (gradientStyle->gradientStopsSet()) {
        stops = grad->stops();
        // If the stop offset equals the one previously added, add an epsilon to make it greater.
        if (offset <= stops.back().first)
            offset = stops.back().first + FLT_EPSILON;
    }

    // If offset is greater than one, it must be clamped to one.
    if (offset > 1.0) {
        if ((stops.size() == 1) || (stops.at(stops.size() - 2).first < 1.0 - FLT_EPSILON)) {
            stops.back().first = 1.0 - FLT_EPSILON;
            grad->setStops(stops);
        }
        offset = 1.0;
    }

    grad->setColorAt(offset, color);
    gradientStyle->setGradientStopsSet(true);
    return true;
}

static bool parseStyleNode(QSvgNode *parent,
                           const QXmlStreamAttributes &attributes,
                           QSvgHandler *handler)
{
    Q_UNUSED(parent);
#ifdef QT_NO_CSSPARSER
    Q_UNUSED(attributes);
    Q_UNUSED(handler);
#else
    const QStringView type = attributes.value(QLatin1String("type"));
    if (type.compare(QLatin1String("text/css"), Qt::CaseInsensitive) == 0 || type.isNull())
        handler->setInStyle(true);
#endif

    return true;
}

static QSvgNode *createSvgNode(QSvgNode *parent,
                               const QXmlStreamAttributes &attributes,
                               QSvgHandler *handler)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);

    QSvgDocument *node = new QSvgDocument(handler->options(), handler->animatorType());
    const QStringView widthStr  = attributes.value(QLatin1String("width"));
    const QStringView heightStr = attributes.value(QLatin1String("height"));
    const QStringView viewBoxStr = attributes.value(QLatin1String("viewBox"));

    QSvgUtils::LengthType type = QSvgUtils::LengthType::LT_PX; // FIXME: is the default correct?
    qreal width = 0;
    if (!widthStr.isEmpty()) {
        width = QSvgUtils::parseLength(widthStr, &type);
        if (type != QSvgUtils::LengthType::LT_PT)
            width = QSvgUtils::convertToPixels(width, true, type);
        node->setWidth(int(width), type == QSvgUtils::LengthType::LT_PERCENT);
    }
    qreal height = 0;
    if (!heightStr.isEmpty()) {
        height = QSvgUtils::parseLength(heightStr, &type);
        if (type != QSvgUtils::LengthType::LT_PT)
            height = QSvgUtils::convertToPixels(height, false, type);
        node->setHeight(int(height), type == QSvgUtils::LengthType::LT_PERCENT);
    }

    auto viewBoxResult = parseViewBox(viewBoxStr);
    if (viewBoxResult) {
        node->setViewBox(*viewBoxResult);
    } else if (width && height) {
        if (type == QSvgUtils::LengthType::LT_PT) {
            width = QSvgUtils::convertToPixels(width, false, type);
            height = QSvgUtils::convertToPixels(height, false, type);
        }
        node->setViewBox(QRectF(0, 0, width, height));
    }
    handler->setDefaultCoordinateSystem(QSvgUtils::LengthType::LT_PX);

    return node;
}

static QSvgNode *createSwitchNode(QSvgNode *parent,
                                  const QXmlStreamAttributes &attributes,
                                  QSvgHandler *)
{
    Q_UNUSED(attributes);
    QSvgSwitch *node = new QSvgSwitch(parent);
    return node;
}

static QSvgNode *createPatternNode(QSvgNode *parent,
                                   const QXmlStreamAttributes &attributes,
                                   QSvgHandler *handler)
{
    const QStringView x      = attributes.value(QLatin1String("x"));
    const QStringView y      = attributes.value(QLatin1String("y"));
    const QStringView width  = attributes.value(QLatin1String("width"));
    const QStringView height = attributes.value(QLatin1String("height"));
    const QStringView patternUnits     = attributes.value(QLatin1String("patternUnits"));
    const QStringView patternContentUnits    = attributes.value(QLatin1String("patternContentUnits"));
    const QStringView patternTransform = attributes.value(QLatin1String("patternTransform"));

    QtSvg::UnitTypes nPatternUnits = patternUnits.contains(QLatin1String("userSpaceOnUse")) ?
                                        QtSvg::UnitTypes::userSpaceOnUse : QtSvg::UnitTypes::objectBoundingBox;

    QtSvg::UnitTypes nPatternContentUnits = patternContentUnits.contains(QLatin1String("objectBoundingBox")) ?
                                               QtSvg::UnitTypes::objectBoundingBox : QtSvg::UnitTypes::userSpaceOnUse;

    const QStringView viewBoxStr = attributes.value(QLatin1String("viewBox"));

    bool ok = false;
    QSvgUtils::LengthType type;

    qreal nx = QSvgUtils::parseLength(x, &type, &ok);
    nx = QSvgUtils::convertToPixels(nx, true, type);
    if (!ok)
        nx = 0.0;
    else if (type == QSvgUtils::LengthType::LT_PERCENT && nPatternUnits == QtSvg::UnitTypes::userSpaceOnUse)
        nx = (nx / 100.) * handler->document()->viewBox().width();
    else if (type == QSvgUtils::LengthType::LT_PERCENT)
        nx = nx / 100.;

    qreal ny = QSvgUtils::parseLength(y, &type, &ok);
    ny = QSvgUtils::convertToPixels(ny, true, type);
    if (!ok)
        ny = 0.0;
    else if (type == QSvgUtils::LengthType::LT_PERCENT && nPatternUnits == QtSvg::UnitTypes::userSpaceOnUse)
        ny = (ny / 100.) * handler->document()->viewBox().height();
    else if (type == QSvgUtils::LengthType::LT_PERCENT)
        ny = ny / 100.;

    qreal nwidth = QSvgUtils::parseLength(width, &type, &ok);
    nwidth = QSvgUtils::convertToPixels(nwidth, true, type);
    if (!ok)
        nwidth = 0.0;
    else if (type == QSvgUtils::LengthType::LT_PERCENT && nPatternUnits == QtSvg::UnitTypes::userSpaceOnUse)
        nwidth = (nwidth / 100.) * handler->document()->viewBox().width();
    else if (type == QSvgUtils::LengthType::LT_PERCENT)
        nwidth = nwidth / 100.;

    qreal nheight = QSvgUtils::parseLength(height, &type, &ok);
    nheight = QSvgUtils::convertToPixels(nheight, true, type);
    if (!ok)
        nheight = 0.0;
    else if (type == QSvgUtils::LengthType::LT_PERCENT && nPatternUnits == QtSvg::UnitTypes::userSpaceOnUse)
        nheight = (nheight / 100.) * handler->document()->viewBox().height();
    else if (type == QSvgUtils::LengthType::LT_PERCENT)
        nheight = nheight / 100.;

    QRectF viewBox;
    auto viewBoxResult = parseViewBox(viewBoxStr);
    if (viewBoxResult) {
        if (viewBoxResult->width() > 0 && viewBoxResult->height() > 0)
            viewBox = *viewBoxResult;
    }

    QTransform matrix;
    if (!patternTransform.isEmpty())
        matrix = parseTransformationMatrix(patternTransform);

    QRectF bounds(nx, ny, nwidth, nheight);
    if (bounds.isEmpty())
        return nullptr;

    QSvgRectF patternRectF(bounds, nPatternUnits, nPatternUnits, nPatternUnits, nPatternUnits);
    QSvgPattern *node = new QSvgPattern(parent, patternRectF, viewBox, nPatternContentUnits, matrix);

    // Create a style node for the Pattern.
    QSvgPatternStyle *prop = new QSvgPatternStyle(node);
    node->appendStyleProperty(prop, someId(attributes));

    return node;
}

static bool parseTbreakNode(QSvgNode *parent,
                            const QXmlStreamAttributes &,
                            QSvgHandler *)
{
    if (parent->type() != QSvgNode::Textarea)
        return false;
    static_cast<QSvgText*>(parent)->addLineBreak();
    return true;
}

static QSvgNode *createTextNode(QSvgNode *parent,
                                const QXmlStreamAttributes &attributes,
                                QSvgHandler *)
{
    const QStringView x = attributes.value(QLatin1String("x"));
    const QStringView y = attributes.value(QLatin1String("y"));
    //### editable and rotate not handled
    QSvgUtils::LengthType type;
    qreal nx = QSvgUtils::parseLength(x, &type);
    nx = QSvgUtils::convertToPixels(nx, true, type);
    qreal ny = QSvgUtils::parseLength(y, &type);
    ny = QSvgUtils::convertToPixels(ny, true, type);

    QSvgNode *text = new QSvgText(parent, QPointF(nx, ny));
    return text;
}

static QSvgNode *createTextAreaNode(QSvgNode *parent,
                                    const QXmlStreamAttributes &attributes,
                                    QSvgHandler *handler)
{
    QSvgText *node = static_cast<QSvgText *>(createTextNode(parent, attributes, handler));
    if (node) {
        QSvgUtils::LengthType type;
        qreal width = QSvgUtils::parseLength(attributes.value(QLatin1String("width")), &type);
        qreal height = QSvgUtils::parseLength(attributes.value(QLatin1String("height")), &type);
        node->setTextArea(QSizeF(width, height));
    }
    return node;
}

static QSvgNode *createTspanNode(QSvgNode *parent,
                                    const QXmlStreamAttributes &,
                                    QSvgHandler *)
{
    return new QSvgTspan(parent);
}

static QSvgNode *createUseNode(QSvgNode *parent,
                               const QXmlStreamAttributes &attributes,
                               QSvgHandler *)
{
    QStringView linkId     = attributes.value(QLatin1String("xlink:href"));
    const QStringView xStr = attributes.value(QLatin1String("x"));
    const QStringView yStr = attributes.value(QLatin1String("y"));
    QSvgStructureNode *group = nullptr;

    if (linkId.isEmpty())
        linkId = attributes.value(QLatin1String("href"));
    QString linkIdStr = linkId.mid(1).toString();

    switch (parent->type()) {
    case QSvgNode::Doc:
    case QSvgNode::Defs:
    case QSvgNode::Group:
    case QSvgNode::Switch:
    case QSvgNode::Mask:
        group = static_cast<QSvgStructureNode*>(parent);
        break;
    default:
        break;
    }

    if (group) {
        QPointF pt;
        if (!xStr.isNull() || !yStr.isNull()) {
            QSvgUtils::LengthType type;
            qreal nx = QSvgUtils::parseLength(xStr, &type);
            nx = QSvgUtils::convertToPixels(nx, true, type);

            qreal ny = QSvgUtils::parseLength(yStr, &type);
            ny = QSvgUtils::convertToPixels(ny, true, type);
            pt = QPointF(nx, ny);
        }

        QSvgNode *link = group->scopeNode(linkIdStr);
        if (link) {
            if (parent->isDescendantOf(link))
                qCWarning(lcSvgHandler, "link %ls is recursive!", qUtf16Printable(linkIdStr));

            return new QSvgUse(pt, parent, link);
        }

        //delay link resolving, link might have not been created yet
        return new QSvgUse(pt, parent, linkIdStr);
    }

    qCWarning(lcSvgHandler, "<use> element %ls in wrong context!", qUtf16Printable(linkIdStr));
    return 0;
}

static QSvgNode *createVideoNode(QSvgNode *parent,
                                 const QXmlStreamAttributes &attributes,
                                 QSvgHandler *)
{
    Q_UNUSED(parent); Q_UNUSED(attributes);
    return 0;
}

typedef QSvgNode *(*FactoryMethod)(QSvgNode *, const QXmlStreamAttributes &, QSvgHandler *);

static FactoryMethod findGroupFactory(const QStringView name, QtSvg::Options options)
{
    if (name.isEmpty())
        return 0;

    QStringView ref = name.mid(1);
    switch (name.at(0).unicode()) {
    case 'd':
        if (ref == QLatin1String("efs")) return createDefsNode;
        break;
    case 'f':
        if (ref == QLatin1String("ilter") && !options.testFlag(QtSvg::Tiny12FeaturesOnly)) return createFilterNode;
        break;
    case 'g':
        if (ref.isEmpty()) return createGNode;
        break;
    case 'm':
        if (ref == QLatin1String("ask") && !options.testFlag(QtSvg::Tiny12FeaturesOnly)) return createMaskNode;
        if (ref == QLatin1String("arker") && !options.testFlag(QtSvg::Tiny12FeaturesOnly)) return createMarkerNode;
        break;
    case 's':
        if (ref == QLatin1String("vg")) return createSvgNode;
        if (ref == QLatin1String("witch")) return createSwitchNode;
        if (ref == QLatin1String("ymbol") && !options.testFlag(QtSvg::Tiny12FeaturesOnly)) return createSymbolNode;
        break;
    case 'p':
        if (ref == QLatin1String("attern") && !options.testFlag(QtSvg::Tiny12FeaturesOnly)) return createPatternNode;
        break;
    default:
        break;
    }
    return 0;
}

static FactoryMethod findGraphicsFactory(const QStringView name, QtSvg::Options options)
{
    Q_UNUSED(options);
    if (name.isEmpty())
        return 0;

    QStringView ref = name.mid(1);
    switch (name.at(0).unicode()) {
    case 'c':
        if (ref == QLatin1String("ircle")) return createCircleNode;
        break;
    case 'e':
        if (ref == QLatin1String("llipse")) return createEllipseNode;
        break;
    case 'i':
        if (ref == QLatin1String("mage")) return createImageNode;
        break;
    case 'l':
        if (ref == QLatin1String("ine")) return createLineNode;
        break;
    case 'p':
        if (ref == QLatin1String("ath")) return createPathNode;
        if (ref == QLatin1String("olygon")) return createPolygonNode;
        if (ref == QLatin1String("olyline")) return createPolylineNode;
        break;
    case 'r':
        if (ref == QLatin1String("ect")) return createRectNode;
        break;
    case 't':
        if (ref == QLatin1String("ext")) return createTextNode;
        if (ref == QLatin1String("extArea")) return createTextAreaNode;
        if (ref == QLatin1String("span")) return createTspanNode;
        break;
    case 'u':
        if (ref == QLatin1String("se")) return createUseNode;
        break;
    case 'v':
        if (ref == QLatin1String("ideo")) return createVideoNode;
        break;
    default:
        break;
    }
    return 0;
}

static FactoryMethod findFilterFactory(const QStringView name, QtSvg::Options options)
{
    if (options.testFlag(QtSvg::Tiny12FeaturesOnly))
        return 0;

    if (name.isEmpty())
        return 0;

    if (!name.startsWith(QLatin1String("fe")))
        return 0;

    if (name == QLatin1String("feMerge")) return createFeMergeNode;
    if (name == QLatin1String("feColorMatrix")) return createFeColorMatrixNode;
    if (name == QLatin1String("feGaussianBlur")) return createFeGaussianBlurNode;
    if (name == QLatin1String("feOffset")) return createFeOffsetNode;
    if (name == QLatin1String("feMergeNode")) return createFeMergeNodeNode;
    if (name == QLatin1String("feComposite")) return createFeCompositeNode;
    if (name == QLatin1String("feFlood")) return createFeFloodNode;
    if (name == QLatin1String("feBlend")) return createFeBlendNode;

    static const QStringList unsupportedFilters = {
        QStringLiteral("feComponentTransfer"),
        QStringLiteral("feConvolveMatrix"),
        QStringLiteral("feDiffuseLighting"),
        QStringLiteral("feDisplacementMap"),
        QStringLiteral("feDropShadow"),
        QStringLiteral("feFuncA"),
        QStringLiteral("feFuncB"),
        QStringLiteral("feFuncG"),
        QStringLiteral("feFuncR"),
        QStringLiteral("feImage"),
        QStringLiteral("feMorphology"),
        QStringLiteral("feSpecularLighting"),
        QStringLiteral("feTile"),
        QStringLiteral("feTurbulence")
    };

    if (unsupportedFilters.contains(name))
        return createFeUnsupportedNode;

    return 0;
}

typedef QSvgNode *(*AnimationMethod)(QSvgNode *, const QXmlStreamAttributes &, QSvgHandler *);

static AnimationMethod findAnimationFactory(const QStringView name, QtSvg::Options options)
{
    if (name.isEmpty() || options.testFlag(QtSvg::DisableSMILAnimations))
        return 0;

    QStringView ref = name.mid(1);

    switch (name.at(0).unicode()) {
    case 'a':
        if (ref == QLatin1String("nimate")) return createAnimateNode;
        if (ref == QLatin1String("nimateColor")) return createAnimateColorNode;
        if (ref == QLatin1String("nimateMotion")) return createAimateMotionNode;
        if (ref == QLatin1String("nimateTransform")) return createAnimateTransformNode;
        break;
    default:
        break;
    }

    return 0;
}

typedef bool (*ParseMethod)(QSvgNode *, const QXmlStreamAttributes &, QSvgHandler *);

static ParseMethod findUtilFactory(const QStringView name, QtSvg::Options options)
{
    if (name.isEmpty())
        return 0;

    QStringView ref = name.mid(1);
    switch (name.at(0).unicode()) {
    case 'a':
        if (ref.isEmpty()) return parseAnchorNode;
        if (ref == QLatin1String("udio")) return parseAudioNode;
        break;
    case 'd':
        if (ref == QLatin1String("iscard")) return parseDiscardNode;
        break;
    case 'f':
        if (ref == QLatin1String("oreignObject")) return parseForeignObjectNode;
        break;
    case 'h':
        if (ref == QLatin1String("andler")) return parseHandlerNode;
        if (ref == QLatin1String("kern")) return parseHkernNode;
        break;
    case 'm':
        if (ref == QLatin1String("etadata")) return parseMetadataNode;
        if (ref == QLatin1String("path")) return parseMpathNode;
        if (ref == QLatin1String("ask") && !options.testFlag(QtSvg::Tiny12FeaturesOnly)) return parseMaskNode;
        if (ref == QLatin1String("arker") && !options.testFlag(QtSvg::Tiny12FeaturesOnly)) return parseMarkerNode;
        break;
    case 'p':
        if (ref == QLatin1String("refetch")) return parsePrefetchNode;
        break;
    case 's':
        if (ref == QLatin1String("cript")) return parseScriptNode;
        if (ref == QLatin1String("et")) return parseSetNode;
        if (ref == QLatin1String("tyle")) return parseStyleNode;
        break;
    case 't':
        if (ref == QLatin1String("break")) return parseTbreakNode;
        break;
    default:
        break;
    }
    return 0;
}

typedef QSvgStyleProperty *(*StyleFactoryMethod)(QSvgNode *,
                                                 const QXmlStreamAttributes &,
                                                 QSvgHandler *);

static StyleFactoryMethod findStyleFactoryMethod(const QStringView name)
{
    if (name.isEmpty())
        return 0;

    QStringView ref = name.mid(1);
    switch (name.at(0).unicode()) {
    case 'f':
        if (ref == QLatin1String("ont")) return createFontNode;
        break;
    case 'l':
        if (ref == QLatin1String("inearGradient")) return createLinearGradientNode;
        break;
    case 'r':
        if (ref == QLatin1String("adialGradient")) return createRadialGradientNode;
        break;
    case 's':
        if (ref == QLatin1String("olidColor")) return createSolidColorNode;
        break;
    default:
        break;
    }
    return 0;
}

typedef bool (*StyleParseMethod)(QSvgStyleProperty *,
                                 const QXmlStreamAttributes &,
                                 QSvgHandler *);

static StyleParseMethod findStyleUtilFactoryMethod(const QStringView name)
{
    if (name.isEmpty())
        return 0;

    QStringView ref = name.mid(1);
    switch (name.at(0).unicode()) {
    case 'f':
        if (ref == QLatin1String("ont-face")) return parseFontFaceNode;
        if (ref == QLatin1String("ont-face-name")) return parseFontFaceNameNode;
        if (ref == QLatin1String("ont-face-src")) return parseFontFaceSrcNode;
        if (ref == QLatin1String("ont-face-uri")) return parseFontFaceUriNode;
        break;
    case 'g':
        if (ref == QLatin1String("lyph")) return parseGlyphNode;
        break;
    case 'm':
        if (ref == QLatin1String("issing-glyph")) return parseMissingGlyphNode;
        break;
    case 's':
        if (ref == QLatin1String("top")) return parseStopNode;
        break;
    default:
        break;
    }
    return 0;
}

QSvgHandler::QSvgHandler(QIODevice *device, QtSvg::Options options,
                         QtSvg::AnimatorType type)
    : xml(new QXmlStreamReader(device))
    , m_ownsReader(true)
    , m_options(options)
    , m_animatorType(type)
{
    init();
}

QSvgHandler::QSvgHandler(const QByteArray &data, QtSvg::Options options,
                         QtSvg::AnimatorType type)
    : xml(new QXmlStreamReader(data))
    , m_ownsReader(true)
    , m_options(options)
    , m_animatorType(type)
{
    init();
}

QSvgHandler::QSvgHandler(QXmlStreamReader *const reader, QtSvg::Options options,
                         QtSvg::AnimatorType type)
    : xml(reader)
    , m_ownsReader(false)
    , m_options(options)
    , m_animatorType(type)
{
    init();
}

void QSvgHandler::init()
{
    m_doc = 0;
    m_style = 0;
    m_animEnd = 0;
    m_defaultCoords = QSvgUtils::LT_PX;
    m_defaultPen = QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap, Qt::SvgMiterJoin);
    m_defaultPen.setMiterLimit(4);
    parse();
}

static bool detectPatternCycles(const QSvgNode *node, QList<const QSvgNode *> active = {})
{
    QSvgFillStyle *fillStyle = static_cast<QSvgFillStyle*>
        (node->styleProperty(QSvgStyleProperty::FILL));
    if (fillStyle && fillStyle->style() && fillStyle->style()->type() == QSvgStyleProperty::PATTERN) {
        QSvgPatternStyle *patternStyle = static_cast<QSvgPatternStyle *>(fillStyle->style());
        if (active.contains(patternStyle->patternNode()))
            return true;
    }

    QSvgStrokeStyle *strokeStyle = static_cast<QSvgStrokeStyle*>
        (node->styleProperty(QSvgStyleProperty::STROKE));
    if (strokeStyle && strokeStyle->style() && strokeStyle->style()->type() == QSvgStyleProperty::PATTERN) {
        QSvgPatternStyle *patternStyle = static_cast<QSvgPatternStyle *>(strokeStyle->style());
        if (active.contains(patternStyle->patternNode()))
            return true;
    }

    return false;
}

static bool detectCycles(const QSvgNode *node, QList<const QSvgNode *> active = {})
{
    if (Q_UNLIKELY(!node))
        return false;
    switch (node->type()) {
    case QSvgNode::Doc:
    case QSvgNode::Group:
    case QSvgNode::Defs:
    case QSvgNode::Pattern:
    {
        if (node->type() == QSvgNode::Pattern)
            active.append(node);

        auto *g = static_cast<const QSvgStructureNode*>(node);
        for (auto &node : g->renderers()) {
            if (detectCycles(node.get(), active))
                return true;
        }
    }
    break;
    case QSvgNode::Use:
    {
        if (active.contains(node))
            return true;

        auto *u = static_cast<const QSvgUse*>(node);
        auto *target = u->link();
        if (target) {
            active.append(u);
            if (detectCycles(target, active))
                return true;
        }
    }
    break;
    case QSvgNode::Rect:
    case QSvgNode::Ellipse:
    case QSvgNode::Circle:
    case QSvgNode::Line:
    case QSvgNode::Path:
    case QSvgNode::Polygon:
    case QSvgNode::Polyline:
    case QSvgNode::Tspan:
        if (detectPatternCycles(node, active))
            return true;
        break;
    default:
        break;
    }
    return false;
}

static bool detectCyclesAndWarn(const QSvgNode *node) {
    const bool cycleFound = detectCycles(node);
    if (cycleFound)
        qCWarning(lcSvgHandler, "Cycles detected in SVG, document discarded.");
    return cycleFound;
}

// Having too many unfinished elements will cause a stack overflow
// in the dtor of QSvgDocument, see oss-fuzz issue 24000.
static const int unfinishedElementsLimit = 2048;

void QSvgHandler::parse()
{
    xml->setNamespaceProcessing(false);
#ifndef QT_NO_CSSPARSER
    m_inStyle = false;
#endif
    bool done = false;
    int remainingUnfinishedElements = unfinishedElementsLimit;
    while (!xml->atEnd() && !done) {
        switch (xml->readNext()) {
        case QXmlStreamReader::StartElement:
            // he we could/should verify the namespaces, and simply
            // call m_skipNodes(Unknown) if we don't know the
            // namespace.  We do support http://www.w3.org/2000/svg
            // but also http://www.w3.org/2000/svg-20000303-stylable
            // And if the document uses an external dtd, the reported
            // namespaceUri is empty. The only possible strategy at
            // this point is to do what everyone else seems to do and
            // ignore the reported namespaceUri completely.
            if (remainingUnfinishedElements && startElement(xml->name(), xml->attributes())) {
                --remainingUnfinishedElements;
            } else {
                delete m_doc;
                m_doc = nullptr;
                return;
            }
            break;
        case QXmlStreamReader::EndElement:
            done = endElement(xml->name());
            ++remainingUnfinishedElements;
            break;
        case QXmlStreamReader::Characters:
            characters(xml->text());
            break;
        case QXmlStreamReader::ProcessingInstruction:
            processingInstruction(xml->processingInstructionTarget(), xml->processingInstructionData());
            break;
        default:
            break;
        }
    }
    resolvePaintServers(m_doc);
    resolveNodes();
    if (detectCyclesAndWarn(m_doc)) {
        delete m_doc;
        m_doc = nullptr;
    }
}

bool QSvgHandler::startElement(const QStringView localName,
                               const QXmlStreamAttributes &attributes)
{
    QSvgNode *node = nullptr;

    pushColorCopy();

    /* The xml:space attribute may appear on any element. We do
     * a lookup by the qualified name here, but this is namespace aware, since
     * the XML namespace can only be bound to prefix "xml." */
    const QStringView xmlSpace(attributes.value(QLatin1String("xml:space")));
    if (xmlSpace.isNull()) {
        // This element has no xml:space attribute.
        m_whitespaceMode.push(m_whitespaceMode.isEmpty() ? QSvgText::Default : m_whitespaceMode.top());
    } else if (xmlSpace == QLatin1String("preserve")) {
        m_whitespaceMode.push(QSvgText::Preserve);
    } else if (xmlSpace == QLatin1String("default")) {
        m_whitespaceMode.push(QSvgText::Default);
    } else {
        const QByteArray msg = '"' + xmlSpace.toLocal8Bit()
                               + "\" is an invalid value for attribute xml:space. "
                                 "Valid values are \"preserve\" and \"default\".";
        qCWarning(lcSvgHandler, "%s", prefixMessage(msg, xml).constData());
        m_whitespaceMode.push(QSvgText::Default);
    }

    if (!m_doc && localName != QLatin1String("svg"))
        return false;

    if (m_doc && localName == QLatin1String("svg")) {
        m_skipNodes.push(Doc);
        qCWarning(lcSvgHandler) << "Skipping a nested svg element, because "
                                   "SVG Document must not contain nested svg elements in Svg Tiny 1.2";
    }

    if (!m_skipNodes.isEmpty() && m_skipNodes.top() == Doc)
        return true;

    if (FactoryMethod method = findGroupFactory(localName, options())) {
        //group
        if (!m_doc) {
            node = method(nullptr, attributes, this);
            if (node) {
                Q_ASSERT(node->type() == QSvgNode::Doc);
                m_doc = static_cast<QSvgDocument*>(node);
            }
        } else {
            switch (m_nodes.top()->type()) {
            case QSvgNode::Doc:
            case QSvgNode::Group:
            case QSvgNode::Defs:
            case QSvgNode::Switch:
            case QSvgNode::Mask:
            case QSvgNode::Symbol:
            case QSvgNode::Marker:
            case QSvgNode::Pattern:
            {
                node = method(m_nodes.top(), attributes, this);
                if (node) {
                    QSvgStructureNode *group =
                        static_cast<QSvgStructureNode*>(m_nodes.top());
                    group->addChild(std::unique_ptr<QSvgNode>(node), someId(attributes));
                }
            }
                break;
            default:
                const QByteArray msg = QByteArrayLiteral("Could not add child element to parent element because the types are incorrect.");
                qCWarning(lcSvgHandler, "%s", prefixMessage(msg, xml).constData());
                break;
            }
        }

        if (node) {
            parseCoreNode(node, attributes);
            parseStyle(node, attributes, this);
            if (node->type() == QSvgNode::Filter)
                m_toBeResolved.append(node);
        }
    } else if (FactoryMethod method = findGraphicsFactory(localName, options())) {
        //rendering element
        Q_ASSERT(!m_nodes.isEmpty());
        switch (m_nodes.top()->type()) {
        case QSvgNode::Doc:
        case QSvgNode::Group:
        case QSvgNode::Defs:
        case QSvgNode::Switch:
        case QSvgNode::Mask:
        case QSvgNode::Symbol:
        case QSvgNode::Marker:
        case QSvgNode::Pattern:
        {
            if (localName == QLatin1String("tspan")) {
                const QByteArray msg = QByteArrayLiteral("\'tspan\' element in wrong context.");
                qCWarning(lcSvgHandler, "%s", prefixMessage(msg, xml).constData());
                break;
            }
            node = method(m_nodes.top(), attributes, this);
            if (node) {
                QSvgStructureNode *group =
                    static_cast<QSvgStructureNode*>(m_nodes.top());
                group->addChild(std::unique_ptr<QSvgNode>(node), someId(attributes));
            }
        }
            break;
        case QSvgNode::Text:
        case QSvgNode::Textarea:
            if (localName == QLatin1String("tspan")) {
                node = method(m_nodes.top(), attributes, this);
                if (node) {
                    static_cast<QSvgText *>(m_nodes.top())->addTspan(static_cast<QSvgTspan *>(node));
                }
            } else {
                const QByteArray msg = QByteArrayLiteral("\'text\' or \'textArea\' element contains invalid element type.");
                qCWarning(lcSvgHandler, "%s", prefixMessage(msg, xml).constData());
            }
            break;
        default:
            const QByteArray msg = QByteArrayLiteral("Could not add child element to parent element because the types are incorrect.");
            qCWarning(lcSvgHandler, "%s", prefixMessage(msg, xml).constData());
            break;
        }

        if (node) {
            parseCoreNode(node, attributes);
            parseStyle(node, attributes, this);
            if (node->type() == QSvgNode::Text || node->type() == QSvgNode::Textarea) {
                static_cast<QSvgText *>(node)->setWhitespaceMode(m_whitespaceMode.top());
            } else if (node->type() == QSvgNode::Tspan) {
                static_cast<QSvgTspan *>(node)->setWhitespaceMode(m_whitespaceMode.top());
            } else if (node->type() == QSvgNode::Use) {
                auto useNode = static_cast<QSvgUse *>(node);
                if (!useNode->isResolved())
                    m_toBeResolved.append(useNode);
            }
        }
    } else if (FactoryMethod method = findFilterFactory(localName, options())) {
        //filter nodes to be aded to be filtercontainer
        Q_ASSERT(!m_nodes.isEmpty());
        if (m_nodes.top()->type() == QSvgNode::Filter ||
            (m_nodes.top()->type() == QSvgNode::FeMerge && localName == QLatin1String("feMergeNode"))) {
            node = method(m_nodes.top(), attributes, this);
            if (node) {
                QSvgStructureNode *container =
                    static_cast<QSvgStructureNode*>(m_nodes.top());
                container->addChild(std::unique_ptr<QSvgNode>(node), someId(attributes));
            }
        } else {
            const QByteArray msg = QByteArrayLiteral("Could not add child element to parent element because the types are incorrect.");
            qCWarning(lcSvgHandler, "%s", prefixMessage(msg, xml).constData());
        }
    } else if (AnimationMethod method = findAnimationFactory(localName, options())) {
        Q_ASSERT(!m_nodes.isEmpty());
        node = method(m_nodes.top(), attributes, this);
        if (node) {
            QSvgAnimateNode *anim = static_cast<QSvgAnimateNode *>(node);
            if (anim->linkId().isEmpty())
                m_doc->animator()->appendAnimation(m_nodes.top(), anim);
            else if (m_doc->namedNode(anim->linkId()))
                m_doc->animator()->appendAnimation(m_doc->namedNode(anim->linkId()), anim);
            else
                m_toBeResolved.append(anim);
        }
    } else if (ParseMethod method = findUtilFactory(localName, options())) {
        Q_ASSERT(!m_nodes.isEmpty());
        if (!method(m_nodes.top(), attributes, this))
            qCWarning(lcSvgHandler, "%s", msgProblemParsing(localName, xml).constData());
    } else if (StyleFactoryMethod method = findStyleFactoryMethod(localName)) {
        QSvgStyleProperty *prop = method(m_nodes.top(), attributes, this);
        if (prop) {
            m_style = prop;
            m_nodes.top()->appendStyleProperty(prop, someId(attributes));
        } else {
            const QByteArray msg = QByteArrayLiteral("Could not parse node: ") + localName.toLocal8Bit();
            qCWarning(lcSvgHandler, "%s", prefixMessage(msg, xml).constData());
        }
    } else if (StyleParseMethod method = findStyleUtilFactoryMethod(localName)) {
        if (m_style) {
            if (!method(m_style, attributes, this))
                qCWarning(lcSvgHandler, "%s", msgProblemParsing(localName, xml).constData());
        }
    } else {
        qCDebug(lcSvgHandler) << "Skipping unknown element" << localName;
        m_skipNodes.push(Unknown);
        return true;
    }

    if (node) {
        m_nodes.push(node);
        m_skipNodes.push(Graphics);
    } else {
        //qDebug()<<"Skipping "<<localName;
        m_skipNodes.push(Style);
    }
    return true;
}

bool QSvgHandler::endElement(const QStringView localName)
{
    CurrentNode node = m_skipNodes.top();

    if (node == Doc && localName != QLatin1String("svg"))
        return false;

    m_skipNodes.pop();
    m_whitespaceMode.pop();

    popColor();

    if (node == Unknown)
        return false;

#ifdef QT_NO_CSSPARSER
    Q_UNUSED(localName);
#else
    if (m_inStyle && localName == QLatin1String("style"))
        m_inStyle = false;
#endif

    if (node == Graphics)
        m_nodes.pop();
    else if (m_style && !m_skipNodes.isEmpty() && m_skipNodes.top() != Style)
        m_style = 0;

    return ((localName == QLatin1String("svg")) && (node != Doc));
}

void QSvgHandler::resolvePaintServers(QSvgNode *node, int nestedDepth)
{
    if (!node || (node->type() != QSvgNode::Doc && node->type() != QSvgNode::Group
        && node->type() != QSvgNode::Defs && node->type() != QSvgNode::Switch)) {
        return;
    }

    QSvgStructureNode *structureNode = static_cast<QSvgStructureNode *>(node);

    for (auto &node : structureNode->renderers()) {
        QSvgFillStyle *fill = static_cast<QSvgFillStyle *>(node->styleProperty(QSvgStyleProperty::FILL));
        if (fill && !fill->isPaintStyleResolved()) {
            QString id = fill->paintStyleId();
            QSvgPaintStyleProperty *style = structureNode->styleProperty(id);
            if (style) {
                fill->setFillStyle(style);
            } else {
                qCWarning(lcSvgHandler, "%s", msgCouldNotResolveProperty(id, xml).constData());
                fill->setBrush(Qt::NoBrush);
            }
        }

        QSvgStrokeStyle *stroke = static_cast<QSvgStrokeStyle *>(node->styleProperty(QSvgStyleProperty::STROKE));
        if (stroke && !stroke->isPaintStyleResolved()) {
            QString id = stroke->paintStyleId();
            QSvgPaintStyleProperty *style = structureNode->styleProperty(id);
            if (style) {
                stroke->setStyle(style);
            } else {
                qCWarning(lcSvgHandler, "%s", msgCouldNotResolveProperty(id, xml).constData());
                stroke->setStroke(Qt::NoBrush);
            }
        }

        if (nestedDepth < 2048)
            resolvePaintServers(node.get(), nestedDepth + 1);
    }
}

void QSvgHandler::resolveNodes()
{
    for (QSvgNode *node : std::as_const(m_toBeResolved)) {
        if (node->type() == QSvgNode::Use) {
            QSvgUse *useNode = static_cast<QSvgUse *>(node);
            const auto parent = useNode->parent();
            if (!parent)
                continue;

            QSvgNode::Type t = parent->type();
            if (t != QSvgNode::Doc && t != QSvgNode::Defs && t != QSvgNode::Group && t != QSvgNode::Switch)
                continue;

            QSvgStructureNode *group = static_cast<QSvgStructureNode *>(parent);
            QSvgNode *link = group->scopeNode(useNode->linkId());
            if (!link) {
                qCWarning(lcSvgHandler, "link #%s is undefined!", qPrintable(useNode->linkId()));
                continue;
            }

            if (useNode->parent()->isDescendantOf(link))
                qCWarning(lcSvgHandler, "link #%s is recursive!", qPrintable(useNode->linkId()));

            useNode->setLink(link);
        } else if (node->type() == QSvgNode::Filter) {
            QSvgFilterContainer *filter = static_cast<QSvgFilterContainer *>(node);
            for (auto &renderer : filter->renderers()) {
                const QSvgFeFilterPrimitive *primitive = QSvgFeFilterPrimitive::castToFilterPrimitive(renderer.get());
                if (!primitive || primitive->type() == QSvgNode::FeUnsupported) {
                    filter->setSupported(false);
                    break;
                }
            }
        } else if (node->type() == QSvgNode::AnimateTransform || node->type() == QSvgNode::AnimateColor) {
            QSvgAnimateNode *anim = static_cast<QSvgAnimateNode *>(node);
            QSvgNode *targetNode = m_doc->namedNode(anim->linkId());
            if (targetNode) {
                m_doc->animator()->appendAnimation(targetNode, anim);
            } else {
                qCWarning(lcSvgHandler, "Cannot find target for link #%s!",
                          qPrintable(anim->linkId()));
                delete anim;
            }
        }
    }
    m_toBeResolved.clear();
}

bool QSvgHandler::characters(const QStringView str)
{
#ifndef QT_NO_CSSPARSER
    if (m_inStyle) {
        m_cssHandler.parseStyleSheet(str);
        return true;
    }
#endif
    if (m_skipNodes.isEmpty() || m_skipNodes.top() == Unknown || m_nodes.isEmpty())
        return true;

    if (m_nodes.top()->type() == QSvgNode::Text || m_nodes.top()->type() == QSvgNode::Textarea) {
        static_cast<QSvgText*>(m_nodes.top())->addText(str);
    } else if (m_nodes.top()->type() == QSvgNode::Tspan) {
        static_cast<QSvgTspan*>(m_nodes.top())->addText(str);
    }

    return true;
}

QIODevice *QSvgHandler::device() const
{
    return xml->device();
}

QSvgDocument *QSvgHandler::document() const
{
    return m_doc;
}

QSvgUtils::LengthType QSvgHandler::defaultCoordinateSystem() const
{
    return m_defaultCoords;
}

void QSvgHandler::setDefaultCoordinateSystem(QSvgUtils::LengthType type)
{
    m_defaultCoords = type;
}

void QSvgHandler::pushColor(const QColor &color)
{
    m_colorStack.push(color);
    m_colorTagCount.push(1);
}

void QSvgHandler::pushColorCopy()
{
    if (m_colorTagCount.size())
        ++m_colorTagCount.top();
    else
        pushColor(Qt::black);
}

void QSvgHandler::popColor()
{
    if (m_colorTagCount.size()) {
        if (!--m_colorTagCount.top()) {
            m_colorStack.pop();
            m_colorTagCount.pop();
        }
    }
}

QColor QSvgHandler::currentColor() const
{
    if (!m_colorStack.isEmpty())
        return m_colorStack.top();
    else
        return QColor(0, 0, 0);
}

#ifndef QT_NO_CSSPARSER

void QSvgHandler::setInStyle(bool b)
{
    m_inStyle = b;
}

bool QSvgHandler::inStyle() const
{
    return m_inStyle;
}

QSvgCssHandler &QSvgHandler::cssHandler()
{
    return m_cssHandler;
}

#endif // QT_NO_CSSPARSER

bool QSvgHandler::processingInstruction(const QStringView target, const QStringView data)
{
#ifdef QT_NO_CSSPARSER
    Q_UNUSED(target);
    Q_UNUSED(data);
#else
    if (target == QLatin1String("xml-stylesheet")) {
        static const QRegularExpression rx(QStringLiteral("type=\\\"(.+)\\\""),
                              QRegularExpression::InvertedGreedinessOption);
        QRegularExpressionMatchIterator iter = rx.globalMatchView(data);
        bool isCss = false;
        while (iter.hasNext()) {
            QRegularExpressionMatch match = iter.next();
            QString type = match.captured(1);
            if (type.toLower() == QLatin1String("text/css")) {
                isCss = true;
            }
        }

        if (isCss) {
            static const QRegularExpression rx(QStringLiteral("href=\\\"(.+)\\\""),
                                  QRegularExpression::InvertedGreedinessOption);
            QRegularExpressionMatch match = rx.matchView(data);
            QString addr = match.captured(1);
            QFileInfo fi(addr);
            //qDebug()<<"External CSS file "<<fi.absoluteFilePath()<<fi.exists();
            if (fi.exists()) {
                QFile file(fi.absoluteFilePath());
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    return true;
                }
                QByteArray cssData = file.readAll();
                QString css = QString::fromUtf8(cssData);
                m_cssHandler.parseStyleSheet(css);
            }

        }
    }
#endif

    return true;
}

void QSvgHandler::setAnimPeriod(int start, int end)
{
    Q_UNUSED(start);
    m_animEnd   = qMax(end, m_animEnd);
}

int QSvgHandler::animationDuration() const
{
    return m_animEnd;
}

QSvgHandler::~QSvgHandler()
{
    if(m_ownsReader)
        delete xml;
}

QT_END_NAMESPACE
