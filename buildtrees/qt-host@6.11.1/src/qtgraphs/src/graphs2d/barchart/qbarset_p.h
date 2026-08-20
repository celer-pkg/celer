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

#ifndef QBARSET_P_H
#define QBARSET_P_H

#include <QSet>
#include <QtCore/QMap>
#include <QtGraphs/qbarset.h>
#include <QtGui/QBrush>
#include <QtGui/QFont>
#include <QtGui/QPen>
#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QBarSetPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QBarSet)
public:
    QBarSetPrivate(const QString &label);
    ~QBarSetPrivate() override;

    void append(QPointF value);
    void append(const QList<QPointF> &values);
    void append(const QList<qreal> &values);

    void insert(qsizetype index, qreal value);
    void insert(qsizetype index, QPointF value);
    qsizetype remove(qsizetype index, qsizetype count);

    void replace(qsizetype index, qreal value);

    qreal pos(qsizetype index) const;
    qreal value(qsizetype index) const;

    void setVisualsDirty(bool dirty) { m_visualsDirty = dirty; }
    bool visualsDirty() const { return m_visualsDirty; }
    void setLabelsDirty(bool dirty) { m_labelsDirty = dirty; }
    bool labelsDirty() const { return m_labelsDirty; }

    void setBarSelected(qsizetype index, bool selected, bool &callSignal);
    bool isBarSelected(qsizetype index) const;

public:
    QString m_label;
    QList<QPointF> m_values;
    QSet<qsizetype> m_selectedBars;
    // By default colors are transparent, meaning that use the ones from theme
    QColor m_color = QColor(Qt::transparent);
    QColor m_borderColor = QColor(Qt::transparent);
    QColor m_labelColor = QColor(Qt::transparent);
    QColor m_selectedColor = QColor(Qt::transparent);
    // By default border width is -1, meaning that use the one from theme
    qreal m_borderWidth = -1;
    bool m_visualsDirty;
    bool m_labelsDirty;

    friend class QBarSeries;
};

QT_END_NAMESPACE

#endif // QBARSETPRIVATE_P_H
