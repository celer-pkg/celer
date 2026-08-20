// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qsvgcsshandler_p.h"
#include <QtSvg/private/qsvgstyleselector_p.h>
#include <QtSvg/private/qsvganimatedproperty_p.h>
#include <QtSvg/private/qsvgutils_p.h>
#include <QtGui/private/qmath_p.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

namespace {

struct CssKeyFrameValue{
    qreal keyFrame;
    QList<QCss::Value> values;
};

bool fillColorProperty(const QList<CssKeyFrameValue> &keyFrames, QSvgAnimatedPropertyColor *prop)
{
    for (CssKeyFrameValue keyFrame : keyFrames) {
        if (keyFrame.values.size() != 1)
            return false;

        QString colorStr = keyFrame.values.first().toString();
        QColor color = QColor::fromString(colorStr);
        prop->appendColor(color);
        prop->appendKeyFrame(keyFrame.keyFrame);
    }

    return true;
}

bool fillOpacityProperty(const QList<CssKeyFrameValue> &keyFrames, QSvgAnimatedPropertyFloat *prop)
{
    for (CssKeyFrameValue keyFrame : keyFrames) {
        if (keyFrame.values.size() != 1)
            return false;

        QString opacity = keyFrame.values.first().toString();
        prop->appendValue(opacity.toDouble());
        prop->appendKeyFrame(keyFrame.keyFrame);
    }

    return true;
}

bool validateTransform(QList<QList<QSvgAnimatedPropertyTransform::TransformComponent>> &keyFrameComponents) {

    if (keyFrameComponents.size() < 2)
        return false;

    qsizetype maxIndex = 0;
    qsizetype maxSize = 0;
    for (int i = 1; i < keyFrameComponents.size(); i++) {
        auto &listA = keyFrameComponents[i - 1];
        auto &listB = keyFrameComponents[i];
        for (int j = 0; j < qMin(listA.size(), listB.size()); j++) {
            auto typeA = listA.at(j).type;
            auto typeB = listB.at(j).type;
            // TODO: Handle type mismatch as mentioned in CSS Transform Module specs.
            if (typeA != typeB)
                return false;
        }

        if (listA.size() > maxSize) {
            maxIndex = i - 1;
            maxSize = listA.size();
        }

        if (listB.size() > maxSize) {
            maxIndex = i;
            maxSize = listB.size();
        }
    }

    const auto &longList = keyFrameComponents.at(maxIndex);
    // pad the shorter list with identical transforms set to default values.
    for (auto &list : keyFrameComponents) {
        qsizetype size = list.size();

        for (int j = size; j < maxSize; j++) {
            QSvgAnimatedPropertyTransform::TransformComponent comp = longList.value(j);
            switch (comp.type) {
            case QSvgAnimatedPropertyTransform::TransformComponent::Translate:
            case QSvgAnimatedPropertyTransform::TransformComponent::Skew:
                comp.values[0] = 0;
                comp.values[1] = 0;
                break;
            case QSvgAnimatedPropertyTransform::TransformComponent::Rotate:
                comp.values[0] = 0;
                comp.values[1] = 0;
                comp.values[2] = 0;
                break;
            case QSvgAnimatedPropertyTransform::TransformComponent::Scale:
                comp.values[0] = 1;
                comp.values[1] = 1;
                break;
            default:
                break;
            }
            list.append(comp);
        }
    }

    return true;
}

bool fillTransformProperty(const QList<CssKeyFrameValue> &keyFrames, QSvgAnimatedPropertyTransform *prop)
{
    // Stores each key frame's list of components
    QList<QList<QSvgAnimatedPropertyTransform::TransformComponent>> keyFramesComponents;

    for (const CssKeyFrameValue &keyFrame : keyFrames) {
        QList<QSvgAnimatedPropertyTransform::TransformComponent> components;
        for (const QCss::Value &val : keyFrame.values) {
            if (val.type == QCss::Value::Function) {
                QStringList lst = val.variant.toStringList();
                QStringView transformType = lst.value(0);
                QStringList args = lst.value(1).split(QStringLiteral(","), Qt::SkipEmptyParts);
                if (transformType == QStringLiteral("scale")) {
                    QSvgAnimatedPropertyTransform::TransformComponent component;
                    qreal scale0 = QSvgUtils::toDouble(args.value(0).trimmed());
                    qreal scale1 = QSvgUtils::toDouble(args.value(1).trimmed());
                    component.type = QSvgAnimatedPropertyTransform::TransformComponent::Scale;
                    component.values.append(scale0);
                    component.values.append(scale1);
                    components.append(component);
                } else if (transformType == QStringLiteral("translate")) {
                    QSvgAnimatedPropertyTransform::TransformComponent component;
                    QSvgUtils::LengthType type;
                    qreal translate0 = QSvgUtils::parseLength(args.value(0), &type);
                    translate0 = QSvgUtils::convertToPixels(translate0, false, type);
                    qreal translate1 = QSvgUtils::parseLength(args.value(1), &type);
                    translate1 = QSvgUtils::convertToPixels(translate1, false, type);
                    component.type = QSvgAnimatedPropertyTransform::TransformComponent::Translate;
                    component.values.append(translate0);
                    component.values.append(translate1);
                    components.append(component);
                } else if (transformType == QStringLiteral("rotate")) {
                    QSvgAnimatedPropertyTransform::TransformComponent component;
                    qreal rotationAngle = QSvgUtils::parseAngle(args.value(0)).value_or(0);
                    component.type = QSvgAnimatedPropertyTransform::TransformComponent::Rotate;
                    component.values.append(rotationAngle);
                    component.values.append(0);
                    component.values.append(0);
                    components.append(component);
                } else if (transformType == QStringLiteral("skew")) {
                    QSvgAnimatedPropertyTransform::TransformComponent component;
                    qreal skew0 = QSvgUtils::parseAngle(args.value(0)).value_or(0);
                    qreal skew1 = QSvgUtils::parseAngle(args.value(1)).value_or(0);
                    component.type = QSvgAnimatedPropertyTransform::TransformComponent::Skew;
                    component.values.append(skew0);
                    component.values.append(skew1);
                    components.append(component);
                } else if (transformType == QStringLiteral("matrix")) {
                    QSvgAnimatedPropertyTransform::TransformComponent component1, component2, component3;
                    QSvgUtils::LengthType type;
                    qreal translate0 = QSvgUtils::parseLength(args.value(4), &type);
                    translate0 = QSvgUtils::convertToPixels(translate0, false, type);
                    qreal translate1 = QSvgUtils::parseLength(args.value(5), &type);
                    translate1 = QSvgUtils::convertToPixels(translate1, false, type);
                    qreal scale0 = QSvgUtils::toDouble(args.value(0).trimmed());
                    qreal scale1 = QSvgUtils::toDouble(args.value(3).trimmed());
                    qreal skew0 = QSvgUtils::toDouble((args.value(1).trimmed()));
                    qreal skew1 = QSvgUtils::toDouble((args.value(2).trimmed()));
                    component1.type = QSvgAnimatedPropertyTransform::TransformComponent::Translate;
                    component1.values.append(translate0);
                    component1.values.append(translate1);
                    component2.type = QSvgAnimatedPropertyTransform::TransformComponent::Scale;
                    component2.values.append(scale0);
                    component2.values.append(scale1);
                    component3.type = QSvgAnimatedPropertyTransform::TransformComponent::Skew;
                    component3.values.append(skew0);
                    component3.values.append(skew1);
                    components.append(component1);
                    components.append(component2);
                    components.append(component3);
                }
            }
        }
        keyFramesComponents.append(components);
        prop->appendKeyFrame(keyFrame.keyFrame);
    }

    if (!validateTransform(keyFramesComponents))
        return false;

    for (const auto &comp : std::as_const(keyFramesComponents)) {
        prop->appendComponents(comp);
    }
    prop->setTransformCount(keyFramesComponents.first().size());

    return true;
}

bool fillOffsetDistanceProperty(const QList<CssKeyFrameValue> &keyFrames, QSvgAnimatedPropertyFloat *prop)
{
    for (const CssKeyFrameValue &keyFrame : keyFrames) {
        if (keyFrame.values.size() != 1)
            return false;

        QString offsetDistance = keyFrame.values.first().toString();

        bool ok = false;
        qreal distance = offsetDistance.toDouble(&ok);
        if (!ok)
            return false;

        QCss::Value::Type type = keyFrame.values.first().type;
        if (type != QCss::Value::Percentage && !qFuzzyCompare(distance, 0.))
            return false;
        distance /= 100;
        prop->appendValue(distance);
        prop->appendKeyFrame(keyFrame.keyFrame);
    }

    return true;
}

}

QSvgCssHandler::QSvgCssHandler()
    : m_selector(new QSvgStyleSelector)
{

}

QSvgCssHandler::~QSvgCssHandler()
{
    delete m_selector;
    m_selector = nullptr;
}

QSvgCssAnimation *QSvgCssHandler::createAnimation(QStringView name)
{
    if (!m_animations.contains(name))
        return nullptr;

    QCss::AnimationRule animationRule = m_animations[name];
    QHash<QString, QSvgAbstractAnimatedProperty*> animatedProperies;
    QSvgCssAnimation *animation = new QSvgCssAnimation;


    // Css Parser returns a list of all properties for each key frame. Here,
    // we store the key frames and values for each property for easier parsing.
    QHash<QString, QList<CssKeyFrameValue>> keyFrameValues;
    for (const auto &ruleSet : std::as_const(animationRule.ruleSets)) {
        for (QCss::Declaration decl : ruleSet.declarations) {
            CssKeyFrameValue keyFrameValue = {ruleSet.keyFrame, decl.d->values};
            QList<CssKeyFrameValue> &value = keyFrameValues[decl.d->property];
            value.append(keyFrameValue);
        }
    }

    for (auto it = keyFrameValues.begin(); it != keyFrameValues.end(); it++) {
        QStringView property = it.key();
        const QList<CssKeyFrameValue> &keyFrames = it.value();
        auto *prop = QSvgAbstractAnimatedProperty::createAnimatedProperty(property.toString());
        if (!prop)
            continue;

        bool result = false;
        if (property == QLatin1StringView("fill") || property == QLatin1StringView("stroke"))
            result = fillColorProperty(keyFrames, static_cast<QSvgAnimatedPropertyColor*>(prop));
        else if (property == QLatin1StringView("transform"))
            result = fillTransformProperty(keyFrames, static_cast<QSvgAnimatedPropertyTransform*>(prop));
        else if (property == QLatin1StringView("fill-opacity") || property == QLatin1StringView("stroke-opacity")
                 || property == QLatin1StringView("opacity"))
            result = fillOpacityProperty(keyFrames, static_cast<QSvgAnimatedPropertyFloat*>(prop));
        else if (property == QLatin1StringView("offset-distance"))
            result = fillOffsetDistanceProperty(keyFrames, static_cast<QSvgAnimatedPropertyFloat*>(prop));

        if (!result) {
            delete prop;
            continue;
        }

        animatedProperies[property] = prop;
    }

    for (auto it = animatedProperies.begin(); it != animatedProperies.end(); it++)
        animation->appendProperty(it.value());

    return animation;
}

QSvgCssEasingPtr QSvgCssHandler::createEasing(QSvgCssValues::EasingFunction easingFunction,
                                              const QSvgCssValues::EasingValues &values)
{
    QSvgCssEasingPtr easing;

    switch (easingFunction) {
    case QSvgCssValues::EasingFunction::Ease:
    case QSvgCssValues::EasingFunction::EaseIn:
    case QSvgCssValues::EasingFunction::EaseOut:
    case QSvgCssValues::EasingFunction::EaseInOut:
    case QSvgCssValues::EasingFunction::Linear:
        easing = createEasingFromKeyword(easingFunction);
        break;
    case QSvgCssValues::EasingFunction::Steps:
        easing = createStepsEasing(std::get<QSvgCssValues::StepValues>(values));
        break;
    default:
        easing = createEasingFromKeyword(QSvgCssValues::EasingFunction::Ease);
        break;
    }

    return easing;
}

void QSvgCssHandler::collectAnimations(const QCss::StyleSheet &sheet)
{
    auto sortFunction = [](QCss::AnimationRule::AnimationRuleSet r1, QCss::AnimationRule::AnimationRuleSet r2) {
        return r1.keyFrame < r2.keyFrame;
    };

    QList<QCss::AnimationRule> animationRules = sheet.animationRules;
    for (QCss::AnimationRule &rule : animationRules) {
        std::sort(rule.ruleSets.begin(), rule.ruleSets.end(), sortFunction);
        m_animations[rule.animName] = rule;
    }
}

void QSvgCssHandler::parseStyleSheet(const QStringView str)
{
    QString css = str.toString();
    QCss::StyleSheet sheet;
    QCss::Parser(css).parse(&sheet);
    m_selector->styleSheets.append(sheet);

    collectAnimations(sheet);
}

void QSvgCssHandler::parseCSStoXMLAttrs(const QList<QCss::Declaration> &declarations, QXmlStreamAttributes &attributes) const
{
    for (int i = 0; i < declarations.size(); ++i) {
        const QCss::Declaration &decl = declarations.at(i);
        if (decl.d->property.isEmpty())
            continue;
        QString valueStr;
        const int valCount = decl.d->values.size();
        for (int i = 0; i < valCount; ++i) {
            QCss::Value val = decl.d->values.at(i);
            switch (val.type) {
            case QCss::Value::TermOperatorComma:
                valueStr += QLatin1Char(';');
                break;
            case QCss::Value::Uri:
            {
                QString temp = val.toString();
                temp.prepend(QLatin1String("url("));
                temp.append(QLatin1Char(')'));
                valueStr += temp;
                break;
            }
            case QCss::Value::Function:
            {
                QStringList lst = val.variant.toStringList();
                valueStr.append(lst.at(0));
                valueStr.append(QLatin1Char('('));
                for (int i = 1; i < lst.size(); ++i) {
                    valueStr.append(lst.at(i));
                    if ((i +1) < lst.size())
                        valueStr.append(QLatin1Char(','));
                }
                valueStr.append(QLatin1Char(')'));
                break;
            }
            case QCss::Value::KnownIdentifier:
                switch (val.variant.toInt()) {
                case QCss::Value_None:
                    valueStr += QLatin1String("none");
                    break;
                case QCss::Value_Auto:
                    valueStr += QLatin1String("auto");
                    break;
                default:
                    valueStr += val.toString();
                    break;
                }
                break;
            case QCss::Value::Percentage:
                valueStr += val.toString() + QLatin1Char('%');
                break;
            default:
                valueStr += val.toString();
                break;
            }

            if (i + 1 < valCount)
                valueStr += QLatin1Char(' ');
        }

        attributes.append(QString(), decl.d->property, valueStr);
    }
}

void QSvgCssHandler::parseCSStoXMLAttrs(const QString &css, QXmlStreamAttributes &attributes) const
{
    // preprocess (for unicode escapes), tokenize and remove comments
    QCss::Parser parser(css);
    QString key;

    while (parser.hasNext()) {
        parser.skipSpace();

        if (!parser.hasNext())
            break;
        parser.next();

        QString name;
        QString value;

        if (parser.hasEscapeSequences) {
            key = parser.lexem();
            name = key;
        } else {
            const QCss::Symbol &sym = parser.symbol();
            name = sym.text.mid(sym.start, sym.len);
        }

        parser.skipSpace();
        if (!parser.test(QCss::COLON))
            break;

        parser.skipSpace();
        if (!parser.hasNext())
            break;

        const int firstSymbol = parser.index;
        int symbolCount = 0;
        do {
            parser.next();
            ++symbolCount;
        } while (parser.hasNext() && !parser.test(QCss::SEMICOLON));

        bool canExtractValueByRef = !parser.hasEscapeSequences;
        if (canExtractValueByRef) {
            int len = parser.symbols.at(firstSymbol).len;
            for (int i = firstSymbol + 1; i < firstSymbol + symbolCount; ++i) {
                len += parser.symbols.at(i).len;

                if (parser.symbols.at(i - 1).start + parser.symbols.at(i - 1).len
                    != parser.symbols.at(i).start) {
                    canExtractValueByRef = false;
                    break;
                }
            }
            if (canExtractValueByRef) {
                const QCss::Symbol &sym = parser.symbols.at(firstSymbol);
                value = sym.text.mid(sym.start, len);
            }
        }
        if (!canExtractValueByRef) {

            for (int i = firstSymbol; i < parser.index - 1; ++i)
                value += parser.symbols.at(i).lexem();
        }

        attributes.append(QString(), name, value);

        parser.skipSpace();
    }
}

void QSvgCssHandler::styleLookup(QSvgNode *node, QXmlStreamAttributes &attributes) const
{
    QCss::StyleSelector::NodePtr cssNode;
    cssNode.ptr = node;
    QList<QCss::Declaration> decls = m_selector->declarationsForNode(cssNode);

    parseCSStoXMLAttrs(decls, attributes);
}

QSvgCssEasingPtr QSvgCssHandler::createEasingFromKeyword(QSvgCssValues::EasingFunction easingFunction)
{
    constexpr QPointF easeC1(0.25, 0.1);
    constexpr QPointF easeC2(0.25, 1);
    constexpr QPointF easeInC1(0.42, 0);
    constexpr QPointF easeInC2(1, 1);
    constexpr QPointF easeOutC1(0, 0);
    constexpr QPointF easeOutC2(0.58, 1);
    constexpr QPointF linearC1(0, 0);
    constexpr QPointF linearC2(1, 1);

    QSvgCssEasingPtr easing;

    switch (easingFunction) {
    case QSvgCssValues::EasingFunction::Ease:
        easing = std::make_unique<QSvgCssCubicBezierEasing>(easingFunction, easeC1, easeC2);
        break;
    case QSvgCssValues::EasingFunction::EaseIn:
        easing = std::make_unique<QSvgCssCubicBezierEasing>(easingFunction, easeInC1, easeInC2);
        break;
    case QSvgCssValues::EasingFunction::EaseOut:
        easing = std::make_unique<QSvgCssCubicBezierEasing>(easingFunction, easeOutC1, easeOutC2);
        break;
    case QSvgCssValues::EasingFunction::EaseInOut:
        easing = std::make_unique<QSvgCssCubicBezierEasing>(easingFunction, easeInC1, easeOutC2);
        break;
    case QSvgCssValues::EasingFunction::Linear:
        easing = std::make_unique<QSvgCssCubicBezierEasing>(easingFunction, linearC1, linearC2);
        break;
    default:
        Q_UNREACHABLE();
        break;
    }

    return easing;
}

QSvgCssEasingPtr QSvgCssHandler::createStepsEasing(const QSvgCssValues::StepValues &values)
{
    quint32 steps = values.steps;
    QSvgCssValues::StepPosition position = values.stepPosition;

    return std::make_unique<QSvgCssStepsEasing>(steps, position);
}

QT_END_NAMESPACE
