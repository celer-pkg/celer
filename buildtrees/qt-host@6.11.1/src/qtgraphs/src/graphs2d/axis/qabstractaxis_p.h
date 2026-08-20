// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QABSTRACTAXIS_P_H
#define QABSTRACTAXIS_P_H

#include <QtGraphs/QAbstractAxis>
#include <private/qgraphsview_p.h>
#include <QtCore/QDebug>
#include <QtCore/private/qobject_p.h>
#include <QColor>

#include <memory>

QT_BEGIN_NAMESPACE

class QQmlComponent;

class QAbstractAxisPrivate : public QObjectPrivate
{
public:
    QAbstractAxisPrivate();
    ~QAbstractAxisPrivate() override;

public:
    void setGraph(QGraphsView *graph);

    //interface for manipulating range form base class
    virtual void setMin(const QVariant &min) = 0;
    virtual void setMax(const QVariant &max) = 0;
    virtual void setRange(const QVariant &min, const QVariant &max) = 0;

    //interface manipulating range form domain
    virtual void setRange(qreal min, qreal max) = 0;
    virtual qreal min() = 0;
    virtual qreal max() = 0;

public Q_SLOTS:
    void handleRangeChanged(qreal min, qreal max);

protected:
    QGraphsView *m_graph = nullptr;

private:
    bool m_visible = true;

    bool m_lineVisible = true;

    bool m_gridVisible = true;
    bool m_subGridVisible = true;

    bool m_labelsVisible = true;
    qreal m_labelsAngle = 0;
    QQmlComponent *m_labelDelegate = nullptr;

    bool m_titleVisible = true;
    QColor m_titleColor;
    QFont m_titleFont;
    QString m_title;

    Qt::Alignment m_alignment = Qt::AlignAbsolute;
    Qt::TextElideMode m_textElide = Qt::ElideNone;

    QColor m_subColor;
    QColor m_color;

    Q_DECLARE_PUBLIC(QAbstractAxis)
};

QT_END_NAMESPACE

#endif
