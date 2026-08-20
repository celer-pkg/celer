// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGDIRECTIONALLIGHT_H
#define QSSGDIRECTIONALLIGHT_H

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

#include <QtQuick3D/private/qquick3dabstractlight_p.h>

#include <QColor>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DDirectionalLight : public QQuick3DAbstractLight
{
    Q_OBJECT

    QML_NAMED_ELEMENT(DirectionalLight)

    Q_PROPERTY(float csmSplit1 READ csmSplit1 WRITE setCsmSplit1 NOTIFY csmSplit1Changed FINAL REVISION(6, 8))
    Q_PROPERTY(float csmSplit2 READ csmSplit2 WRITE setCsmSplit2 NOTIFY csmSplit2Changed FINAL REVISION(6, 8))
    Q_PROPERTY(float csmSplit3 READ csmSplit3 WRITE setCsmSplit3 NOTIFY csmSplit3Changed FINAL REVISION(6, 8))
    Q_PROPERTY(int csmNumSplits READ csmNumSplits WRITE setCsmNumSplits NOTIFY csmNumSplitsChanged FINAL REVISION(6, 8))
    Q_PROPERTY(float csmBlendRatio READ csmBlendRatio WRITE setCsmBlendRatio NOTIFY csmBlendRatioChanged FINAL REVISION(6, 8))
    Q_PROPERTY(bool lockShadowmapTexels READ lockShadowmapTexels WRITE setLockShadowmapTexels NOTIFY
                       lockShadowmapTexelsChanged FINAL REVISION(6, 9))

public:
    explicit QQuick3DDirectionalLight(QQuick3DNode *parent = nullptr);
    ~QQuick3DDirectionalLight() override {}

    Q_REVISION(6, 8) float csmSplit1() const;
    Q_REVISION(6, 8) float csmSplit2() const;
    Q_REVISION(6, 8) float csmSplit3() const;
    Q_REVISION(6, 8) int csmNumSplits() const;
    Q_REVISION(6, 8) float csmBlendRatio() const;
    Q_REVISION(6, 9) bool lockShadowmapTexels() const;

    Q_REVISION(6, 8) void setCsmSplit1(float newcsmSplit1);
    Q_REVISION(6, 8) void setCsmSplit2(float newcsmSplit2);
    Q_REVISION(6, 8) void setCsmSplit3(float newcsmSplit3);
    Q_REVISION(6, 8) void setCsmNumSplits(int newcsmNumSplits);
    Q_REVISION(6, 8) void setCsmBlendRatio(float newcsmBlendRatio);
    Q_REVISION(6, 9) void setLockShadowmapTexels(bool newLockShadowmapTexels);

Q_SIGNALS:
    Q_REVISION(6, 8) void csmSplit1Changed();
    Q_REVISION(6, 8) void csmSplit2Changed();
    Q_REVISION(6, 8) void csmSplit3Changed();
    Q_REVISION(6, 8) void csmNumSplitsChanged();
    Q_REVISION(6, 8) void csmBlendRatioChanged();
    Q_REVISION(6, 9) void lockShadowmapTexelsChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

private:
    float m_csmSplit1 = 0.1f;
    float m_csmSplit2 = 0.25f;
    float m_csmSplit3 = 0.5f;
    int m_csmNumSplits = 0;
    float m_csmBlendRatio = 0.05f;
    bool m_lockShadowmapTexels = false;
};

QT_END_NAMESPACE
#endif // QSSGDIRECTIONALLIGHT_H
