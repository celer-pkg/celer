// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottiebase_p.h"

#include <QLoggingCategory>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

Q_LOGGING_CATEGORY(lcLottieQtLottieParser, "qt.lottieqt.lottie.parser", QtWarningMsg);
Q_LOGGING_CATEGORY(lcLottieQtLottieUpdate, "qt.lottieqt.lottie.update");
Q_LOGGING_CATEGORY(lcLottieQtLottieRender, "qt.lottieqt.lottie.render");

QLottieBase::QLottieBase(const QLottieBase &other)
{
    m_definition = other.m_definition;
    m_type = other.m_type;
    m_hidden = other.m_hidden;
    m_name = other.m_name;
    m_autoOrient = other.m_autoOrient;
    for (QLottieBase *child : other.m_children) {
        QLottieBase *clone = child->clone();
        clone->setParent(this);
        appendChild(clone);
    }
}

QLottieBase::~QLottieBase()
{
    qDeleteAll(m_children);
}

QLottieBase *QLottieBase::clone() const
{
    return new QLottieBase(*this);
}

QString QLottieBase::name() const
{
    return m_name;
}

void QLottieBase::setName(const QString &name)
{
    m_name = name;
}

bool QLottieBase::setProperty(QLottieLiteral::PropertyType propertyName, QVariant value)
{
    for (QLottieBase *child : std::as_const(m_children)) {
        bool changed = child->setProperty(propertyName, value);
        if (changed)
            return true;
    }
    return false;
}

int QLottieBase::type() const
{
    return m_type;
}

void QLottieBase::setType(int type)
{
    m_type = type;
}

bool QLottieBase::isShapeElement() const
{
    return (m_type >= LOTTIE_SHAPE_ELLIPSE_IX && m_type <= LOTTIE_SHAPE_REPEATER_IX);
}

bool QLottieBase::isPathElement() const
{
    return (m_type == LOTTIE_SHAPE_ELLIPSE_IX || m_type == LOTTIE_SHAPE_RECT_IX
            || m_type == LOTTIE_SHAPE_SHAPE_IX || m_type == LOTTIE_SHAPE_STAR_IX);
}

void QLottieBase::prependChild(QLottieBase *child)
{
    m_children.push_front(child);
}

void QLottieBase::insertChildBeforeLast(QLottieBase *child)
{
    m_children.insert(qMax(m_children.size() - 1, 0), child);
}

void QLottieBase::appendChild(QLottieBase *child)
{
    m_children.push_back(child);
}

QLottieBase *QLottieBase::findChild(const QString &childName)
{
    if (name() == childName)
        return this;

    QLottieBase *found = nullptr;
    for (QLottieBase *child : std::as_const(m_children)) {
        found = child->findChild(childName);
        if (found)
            break;
    }
    return found;
}

void QLottieBase::updateProperties(int frame)
{
    if (m_hidden)
        return;

    for (QLottieBase *child : std::as_const(m_children))
        child->updateProperties(frame);
}

void QLottieBase::render(QLottieRenderer &renderer) const
{
    if (m_hidden)
        return;

    renderer.saveState();
    renderChildren(renderer);
    renderer.restoreState();
}

void QLottieBase::renderChildren(QLottieRenderer &renderer) const
{
    QList<QLottieBase *> pathElements;
    for (QLottieBase *child : std::as_const(m_children)) {
        if (child->isPathElement()) {
            if (!child->m_hidden)
                pathElements.prepend(child);
        } else {
            if (!pathElements.isEmpty()) {
                renderer.renderPathElements(pathElements);
                pathElements.clear();
            }
            if (!child->m_hidden)
                child->render(renderer);
        }
    }
    if (!pathElements.isEmpty())
        renderer.renderPathElements(pathElements);
}

bool QLottieBase::isStructureDumping() const
{
    if (m_structureDumping < 0) {
        QLottieBase *p = parent();
        m_structureDumping = p ? p->isStructureDumping() : 0;
    }
    return bool(m_structureDumping);
}

void QLottieBase::resolveTopRoot()
{
    if (!m_topRoot) {
        QLottieBase *p = this;
        while (p) {
            m_topRoot = p;
            p = p->parent();
        }
    }
    Q_ASSERT(m_topRoot);
}

QLottieBase *QLottieBase::topRoot() const
{
    return m_topRoot;
}

int QLottieBase::parse(const QJsonObject &definition)
{
    qCDebug(lcLottieQtLottieParser) << "QLottieBase::parse()";

    m_definition = definition;

    m_hidden = definition.value("hd"_L1).toBool(false);
    m_name = definition.value("nm"_L1).toString();
    m_matchName = definition.value("mn"_L1).toString();
    m_autoOrient = definition.value("ao"_L1).toBool();

    if (m_autoOrient)
        qCInfo(lcLottieQtLottieParser, "Element has auto-orientation set, but it is not supported");

    return 0;
}

const QJsonObject &QLottieBase::definition() const
{
    return m_definition;
}

bool QLottieBase::active(int frame) const
{
    Q_UNUSED(frame);
    return !m_hidden;
}

bool QLottieBase::hidden() const
{
    return m_hidden;
}

QSize QLottieBase::layerSize() const
{
    return m_parent ? m_parent->layerSize() : QSize();
}

void QLottieBase::setParent(QLottieBase *parent)
{
    m_parent = parent;
}


const QJsonObject QLottieBase::resolveExpression(const QJsonObject &definition)
{
    QString expr = definition.value("x"_L1).toString();

    // If there is no expression, return the original object definition
    if (expr.isEmpty())
        return definition;

    // Find out layer handle
    resolveTopRoot();

    QRegularExpression re(QStringLiteral("effect\\(\\'(.*?)\\'\\)\\(\\'(.*?)\\'\\)"));
    QRegularExpressionMatch match = re.match(expr);
    if (!match.hasMatch())
        return definition;

    QString effect = match.captured(1);
    QString elementName = match.captured(2);

    QJsonObject retVal = definition;

    if (QLottieBase *source = m_topRoot->findChild(effect)) {
        if (source->children().size())
            retVal = source->children().at(0)->definition().value("v"_L1).toObject();
        else
            retVal = source->definition().value("v"_L1).toObject();
        if (source->children().size() > 1)
            qCInfo(lcLottieQtLottieParser) << "Effect source points"
                                                "to a group that has"
                                                "many children. The"
                                                "first is be picked";
    } else {
        qCInfo(lcLottieQtLottieParser) << "Failed to find specified effect" << effect;
    }

    // Let users of the json know that it is originated from expression,
    // so they can adjust their behavior accordingly
    retVal.insert("fromExpression"_L1, true);

    return retVal;
}

bool QLottieBase::checkRequiredKeys(const QJsonObject &definition, const QLatin1StringView type,
                                    const QList<QLatin1StringView> &keys, const QString &name) const
{
    for (auto key : keys) {
        if (!definition.contains(key)) {
            qCWarning(lcLottieQtLottieParser) << type << name << "is missing required key \"" << key << "\"";
            return false;
        }
    }
    return true;
}

QT_END_NAMESPACE
