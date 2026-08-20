// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef DECLARATIVETHEME_P_H
#define DECLARATIVETHEME_P_H

#include <private/datavisualizationglobal_p.h>
#include <private/q3dtheme_p.h>

#include "declarativecolor_p.h"
#include "colorgradient_p.h"

#include <QtQml/qqml.h>
#include <QtQml/qqmlparserstatus.h>

QT_BEGIN_NAMESPACE

class DeclarativeTheme3D : public Q3DTheme, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QQmlListProperty<QObject> themeChildren READ themeChildren)
    Q_PROPERTY(QQmlListProperty<DeclarativeColor> baseColors READ baseColors)
    Q_PROPERTY(QQmlListProperty<ColorGradient> baseGradients READ baseGradients)
    Q_PROPERTY(ColorGradient *singleHighlightGradient READ singleHighlightGradient WRITE setSingleHighlightGradient NOTIFY singleHighlightGradientChanged)
    Q_PROPERTY(ColorGradient *multiHighlightGradient READ multiHighlightGradient WRITE setMultiHighlightGradient NOTIFY multiHighlightGradientChanged)
    Q_CLASSINFO("DefaultProperty", "themeChildren")
    QML_NAMED_ELEMENT(Theme3D)
    QML_ADDED_IN_VERSION(1, 0)

public:
    DeclarativeTheme3D(QObject *parent = 0);
    virtual ~DeclarativeTheme3D();

    QQmlListProperty<QObject> themeChildren();
    static void appendThemeChildren(QQmlListProperty<QObject> *list, QObject *element);

    QQmlListProperty<DeclarativeColor> baseColors();
    static void appendBaseColorsFunc(QQmlListProperty<DeclarativeColor> *list,
                                     DeclarativeColor *color);
    static qsizetype countBaseColorsFunc(QQmlListProperty<DeclarativeColor> *list);
    static DeclarativeColor *atBaseColorsFunc(QQmlListProperty<DeclarativeColor> *list,
                                              qsizetype index);
    static void clearBaseColorsFunc(QQmlListProperty<DeclarativeColor> *list);

    QQmlListProperty<ColorGradient> baseGradients();
    static void appendBaseGradientsFunc(QQmlListProperty<ColorGradient> *list,
                                        ColorGradient *gradient);
    static qsizetype countBaseGradientsFunc(QQmlListProperty<ColorGradient> *list);
    static ColorGradient *atBaseGradientsFunc(QQmlListProperty<ColorGradient> *list,
                                              qsizetype index);
    static void clearBaseGradientsFunc(QQmlListProperty<ColorGradient> *list);

    void setSingleHighlightGradient(ColorGradient *gradient);
    ColorGradient *singleHighlightGradient() const;

    void setMultiHighlightGradient(ColorGradient *gradient);
    ColorGradient *multiHighlightGradient() const;

    // From QQmlParserStatus
    void classBegin() override;
    void componentComplete() override;

Q_SIGNALS:
    void singleHighlightGradientChanged(ColorGradient *gradient);
    void multiHighlightGradientChanged(ColorGradient *gradient);

protected:
    void handleTypeChange(Theme themeType);
    void handleBaseColorUpdate();
    void handleBaseGradientUpdate();
    void handleSingleHLGradientUpdate();
    void handleMultiHLGradientUpdate();

    enum GradientType {
        GradientTypeBase = 0,
        GradientTypeSingleHL,
        GradientTypeMultiHL
    };

private:
    void addColor(DeclarativeColor *color);
    QList<DeclarativeColor *> colorList();
    void clearColors();
    void clearDummyColors();

    void addGradient(ColorGradient *gradient);
    QList<ColorGradient *> gradientList();
    void clearGradients();
    void clearDummyGradients();

    void setThemeGradient(ColorGradient *gradient, GradientType type);
    QLinearGradient convertGradient(ColorGradient *gradient);
    ColorGradient *convertGradient(const QLinearGradient &gradient);

    QList<DeclarativeColor *> m_colors; // Not owned
    QList<ColorGradient *> m_gradients; // Not owned
    ColorGradient *m_singleHLGradient; // Not owned
    ColorGradient *m_multiHLGradient; // Not owned

    bool m_dummyGradients;
    bool m_dummyColors;
};

QT_END_NAMESPACE

#endif
