// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#undef QT_NO_FOREACH // this file contains unported legacy Q_FOREACH uses

#include "declarativetheme_p.h"

QT_BEGIN_NAMESPACE

DeclarativeTheme3D::DeclarativeTheme3D(QObject *parent)
    : Q3DTheme(parent),
      m_colors(QList<DeclarativeColor *>()),
      m_gradients(QList<ColorGradient *>()),
      m_singleHLGradient(0),
      m_multiHLGradient(0),
      m_dummyGradients(false),
      m_dummyColors(false)
{
    connect(this, &Q3DTheme::typeChanged, this, &DeclarativeTheme3D::handleTypeChange);
}

DeclarativeTheme3D::~DeclarativeTheme3D()
{
}

QQmlListProperty<QObject> DeclarativeTheme3D::themeChildren()
{
    return QQmlListProperty<QObject>(this, this, &DeclarativeTheme3D::appendThemeChildren,
                                     0, 0, 0);
}

void DeclarativeTheme3D::appendThemeChildren(QQmlListProperty<QObject> *list, QObject *element)
{
    Q_UNUSED(list);
    Q_UNUSED(element);
    // Nothing to do, themeChildren is there only to enable scoping gradient items in Theme3D item.
}

void DeclarativeTheme3D::handleTypeChange(Theme themeType)
{
    Q_UNUSED(themeType);

    // Theme changed, disconnect base color/gradient connections
    if (!m_colors.isEmpty()) {
        for (auto item: m_colors)
            disconnect(item, 0, this, 0);
        m_colors.clear();
    }
    if (!m_gradients.isEmpty()) {
        for (auto item: m_gradients)
            disconnect(item, 0, this, 0);
        m_gradients.clear();
    }
}

void DeclarativeTheme3D::handleBaseColorUpdate()
{
    int colorCount = m_colors.size();
    int changed = 0;
    // Check which one changed
    DeclarativeColor *color = qobject_cast<DeclarativeColor *>(QObject::sender());
    for (int i = 0; i < colorCount; i++) {
        if (color == m_colors.at(i)) {
            changed = i;
            break;
        }
    }
    // Update the changed one from the list
    QList<QColor> list = Q3DTheme::baseColors();
    list[changed] = m_colors.at(changed)->color();
    // Set the changed list
    Q3DTheme::setBaseColors(list);
}

void DeclarativeTheme3D::handleBaseGradientUpdate()
{
    int gradientCount = m_gradients.size();
    int changed = 0;
    // Check which one changed
    ColorGradient *gradient = qobject_cast<ColorGradient *>(QObject::sender());
    for (int i = 0; i < gradientCount; i++) {
        if (gradient == m_gradients.at(i)) {
            changed = i;
            break;
        }
    }
    // Update the changed one from the list
    QList<QLinearGradient> list = Q3DTheme::baseGradients();
    list[changed] = convertGradient(gradient);
    // Set the changed list
    Q3DTheme::setBaseGradients(list);
}

void DeclarativeTheme3D::handleSingleHLGradientUpdate()
{
    if (m_singleHLGradient)
        setThemeGradient(m_singleHLGradient, GradientTypeSingleHL);
}

void DeclarativeTheme3D::handleMultiHLGradientUpdate()
{
    if (m_multiHLGradient)
        setThemeGradient(m_multiHLGradient, GradientTypeMultiHL);
}

void DeclarativeTheme3D::setSingleHighlightGradient(ColorGradient *gradient)
{
    // connect new / disconnect old
    if (gradient != m_singleHLGradient) {
        if (m_singleHLGradient)
            QObject::disconnect(m_singleHLGradient, 0, this, 0);

        m_singleHLGradient = gradient;

        if (m_singleHLGradient) {
            QObject::connect(m_singleHLGradient, &ColorGradient::updated, this,
                             &DeclarativeTheme3D::handleSingleHLGradientUpdate);
        }

        emit singleHighlightGradientChanged(m_singleHLGradient);
    }

    if (m_singleHLGradient)
        setThemeGradient(m_singleHLGradient, GradientTypeSingleHL);
}

ColorGradient *DeclarativeTheme3D::singleHighlightGradient() const
{
    return m_singleHLGradient;
}

void DeclarativeTheme3D::setMultiHighlightGradient(ColorGradient *gradient)
{
    // connect new / disconnect old
    if (gradient != m_multiHLGradient) {
        if (m_multiHLGradient)
            QObject::disconnect(m_multiHLGradient, 0, this, 0);

        m_multiHLGradient = gradient;

        if (m_multiHLGradient) {
            QObject::connect(m_multiHLGradient, &ColorGradient::updated, this,
                             &DeclarativeTheme3D::handleMultiHLGradientUpdate);
        }

        emit multiHighlightGradientChanged(m_multiHLGradient);
    }

    if (m_multiHLGradient)
        setThemeGradient(m_multiHLGradient, GradientTypeMultiHL);
}

ColorGradient *DeclarativeTheme3D::multiHighlightGradient() const
{
    return m_multiHLGradient;
}

void DeclarativeTheme3D::classBegin()
{
    // Turn off predefined type forcing for the duration of initial class construction
    // so that predefined type customization can be done.
    d_ptr->setForcePredefinedType(false);
}

void DeclarativeTheme3D::componentComplete()
{
    d_ptr->setForcePredefinedType(true);
}


void DeclarativeTheme3D::setThemeGradient(ColorGradient *gradient, GradientType type)
{
    QLinearGradient newGradient = convertGradient(gradient);

    switch (type) {
    case GradientTypeSingleHL:
        Q3DTheme::setSingleHighlightGradient(newGradient);
        break;
    case GradientTypeMultiHL:
        Q3DTheme::setMultiHighlightGradient(newGradient);
        break;
    default:
        qWarning("Incorrect usage. Type may be GradientTypeSingleHL or GradientTypeMultiHL.");
        break;
    }
}

QLinearGradient DeclarativeTheme3D::convertGradient(ColorGradient *gradient)
{
    QLinearGradient newGradient;
    QGradientStops stops;
    QList<ColorGradientStop *> qmlstops = gradient->m_stops;

    // Get sorted gradient stops
    for (int i = 0; i < qmlstops.size(); i++) {
        int j = 0;
        while (j < stops.size() && stops.at(j).first < qmlstops[i]->position())
            j++;
        stops.insert(j, QGradientStop(qmlstops.at(i)->position(), qmlstops.at(i)->color()));
    }

    newGradient.setStops(stops);

    return newGradient;
}

ColorGradient *DeclarativeTheme3D::convertGradient(const QLinearGradient &gradient)
{
    ColorGradient *newGradient = new ColorGradient(this);
    QGradientStops stops = gradient.stops();
    ColorGradientStop *qmlstop;

    // Convert stops
    for (int i = 0; i < stops.size(); i++) {
        qmlstop = new ColorGradientStop(newGradient);
        qmlstop->setColor(stops.at(i).second);
        qmlstop->setPosition(stops.at(i).first);
        newGradient->m_stops.append(qmlstop);
    }

    return newGradient;
}

void DeclarativeTheme3D::addColor(DeclarativeColor *color)
{
    if (!color) {
        qWarning("Color is invalid, use ThemeColor");
        return;
    }
    clearDummyColors();
    m_colors.append(color);
    connect(color, &DeclarativeColor::colorChanged,
            this, &DeclarativeTheme3D::handleBaseColorUpdate);
    QList<QColor> list = Q3DTheme::baseColors();
    list.append(color->color());
    Q3DTheme::setBaseColors(list);
}

QList<DeclarativeColor *> DeclarativeTheme3D::colorList()
{
    if (m_colors.isEmpty()) {
        // Create dummy ThemeColors from theme's colors
        m_dummyColors = true;
        QList<QColor> list = Q3DTheme::baseColors();
        for (auto item: list) {
            DeclarativeColor *color = new DeclarativeColor(this);
            color->setColor(item);
            m_colors.append(color);
            connect(color, &DeclarativeColor::colorChanged,
                    this, &DeclarativeTheme3D::handleBaseColorUpdate);
        }
    }
    return m_colors;
}

void DeclarativeTheme3D::clearColors()
{
    clearDummyColors();
    for (auto item: m_colors)
        disconnect(item, 0, this, 0);
    m_colors.clear();
    Q3DTheme::setBaseColors(QList<QColor>());
}

void DeclarativeTheme3D::clearDummyColors()
{
    if (m_dummyColors) {
        for (auto item: m_colors)
            delete item;
        m_colors.clear();
        m_dummyColors = false;
    }
}

void DeclarativeTheme3D::addGradient(ColorGradient *gradient)
{
    if (!gradient) {
        qWarning("Gradient is invalid, use ColorGradient");
        return;
    }
    clearDummyGradients();
    m_gradients.append(gradient);
    connect(gradient, &ColorGradient::updated,
            this, &DeclarativeTheme3D::handleBaseGradientUpdate);
    QList<QLinearGradient> list = Q3DTheme::baseGradients();
    list.append(convertGradient(gradient));
    Q3DTheme::setBaseGradients(list);
}

QList<ColorGradient *> DeclarativeTheme3D::gradientList()
{
    if (m_gradients.isEmpty()) {
        // Create dummy ColorGradients from theme's gradients
        m_dummyGradients = true;
        QList<QLinearGradient> list = Q3DTheme::baseGradients();
        for (auto item: list) {
            ColorGradient *gradient = convertGradient(item);
            m_gradients.append(gradient);
            connect(gradient, &ColorGradient::updated,
                    this, &DeclarativeTheme3D::handleBaseGradientUpdate);
        }
    }

    return m_gradients;
}

void DeclarativeTheme3D::clearGradients()
{
    clearDummyGradients();
    for (auto item: m_gradients)
        disconnect(item, 0, this, 0);
    m_gradients.clear();
    Q3DTheme::setBaseGradients(QList<QLinearGradient>());
}

void DeclarativeTheme3D::clearDummyGradients()
{
    if (m_dummyGradients) {
        for (auto item: m_gradients)
            delete item;
        m_gradients.clear();
        m_dummyGradients = false;
    }
}

QQmlListProperty<DeclarativeColor> DeclarativeTheme3D::baseColors()
{
    return QQmlListProperty<DeclarativeColor>(this, this,
                                              &DeclarativeTheme3D::appendBaseColorsFunc,
                                              &DeclarativeTheme3D::countBaseColorsFunc,
                                              &DeclarativeTheme3D::atBaseColorsFunc,
                                              &DeclarativeTheme3D::clearBaseColorsFunc);
}

void DeclarativeTheme3D::appendBaseColorsFunc(QQmlListProperty<DeclarativeColor> *list,
                                              DeclarativeColor *color)
{
    reinterpret_cast<DeclarativeTheme3D *>(list->data)->addColor(color);
}

qsizetype DeclarativeTheme3D::countBaseColorsFunc(QQmlListProperty<DeclarativeColor> *list)
{
    return reinterpret_cast<DeclarativeTheme3D *>(list->data)->colorList().size();
}

DeclarativeColor *DeclarativeTheme3D::atBaseColorsFunc(QQmlListProperty<DeclarativeColor> *list,
                                                       qsizetype index)
{
    return reinterpret_cast<DeclarativeTheme3D *>(list->data)->colorList().at(index);
}

void DeclarativeTheme3D::clearBaseColorsFunc(QQmlListProperty<DeclarativeColor> *list)
{
    reinterpret_cast<DeclarativeTheme3D *>(list->data)->clearColors();
}

QQmlListProperty<ColorGradient> DeclarativeTheme3D::baseGradients()
{
    return QQmlListProperty<ColorGradient>(this, this,
                                           &DeclarativeTheme3D::appendBaseGradientsFunc,
                                           &DeclarativeTheme3D::countBaseGradientsFunc,
                                           &DeclarativeTheme3D::atBaseGradientsFunc,
                                           &DeclarativeTheme3D::clearBaseGradientsFunc);
}

void DeclarativeTheme3D::appendBaseGradientsFunc(QQmlListProperty<ColorGradient> *list,
                                                 ColorGradient *gradient)
{
    reinterpret_cast<DeclarativeTheme3D *>(list->data)->addGradient(gradient);
}

qsizetype DeclarativeTheme3D::countBaseGradientsFunc(QQmlListProperty<ColorGradient> *list)
{
    return reinterpret_cast<DeclarativeTheme3D *>(list->data)->gradientList().size();
}

ColorGradient *DeclarativeTheme3D::atBaseGradientsFunc(QQmlListProperty<ColorGradient> *list,
                                                       qsizetype index)
{
    return reinterpret_cast<DeclarativeTheme3D *>(list->data)->gradientList().at(index);
}

void DeclarativeTheme3D::clearBaseGradientsFunc(QQmlListProperty<ColorGradient> *list)
{
    reinterpret_cast<DeclarativeTheme3D *>(list->data)->clearGradients();
}

QT_END_NAMESPACE
