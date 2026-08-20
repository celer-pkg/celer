// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef LOTTIEFEEFORMSHAPE_P_H
#define LOTTIEFEEFORMSHAPE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QPainterPath>
#include <QJsonArray>
#include <QMap>
#include <QHash>
#include <QList>

#include <QtLottie/qtlottieexports.h>
#include <QtLottie/private/qlottieshape_p.h>
#include <QtLottie/private/qlottietrimpath_p.h>
#include <QtLottie/private/qlottierenderer_p.h>

QT_BEGIN_NAMESPACE

class QJsonObject;

class Q_LOTTIE_EXPORT QLottieFreeFormShape : public QLottieShape
{
public:
    explicit QLottieFreeFormShape(const QLottieFreeFormShape &other);
    QLottieFreeFormShape(QLottieBase *parent = nullptr);

    QLottieBase *clone() const override;

    int parse(const QJsonObject &definition) override;

    void updateProperties(int frame) override;
    void render(QLottieRenderer &renderer) const override;

    bool acceptsTrim() const override;

    bool isAnimated() const;
    QLottieProperty2D<QPointF> startPointProperty() const { return m_vertexList.value(0).pos; };

protected:
    struct VertexInfo {
        QLottieProperty2D<QPointF> pos;
        QLottieProperty2D<QPointF> ci;
        QLottieProperty2D<QPointF> co;
    };

    void parseShapeKeyframes(QJsonObject &keyframes);
    void buildShape(const QJsonObject &keyframe);
    void buildShape(int frame);
    void parseEasedVertices(const QJsonObject &keyframe, int startFrame);

    QHash<int, QJsonObject> m_vertexMap;
    QList<VertexInfo> m_vertexList;
    QMap<int, bool> m_closedShape;

private:
    struct VertexBuildInfo
    {
        QJsonArray posKeyframes;
        QJsonArray ciKeyframes;
        QJsonArray coKeyframes;
    };

    void finalizeVertices();

    QMap<int, VertexBuildInfo*> m_vertexInfos;

    QJsonObject createKeyframe(QJsonArray startValue, QJsonArray endValue,
                               int startFrame, QJsonObject easingIn,
                               QJsonObject easingOut);

    void addCurve(QPointF start, QPointF ctlOffset1, QPointF ctlOffset2, QPointF end);
};

QT_END_NAMESPACE

#endif // LOTTIEFEEFORMSHAPE_P_H
