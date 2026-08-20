// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEBASE_P_H
#define QLOTTIEBASE_P_H

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

#include <QJsonObject>
#include <QList>
#include <QSize>

#include <QtLottie/qtlottieexports.h>
#include <QtLottie/private/qlottieconstants_p.h>

#include <QtLottie/private/qlottierenderer_p.h>

QT_BEGIN_NAMESPACE
class Q_LOTTIE_EXPORT QLottieBase
{
public:
    QLottieBase() = default;
    explicit QLottieBase(const QLottieBase &other);
    virtual ~QLottieBase();

    virtual QLottieBase *clone() const;

    virtual bool setProperty(QLottieLiteral::PropertyType propertyType, QVariant value);

    QString name() const;
    void setName(const QString &name);

    int type() const;
    void setType(int type);
    bool isShapeElement() const;
    bool isPathElement() const;
    virtual int parse(const QJsonObject &definition);

    const QJsonObject& definition() const;

    virtual bool active(int frame) const;
    bool hidden() const;

    inline QLottieBase *parent() const { return m_parent; }
    void setParent(QLottieBase *parent);

    const QList<QLottieBase *> &children() const { return m_children; }
    void prependChild(QLottieBase *child);
    void insertChildBeforeLast(QLottieBase *child);
    void appendChild(QLottieBase *child);

    virtual QLottieBase *findChild(const QString &childName);

    virtual void updateProperties(int frame);
    virtual void render(QLottieRenderer &renderer) const;
    virtual QSize layerSize() const;

    bool isStructureDumping() const;

protected:
    void resolveTopRoot();
    QLottieBase *topRoot() const;
    const QJsonObject resolveExpression(const QJsonObject& definition);
    void renderChildren(QLottieRenderer &renderer) const;
    bool checkRequiredKeys(const QJsonObject &definition, const QLatin1StringView type,
                           const QList<QLatin1StringView> &keys, const QString &name = QString()) const;

protected:
    QJsonObject m_definition;
    int m_type = -1;
    bool m_hidden = false;
    mutable qint8 m_structureDumping = -1;
    QString m_name;
    QString m_matchName;
    bool m_autoOrient = false;

    friend class QLottieRasterRenderer;
    friend class QLottieRenderer;

private:
    QLottieBase *m_parent = nullptr;
    QList<QLottieBase *> m_children;

    // Handle to the topmost element on which this element resides
    // Will be resolved when traversing effects
    QLottieBase *m_topRoot = nullptr;
};

QT_END_NAMESPACE

#endif // QLOTTIEBASE_P_H
